/*
 * database.c
 *
 *  Created on: 29/01/2020
 *      Author: dfranca
 */

#include <stddef.h>
#include <stdio.h>

#include "database.h"
#include "config.h"

static terminal_t buffer[MAX_DATABASE_REGISTERS];

// Used only to verify if the buffer has available slot(s)
static size_t added_terminals = 0;

// Used to give a unique id to each new terminal added into database:
static int id_register = 1;

#define AVAILABLE_SLOT -1

static const char *CARD_TYPE_NAMES[] =
{
        "Visa",
        "MasterCard",
        "EFTPOS"
};

static const char *TRANSACTION_TYPE_NAMES[] =
{
        "Cheque",
        "Savings",
        "Credit"
};

void init_database(void)
{
    for (size_t i = 0; i < MAX_DATABASE_REGISTERS; ++i)
    {
        buffer[i].id = AVAILABLE_SLOT;
        buffer[i].card = NULL;
        buffer[i].transaction = NULL;
    }

    id_register = 1;
    added_terminals = 0;

}

size_t get_total_terminal(void)
{
    return added_terminals;
}

error_t get_terminal(int id, const char **card, const char **transaction)
{
    error_t result = ERROR_TERMINAL_NOT_FOUND;

    // Sanity check arguments:
    if (id < 0 || !card || !transaction)
    {
        printf("%s - Invalid argument(s)\n", __func__);
        return ERROR_INVALID_ARGUMENT;
    }

    // Sanity check: buffer is empty
    if (!added_terminals)
    {
        printf("%s - Terminal Id=%d is not present in database\n", __func__, id);
        return ERROR_TERMINAL_NOT_FOUND;
    }

    // Traverse the buffer searching for a terminal with an "id" match:
    for (size_t i = 0; i < MAX_DATABASE_REGISTERS; ++i)
    {
        if (id == buffer[i].id)
        {
            *card = buffer[i].card;
            *transaction = buffer[i].transaction;
            result = ERROR_NO;

            break;
        }
    }

    return result;
}

error_t add_terminal(card_t type, transaction_t transaction, int *id)
{
    error_t result = ERROR_NO;

    // Sanity check the arguments:
    if (!id || (type < VISA || type > EFTPOS) || (transaction < CHEQUE || transaction > CREDIT))
    {
        printf("%s - Invalid argument(s)\n", __func__);
        return ERROR_INVALID_ARGUMENT;
    }

    if (added_terminals < MAX_DATABASE_REGISTERS)
    {
        // Traverse the buffer to find the first slot available:
        for (size_t i = 0; i < MAX_DATABASE_REGISTERS; ++i)
        {
            // A terminal with id value -1 means it is available and ready to be used:
            if (AVAILABLE_SLOT == buffer[i].id)
            {
                // Avoid allocating memory: just point to the correct constant string value
                buffer[i].id = id_register++;
                buffer[i].card = CARD_TYPE_NAMES[type];
                buffer[i].transaction = TRANSACTION_TYPE_NAMES[transaction];

                *id = buffer[i].id;
                ++added_terminals;
                break;
            }
        }
    }
    else
    {
        result = ERROR_DATABASE_FULL;
    }

    return result;
}


