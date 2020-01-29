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

// Auxiliary functions to help debug/understand how http protocol works:
#ifdef DEBUG_SERVER
static int callback_print_out_key (void *cls, enum MHD_ValueKind kind, const char *key, const char *value);
#endif

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
    (void)(con_cls);

    const char *hello_page  = "<html><body>Hello, Smartpay!</body></html>";
    const size_t len = strlen(hello_page);
    void *page = (void*)(hello_page);

    // The following code is only for debug/purposes (to help me understand how this server and http works):
#ifdef DEBUG_SERVER
    printf ("New %s request for %s using version %s\n", method, url, version);
    MHD_get_connection_values (connection, MHD_HEADER_KIND, &callback_print_out_key, NULL);
#endif

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

#ifdef DEBUG_SERVER
static int callback_print_out_key (void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
    (void)(cls);
    (void)(kind);
    printf ("%s: %s\n", key, value);
    return MHD_YES;
}
#endif

