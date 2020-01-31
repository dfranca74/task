/*
 * database.h
 *
 *  Created on: 29/01/2020
 *      Author: dfranca
 */

#ifndef DATABASE_H_
#define DATABASE_H_

#include "errors.h"
#include "config.h"

typedef struct
{
    long int id;
    char card[MAX_VALUE_SIZE];
    char transaction[MAX_VALUE_SIZE];
} terminal_t;

void init_database(void);
void get_terminal_all(char *buffer, size_t size);
error_t get_terminal(terminal_t *terminal);
error_t add_terminal(terminal_t *terminal);


#endif /* DATABASE_H_ */
