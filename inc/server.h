/*
 * server.h
 *
 *  Created on: 29/01/2020
 *      Author: dfranca
 */

#include <stdint.h>
#include "config.h"
#include "errors.h"

#ifndef SERVER_H_
#define SERVER_H_

/**
 * Create a server thread to run the libmicrohttpd daemon
 * @return ERROR_NO Success creating the tread. Any other value represents an error
 */
error_t create_server_thread(void);

/**
 * Wait the server thread to finish
 */
void wait_server_thread(void);

/**
 * Signal the server thread to exit
 */
void exit_server_thread(void);

#endif /* SERVER_H_ */
