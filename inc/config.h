/*
 * config.h
 *
 *  Created on: 29/01/2020
 *      Author: dfranca
 */

#ifndef CONFIG_H_
#define CONFIG_H_

// Server Configuration:
#define SERVER_PORT 9876

// Database Configuration:
#define MAX_DATABASE_REGISTERS  50
#define MAX_VALUE_SIZE          32  // Includes space for C end of string terminator (0x00)

// Transaction types:
#define CHEQUE  "Cheque"
#define CREDIT  "Credit"
#define SAVINGS "Savings"

// Card types:
#define VISA    "Visa"
#define MASTER  "MasterCard"
#define EFTPOS  "EFTPOS"

#endif /* CONFIG_H_ */
