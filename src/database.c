/*
 * database.c
 *
 *  Created on: 29/01/2020
 *      Author: dfranca
 */

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "database.h"
#include "config.h"

// In ram database
static terminal_t buffer[MAX_DATABASE_REGISTERS];

// Used only to verify if the buffer has available slot(s)
static size_t added_terminals = 0;

// Used to give a unique id to each new terminal added into database:
static int id_register = 1;

#define AVAILABLE_SLOT -1

void init_database(void)
{
    memset(buffer, 0x00, sizeof(buffer));

    for (size_t i = 0; i < MAX_DATABASE_REGISTERS; ++i)
    {
        buffer[i].id = AVAILABLE_SLOT;
    }

    id_register = 1;
    added_terminals = 0;
}

void get_terminal_all(char *pbuf, size_t size)
{
    // We will iterate through database and copy terminal info into buffer:
    // Format: id:x card:y type:z\n

    if (added_terminals)
    {
        int offset = 0;

        for (size_t i = 0; i < MAX_DATABASE_REGISTERS; ++i)
        {
            if (AVAILABLE_SLOT != buffer[i].id)
            {
                int total = snprintf(pbuf + offset,
                                     size - offset,
                                     "id:%ld card:%s transaction:%s\n",
                                     buffer[i].id, buffer[i].card, buffer[i].transaction);

                offset += total;
            }
        }
    }
    else
    {
        snprintf(pbuf, size, "Database is empty\n");
    }
}

error_t get_terminal(terminal_t *terminal)
{
    // Sanity check arguments:
    if (!terminal)
    {
        printf("%s - Invalid argument(s)\n", __func__);
        return ERROR_INVALID_ARGUMENT;
    }

    // Sanity check: buffer is empty
    if (!added_terminals)
    {
        printf("%s - Terminal Id=%ld is not present in database\n", __func__, terminal->id);
        return ERROR_TERMINAL_NOT_FOUND;
    }

     bool found = false;

    // Traverse the buffer searching for a terminal with an "id" match:
    for (size_t i = 0; i < MAX_DATABASE_REGISTERS && !found; ++i)
    {
        if (terminal->id == buffer[i].id)
        {
            strncpy(terminal->card, buffer[i].card, MAX_VALUE_SIZE);
            strncpy(terminal->transaction, buffer[i].transaction, MAX_VALUE_SIZE);
            found = true; // break the loop
        }
    }

    return (found) ? ERROR_NO : ERROR_TERMINAL_NOT_FOUND;
}

error_t add_terminal(terminal_t *terminal)
{
    // Sanity check the arguments:
    if (!terminal)
    {
        printf("%s - Invalid argument(s)\n", __func__);
        return ERROR_INVALID_ARGUMENT;
    }

    bool added = false;

    if (added_terminals < MAX_DATABASE_REGISTERS)
    {
        // Traverse the buffer to find the first slot available:
        for (size_t i = 0; i < MAX_DATABASE_REGISTERS && !added; ++i)
        {
            // A terminal with id value -1 means it is available and ready to be used:
            if (AVAILABLE_SLOT == buffer[i].id)
            {
                terminal->id = id_register++;
                buffer[i].id = terminal->id;

                strncpy(buffer[i].card, terminal->card, MAX_VALUE_SIZE);
                strncpy(buffer[i].transaction, terminal->transaction, MAX_VALUE_SIZE);

                ++added_terminals;

                added = true; // break the loop
            }
        }
    }

    return (added) ? ERROR_NO : ERROR_DATABASE_FULL;
}
