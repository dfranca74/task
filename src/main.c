
#include <stdio.h>
#include <stdlib.h>
#include "server.h"

int main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);

    if (ERROR_NO != create_server_thread())
    {
        exit(EXIT_FAILURE);
    }

    wait_server_thread();

    return EXIT_SUCCESS;
}
