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
#include "utils.h"

/**
 * Initialize in ram simple database
 */
void init_database(void);

/**
 * Copy to buffer information from all terminals stored in the database
 * @param buffer Buffer that will receive terminal data
 * @param size Size of buffer
 */
void get_terminal_all(char *buffer, size_t size);

/**
 * Will search the database for a terminal with a specific id
 * @param terminal_t Structure containing the terminal ID to be searched
 *                   and internal buffers to receive the information stored
 * @return ERROR_NO Terminal ID was found. Any other value represents an error.
 */
error_t get_terminal(terminal_t *terminal);

/**
 * Will add a new terminal into database and allocate a unique ID to it
 * The new ID will be returned in ID field from terminal struct variable
 * @param terminal_t Structure containing the terminal ID to be searched
 *                   and internal buffers to receive the information stored
 * @return ERROR_NO Terminal was inserted. Any other value represents an error.
 */
error_t add_terminal(terminal_t *terminal);

#endif /* DATABASE_H_ */
