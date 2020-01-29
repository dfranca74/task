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

error_t create_server_thread(void);
void wait_server_thread(void);

#endif /* SERVER_H_ */
