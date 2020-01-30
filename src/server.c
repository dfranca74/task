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

#include "server.h"

#define DEBUG_SERVER

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

static int handle_helper_pages(struct MHD_Connection *connection, const char *helper);
static int handle_request_terminal_info(struct MHD_Connection *connection, const char *id);
static int handle_unknown_url(struct MHD_Connection *connection, const char *url);

// Auxiliary functions to help debug/understand how http protocol works:
#ifdef DEBUG_SERVER
static int callback_print_out_key (void *cls, enum MHD_ValueKind kind, const char *key, const char *value);
#endif

static const char *greetings_page  = "<html><body>Hello, how are you ?</body></html>";

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
                               MHD_OPTION_END);

    if (!daemon)
    {
        printf("%s - Failed MHD_start_daemon. Exiting server thread\n", __func__);
        return NULL;
    }

    // ToDo: Put here a condition variable to exit the thread (signaled by main.c when a CTRL+C was sent)
    size_t wait = 30;
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

    static int aptr;

    // Filter for methods GET and POST:
    // Accordingly to the specification we need three end points:
    // 1) Create a new terminal (POST)
    // 2) Read the details of an existing terminal (GET)
    // 3) Return a list of all terminals (GET)

    if (0 != strcmp(method, MHD_HTTP_METHOD_GET) &&
        0 != strcmp(method, MHD_HTTP_METHOD_POST))
    {
        printf("%s - HTTP method %s is not supported\n", __func__, method);
        return MHD_NO;
    }

    // Documentation says that:
    // "The correct approach is to simply not queue a message on the first callback unless there is an error"
    // This is very curious. Did not fully understood why....
    if (&aptr != *con_cls)
    {
        *con_cls = &aptr;
        return MHD_YES;
    }

    *con_cls = NULL;

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

        (void) handle_unknown_url(connection, url);
        return MHD_NO;
    }

    printf("Not ran any end point\n");
    return MHD_NO;
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
    snprintf(buffer, sizeof(buffer), "<html><body>The url: %s is unknown by the server</body></html>", url);

    return handle_helper_pages(connection, buffer);
}

static int handle_helper_pages(struct MHD_Connection *connection, const char *description)
{
    const size_t len = strlen(description);
    void *page = (void*)(description);

    struct MHD_Response *response = MHD_create_response_from_buffer (len, page, MHD_RESPMEM_PERSISTENT);

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
    // do nothing yet
    (void)(connection);
    printf("Called %s - id: %s\n", __func__, id);
    return MHD_YES;
}
