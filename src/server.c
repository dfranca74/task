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

#include "server.h"
#include "utils.h"
#include "database.h"

#define DEBUG_SERVER

// Following struct will be used to handle a POST command
// Reference: https://www.gnu.org/software/libmicrohttpd/tutorial.html#Processing-POST-data
typedef struct
{
    struct MHD_PostProcessor *pp;

   terminal_t terminal;

} connection_info_struct_t;

static pthread_t server_id;

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

static int handle_helper_pages(struct MHD_Connection *connection, const char *helper);
static int handle_request_terminal_info(struct MHD_Connection *connection, const char *id);
static int handle_unknown_url(struct MHD_Connection *connection, const char *url);

// Auxiliary functions to help debug/understand how http protocol works:
#ifdef DEBUG_SERVER
static int callback_print_out_key (void *cls, enum MHD_ValueKind kind, const char *key, const char *value);
#endif

static const char *greetings_page  = "<html><body>Hello, how are you ?</body></html>";
static const char *database_full   = "<html><body>Message discarded. Database is full</body></html>";
static const char *parser_error    = "<html><body>Message discarded. Unknown format</body></html>";

error_t create_server_thread(void)
{
    error_t result = ERROR_NO;

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

    // ToDo: Put here a condition variable to exit the thread (signaled by main.c when a CTRL+C was sent)
    size_t wait = 60;
    while (wait--)
    {
        sleep(1);
    }

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


    // Start testing the end points:
    // The following code is only for debug/purposes (to help me understand how this server and http works):
#ifdef DEBUG_SERVER
    printf ("New method:%s request for url:%s using version %s\n", method, url, version);
    MHD_get_connection_values (connection, MHD_HEADER_KIND, &callback_print_out_key, NULL);
#endif

    // First end point: greetings page
    if ((0 == strcmp(url, "/greetings")) && (0 == strcasecmp(method, MHD_HTTP_METHOD_GET)))
    {
       return handle_helper_pages(connection, greetings_page);
    }

    // Second end point: request terminal information based on its ID number
    // I will propose this end point url:
    // http://localhost:9876/terminal/read/x    <--- where 'x' will be the terminal id to be searched in the database and prepare the json response
    if ((NULL != strstr(url, "/terminal/read/")) && (0 == strcasecmp(method, MHD_HTTP_METHOD_GET)))
    {
        // This first substring comparison is relatively weak but enough here (we will discard garbage)
        // The sanitized search for a valid id will be made inside next function:
        const char *id = strrchr(url, '/');
        if (id)
        {
            // Advance for what is supposed to be the ID number:
            ++id;
            return handle_request_terminal_info(connection, id);
        }

        return handle_unknown_url(connection, url);
    }

    // Endpoint to read all terminals stored in the database
    if ((NULL != strstr(url, "/terminal/all")) && (0 == strcasecmp(method, MHD_HTTP_METHOD_GET)))
    {
        char buffer[4096] = { 0 };
        int offset = snprintf(buffer, sizeof(buffer), "<html><body>\n");
        get_terminal_all(buffer + offset, sizeof(buffer) - offset);
        offset = strlen(buffer);
        snprintf(buffer + offset, sizeof(buffer) - offset, "</body></html>");
        return handle_helper_pages(connection, buffer);
    }

    // I ***MUST*** need to have something working to show.
    // I will first code a regular POST data (no json) to add new terminal and debug it.
    // Then I will change the logic to add Json processing (my 48 hours limit is finishing)
    if ((0 == strcmp(url, "/terminal/add")) && (0 == strcasecmp(method, MHD_HTTP_METHOD_POST)))
    {
        connection_info_struct_t *info = *con_cls;

        if (0 != *upload_data_size)
        {
            MHD_post_process (info->pp, upload_data, *upload_data_size);
            *upload_data_size = 0;
            return MHD_YES;
        }
        else
        {
            printf("card: %s - transaction: %s\n", info->terminal.card, info->terminal.transaction);

            char buffer[256] = { 0 };

            const char *message = parser_error;

            // This sanity check will verify if the received message was in the expected key/value format:
            if (strnlen(info->terminal.card, MAX_VALUE_SIZE) &&
                strnlen(info->terminal.transaction, MAX_VALUE_SIZE))
            {
                // We are ready to store the new terminal in the database and send back a
                // page response with the ID associated with it:
                if (ERROR_NO == add_terminal(&info->terminal))
                {
                    snprintf(buffer,
                            sizeof(buffer),
                            "<html><body>id: %ld - card: %s - transaction: %s</body></html>",
                            info->terminal.id, info->terminal.card, info->terminal.transaction);

                    message = buffer;
                }
                else
                {
                    message = database_full;
                }
            }

            return handle_helper_pages(connection, message);
        }
    }

    // Will start with this initial curl syntax (not json yet):
    // curl -d "card=value1&transaction=value2" -X POST http://localhost:9876/terminal/add

    return handle_unknown_url(connection, url);
}

#ifdef DEBUG_SERVER
static int callback_print_out_key (void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
    (void)(cls);
    (void)(kind);
    printf ("%s: %s\n", key, value);
    return MHD_YES;
}
#endif

static int handle_unknown_url(struct MHD_Connection *connection, const char *url)
{
    char buffer[512] = { 0 };
    snprintf(buffer, sizeof(buffer), "<html><body>The url: '%s' is unknown by the server</body></html>", url);

    return handle_helper_pages(connection, buffer);
}

static int handle_helper_pages(struct MHD_Connection *connection, const char *description)
{
    const size_t len = strlen(description);
    void *page = (void*)(description);

    struct MHD_Response *response = MHD_create_response_from_buffer (len, page, MHD_RESPMEM_MUST_COPY);

    if (!response)
    {
        printf("%s - Failed MHD_create_response_from_buffer\n", __func__);
        return MHD_NO;
    }

    const int ret = MHD_queue_response (connection, MHD_HTTP_OK, response);

    if (MHD_YES != ret)
    {
        printf("%s - Failed MHD_queue_response\n", __func__);
    }

    MHD_destroy_response (response);

    return ret;
}

static int handle_request_terminal_info(struct MHD_Connection *connection, const char *id)
{
    terminal_t terminal;

    if (ERROR_NO != convert_str_to_int(id, &terminal.id))
    {
        return MHD_NO;
    }

    // Ok, here we have a valid number.
    // Do we have in our database any terminal with this same number id ?
    char buffer[256] = { 0 };

    if (ERROR_NO == get_terminal(&terminal))
    {
        snprintf(buffer,
                sizeof(buffer),
                "<html><body>Terminal id[%ld]: card=%s - type=%s</body></html>",
                terminal.id, terminal.card, terminal.transaction);
    }
    else
    {
        snprintf(buffer,
                sizeof(buffer), "<html><body>Terminal id[%ld] not stored in database</body></html>",
                terminal.id);
    }

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
