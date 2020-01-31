/*
 * endpoints_handler.h
 *
 *  Created on: 31/01/2020
 *      Author: dfranca
 */

#ifndef ENDPOINTS_HANDLER_H_
#define ENDPOINTS_HANDLER_H_

#include <microhttpd.h>

/**
 * Handler that will send a greetings message to the client (to test if server is online)
 * @param connection Connection to send the greetings page
 * @return #MHD_NO on errors, #MHD_YES on success
 */
int endpoint_greetings(struct MHD_Connection *connection);

/**
 * Handler that will search for specific terminal in the database.
 * If the terminal was inserted into database its values (card type and transaction type) will be returned to the client
 * @param connection Connection to send the terminal info
 * @param url String to be parsed and find the terminal ID
 * @return #MHD_NO on errors, #MHD_YES on success
 */
int endpoint_terminal_read_id(struct MHD_Connection *connection, const char *url);

/**
 * Handler that will send to client a list of all terminals stored in the database
 * @param connection Connection to send the terminal info
 * @return #MHD_NO on errors, #MHD_YES on success
 */
int endpoint_terminal_read_all(struct MHD_Connection *connection);

/**
 * Handler that will process a POST message to include a new terminal into database
 * @param connection Connection to send the terminal info
 * @param con_cls pointer set to some address and that will be
 *        preserved by MHD for future calls for this request;
 *        since the access handler may be called many times
 *        (i.e., for a PUT/POST operation with plenty of upload data)
 *        this allows the application to easily associate some request-specific state.
 * @param upload_data the data being uploaded
 * @param upload_data_size set initially to the size of the upload_data provided;
 */
int endpoint_terminal_add(struct MHD_Connection *connection, void **con_cls, const char *upload_data, size_t *upload_data_size);

#endif /* ENDPOINTS_HANDLER_H_ */
