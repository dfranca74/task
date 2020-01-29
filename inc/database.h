/*
 * database.h
 *
 *  Created on: 29/01/2020
 *      Author: dfranca
 */

#ifndef DATABASE_H_
#define DATABASE_H_

#include "errors.h"

typedef enum
{
    VISA = 0,
    MASTERCARD,
    EFTPOS
} card_t;

typedef enum
{
    CHEQUE = 0,
    SAVINGS,
    CREDIT
} transaction_t;

typedef struct
{
    int id;
    const char *card;
    const char *transaction;
} terminal_t;

void init_database(void);
error_t get_terminal(int id, const char **card, const char **transaction);
error_t add_terminal(card_t card, transaction_t transaction, int *id);
size_t get_total_terminal(void);

#endif /* DATABASE_H_ */
