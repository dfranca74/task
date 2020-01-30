/*
 * utils.c
 *
 *  Created on: 31/01/2020
 *      Author: dfranca
 */

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "errors.h"

error_t convert_str_to_int(const char *str, long int *value)
{
    error_t result = ERROR_NO;

    char *endptr;
    errno = 0;
    const long int val = strtol(str, &endptr, 10);

    // http://man7.org/linux/man-pages/man3/strtol.3.html
    // Check for possible conversion errors:

    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0))
    {
        printf("%s - failed string to number conversion: %s", __func__, strerror(errno));
        return ERROR_INVALID_CONVERSION;
    }

    if (endptr == str)
    {
        printf("%s - failed string to number conversion: no digit found", __func__);
        exit(EXIT_FAILURE);
    }

    *value = val;
    return result;
}
