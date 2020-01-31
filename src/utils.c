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
#include "utils.h"

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

int handle_helper_pages(struct MHD_Connection *connection, const char *description)
{
    const size_t len = strlen(description);
    void *page = (void*)(description);

    struct MHD_Response *response = MHD_create_response_from_buffer (len, page, MHD_RESPMEM_MUST_COPY);

    if (!response)
    {
        printf("%s - Failed MHD_create_response_from_buffer\n", __func__);
        return MHD_NO;
    }

    const int ret = MHD_queue_response (connection, MHD_HTTP_OK, response);

    if (MHD_YES != ret)
    {
        printf("%s - Failed MHD_queue_response\n", __func__);
    }

    MHD_destroy_response (response);

    return ret;
}
