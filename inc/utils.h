/*
 * utils.h
 *
 *  Created on: 31/01/2020
 *      Author: dfranca
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <microhttpd.h>
#include "errors.h"
#include "config.h"

/**
 * Model the terminal information
 */
typedef struct
{
    long int id;
    char card[MAX_VALUE_SIZE];
    char transaction[MAX_VALUE_SIZE];
} terminal_t;

/**
 * Following struct will be used to handle a POST command
 * Reference: https://www.gnu.org/software/libmicrohttpd/tutorial.html#Processing-POST-data
 */
typedef struct
{
    struct MHD_PostProcessor *pp;

   terminal_t terminal;

} connection_info_struct_t;

/**
 * Safe conversion from C string to integer
 * @param str C Striung to be converted into integer
 * @param value Converted value will be stored in this variable
 * @return Success or error code in case the conversion was not valid
 */
error_t convert_str_to_int(const char *str, long int *value);

/**
 * Auxiliary function that will prepare a page to send back to the client
 * @param connection Connection to send the page to
 * @param helper C string containing the data to be sent to the client
 * @return #MHD_NO on errors, #MHD_YES on success
 */
int handle_helper_pages(struct MHD_Connection *connection, const char *helper);

#endif /* UTILS_H_ */
