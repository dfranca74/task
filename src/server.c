/*
 * server.c
 *
 *  Created on: 29/01/2020
 *      Author: dfranca
 */

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "server.h"

static pthread_t server_id;

static void *worker_thread(void *args);

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

    printf("Worker thread called\n");
    sleep(10);

    return NULL;
}

