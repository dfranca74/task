/*
 * endpoints_handler.c
 *
 *  Created on: 31/01/2020
 *      Author: dfranca
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "endpoints_handler.h"
#include "utils.h"
#include "database.h"

static size_t counter = 0;

/**
 * Auxiliary messages
 */
static const char *parser_error = "<html><body>Message discarded. Unknown format</body></html>";
static const char *database_full = "<html><body>Message discarded. Database is full</body></html>";

int endpoint_greetings(struct MHD_Connection *connection)
{
    char buffer[256] = { 0 };
    snprintf(buffer, sizeof(buffer), "<html><body>Hello, how are you [%zu] ?</body></html>\n", counter++);
    return handle_helper_pages(connection, buffer);
}

int endpoint_terminal_read_id(struct MHD_Connection *connection, const char *url)
{
    char buffer[256] = { 0 };

    // Returns a pointer to the last occurence of character '/':
    const char *id = strrchr(url, '/');

    if (id)
    {
        terminal_t terminal;

        // Advance the pointer for what is supposed to be the first character of the ID:
        ++id;

        if (ERROR_NO != convert_str_to_int(id, &terminal.id))
        {
            return MHD_NO;
        }

        // Ok, here we have a valid number.
        // Do we have in our database any terminal with this same number id ?
        if (ERROR_NO == get_terminal(&terminal))
        {
            snprintf(buffer,
                    sizeof(buffer),
                    "<html><body>Terminal id[%ld]: card=%s - type=%s</body></html>\n",
                    terminal.id, terminal.card, terminal.transaction);
        }
        else
        {
            snprintf(buffer,
                    sizeof(buffer), "<html><body>Terminal id[%ld] not found in database</body></html>\n",
                    terminal.id);
        }

        return handle_helper_pages(connection, buffer);
    }

    return MHD_NO;
}

int endpoint_terminal_read_all(struct MHD_Connection *connection)
{
    char buffer[4096] = { 0 };

    int offset = snprintf(buffer, sizeof(buffer), "<html><body>\n");
    get_terminal_all(buffer + offset, sizeof(buffer) - offset);

    offset = strlen(buffer);
    snprintf(buffer + offset, sizeof(buffer) - offset, "</body></html>");

    return handle_helper_pages(connection, buffer);
}

int endpoint_terminal_add(struct MHD_Connection *connection, void **con_cls, const char *upload_data, size_t *upload_data_size)
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
