/*
 * server.c
 *
 *  Created on: 29/01/2020
 *      Author: dfranca
 */

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>

#include "server.h"
#include "utils.h"
#include "database.h"
#include "endpoints_handler.h"

static pthread_t server_id;
static sem_t semaphore;

static void *worker_thread(void *args);
static int callback_connection (void *cls,
                                struct MHD_Connection *connection,
                                const char *url,
                                const char *method,
                                const char *version,
                                const char *upload_data,
                                size_t *upload_data_size,
                                void **con_cls);

static int iterate_post_add(void *coninfo_cls,
                            enum MHD_ValueKind kind,
                            const char *key,
                            const char *filename,
                            const char *content_type,
                            const char *transfer_encoding,
                            const char *data,
                            uint64_t off,
                            size_t size);

static void request_completed (void *cls,
                               struct MHD_Connection *connection,
                               void **con_cls,
                               enum MHD_RequestTerminationCode toe);


static int handle_unknown_url(struct MHD_Connection *connection, const char *url);

error_t create_server_thread(void)
{
    error_t result = ERROR_NO;

    sem_init(&semaphore, 0, 1);
    sem_wait(&semaphore);

    const int error = pthread_create(&server_id, NULL, worker_thread, NULL);

    if (error)
    {
        printf("Failed to create server thread: %d\n", error);
        result = ERROR_CREATE_THREAD;
    }

    return result;
}

void wait_server_thread(void)
{
    pthread_join(server_id, NULL);
}

void exit_server_thread(void)
{
    sem_post(&semaphore);
}

static void *worker_thread(void *args)
{
    (void)(args);

    struct MHD_Daemon *daemon = NULL;

    daemon = MHD_start_daemon (MHD_USE_POLL_INTERNALLY, // Seems to be a good/simpler initial approach
                               SERVER_PORT,
                               NULL,
                               NULL,
                               &callback_connection,
                               NULL,
                               MHD_OPTION_NOTIFY_COMPLETED,
                               &request_completed,
                               NULL,
                               MHD_OPTION_END);

    if (!daemon)
    {
        printf("%s - Failed MHD_start_daemon. Exiting server thread\n", __func__);
        return NULL;
    }

    // Block thread until someone signals the semaphore to exit the thread
    sem_wait(&semaphore);

    // Clean allocated resource:
    MHD_stop_daemon(daemon);
    daemon = NULL;

    printf("%s - Exiting\n", __func__);

    return NULL;
}

// First attempt to setup the HTTP server
// https://www.gnu.org/software/libmicrohttpd/tutorial.html
static int callback_connection (void *cls,
                                struct MHD_Connection *connection,
                                const char *url,
                                const char *method,
                                const char *version,
                                const char *upload_data,
                                size_t *upload_data_size,
                                void **con_cls)
{
    // Not using yet the following parameters:
    (void)(cls);
    (void)(url);
    (void)(method);
    (void)(version);
    (void)(upload_data);
    (void)(upload_data_size);

    if (NULL == *con_cls)
    {
        // Allocate a connection info struct in heap:
        connection_info_struct_t *info = calloc(1, sizeof(connection_info_struct_t));

        if (!info)
        {
            printf("Failed to allocate connection_info_struct_t\n");
            return MHD_NO;
        }

        // Verify if the new request is a POST. If yes the post processor will be created:
        if (0 == strcmp(method, MHD_HTTP_METHOD_POST))
        {
            info->pp = MHD_create_post_processor(connection, 1024, iterate_post_add, (void *)info);

            if (NULL == info->pp)
            {
                free(info);
                info = NULL;
                printf("Failed to create post processor\n");
                return MHD_NO;
            }

            // Now, I thing I understood why this:
            // "The correct approach is to simply not queue a message on the first callback unless there is an error"
            *con_cls = (void*)info;
            return MHD_YES;
        }
    }

    // Accordingly to the specification we need three end points:
    // 1) Create a new terminal (POST)
    // 2) Read the details of an existing terminal (GET)
    // 3) Return a list of all terminals (GET)


    // First end point: greetings page
    // http://localhost:9876/greetings
    if ((0 == strcmp(url, "/greetings")) && (0 == strcasecmp(method, MHD_HTTP_METHOD_GET)))
    {
       return endpoint_greetings(connection);
    }

    // Second end point: request terminal information based on its ID number
    // http://localhost:9876/terminal/read/x    <--- where 'x' will be the terminal id to be searched in the database and prepare the json response
    if ((NULL != strstr(url, "/terminal/read/")) && (0 == strcasecmp(method, MHD_HTTP_METHOD_GET)))
    {
        return endpoint_terminal_read_id(connection, url);
    }

    // Endpoint to read all terminals stored in the database
    if ((NULL != strstr(url, "/terminal/all")) && (0 == strcasecmp(method, MHD_HTTP_METHOD_GET)))
    {
        return endpoint_terminal_read_all(connection);
    }

    // I ***MUST*** need to have something working to show.
    // I will first code a regular POST data (no json) to add new terminal and debug it.
    // Then I will change the logic to add Json processing (my 48 hours limit is finishing)
    if ((0 == strcmp(url, "/terminal/add")) && (0 == strcasecmp(method, MHD_HTTP_METHOD_POST)))
    {
        return endpoint_terminal_add(connection, con_cls, upload_data, upload_data_size);
    }

    return handle_unknown_url(connection, url);
}

static int handle_unknown_url(struct MHD_Connection *connection, const char *url)
{
    char buffer[256] = { 0 };
    snprintf(buffer, sizeof(buffer), "<html><body>The url: '%s' is unknown by the server</body></html>", url);
    return handle_helper_pages(connection, buffer);
}

static int iterate_post_add(void *coninfo_cls,
                            enum MHD_ValueKind kind,
                            const char *key,
                            const char *filename,
                            const char *content_type,
                            const char *transfer_encoding,
                            const char *data,
                            uint64_t off,
                            size_t size)
{
    connection_info_struct_t *info = coninfo_cls;

    (void)(kind);
    (void)(filename);
    (void)(content_type);
    (void)(transfer_encoding);
    (void)(off);
    (void)(size);

    if (0 == strcmp(key, "card"))
    {
        strncpy(info->terminal.card, data, sizeof(info->terminal.card));
    }
    else if (0 == strcmp(key, "transaction"))
    {
        strncpy(info->terminal.transaction, data, sizeof(info->terminal.transaction));
    }
    else
    {
        printf("Unexpected key value: %s", key);
        return MHD_NO;
    }

    if (0 != strnlen(info->terminal.card, MAX_VALUE_SIZE) &&
        0 != strnlen(info->terminal.transaction, MAX_VALUE_SIZE))
    {
        // Ok, we have received all values:
        return MHD_NO;
    }

    // We have more data to be received:
    return MHD_YES;
}

static void request_completed(void *cls,
                              struct MHD_Connection *connection,
                              void **con_cls,
                              enum MHD_RequestTerminationCode toe)
{
    // Clean-up resources:
    connection_info_struct_t *info = *con_cls;

    (void)(cls);
    (void)(connection);
    (void)(toe);

    if (NULL == info)
    {
        // Do nothing
        return;
    }

    if (info->pp)
    {
        MHD_destroy_post_processor(info->pp);
        info->pp = NULL;
    }

    free(info);
    info = NULL;
    *con_cls = NULL;
}
