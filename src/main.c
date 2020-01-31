
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "server.h"
#include "database.h"

static void signal_handler(int) ;

int main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);

    struct sigaction act;
    act.sa_handler = signal_handler;
    sigaction(SIGINT, &act, NULL);

    init_database();

    if (ERROR_NO != create_server_thread())
    {
        exit(EXIT_FAILURE);
    }

    wait_server_thread();

    return EXIT_SUCCESS;
}

static void signal_handler(int signal)
{
    (void)(signal);
    exit_server_thread();
}
