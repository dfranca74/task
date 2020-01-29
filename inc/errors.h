/*
 * errors.h
 *
 *  Created on: 29/01/2020
 *      Author: dfranca
 */

#ifndef ERRORS_H_
#define ERRORS_H_

typedef enum
{
    ERROR_NO = 0,
    ERROR_CREATE_THREAD,
    ERROR_DATABASE_FULL,
    ERROR_INVALID_ARGUMENT,
    ERROR_TERMINAL_NOT_FOUND

} error_t;

#endif /* ERRORS_H_ */
