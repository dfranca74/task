a) Development environment:

I have used the Eclipse CDT version cpp-2019-12 with internal management of Makefiles.
My eclipse config and settings file were also pushed into the GitHub. 
I suggest you opening the solution in Eclipse to build it locally.

My virtual machine runs Debian:
Linux dfranca 4.9.0-11-amd64 #1 SMP Debian 4.9.189-3+deb9u2 (2019-11-11) x86_64 GNU/Linux

My Eclipse configuration is searching the libmicrohttpd header files in this folder: /usr/include/
And the shared library here: /usr/lib/x86_64-linux-gnu/libmicrohttpd.so -> libmicrohttpd.so.12.39.0

I also had to install cURL to send GET and POST commands to my server.

In the repository you will find in the Debug folder the latest application binary I have generated.
To run it just type: ./task
To stop it: send a CTRL+C signal to the application

b) Links

The public project is hosted here:
https://github.com/dfranca74/task

To clone it:
git clone git@github.com:dfranca74/task.git

During study and development I also used these sites as references:

https://github.com/scottjg/libmicrohttpd/blob/master/src/examples/post_example.c
https://git.taler.net/exchange.git/tree/src/bank-lib/fakebank.c
https://lists.gnu.org/archive/html/libmicrohttpd/2018-05/msg00001.html
https://git.gnunet.org/gnunet.git/tree/src/json/json_mhd.c?h=v0.12.2

c) Scripts to test the server

You will find inside the script folder a set of auxiliary bash scripts that will send cURL commands to the server.
After starting the server (./task) we can use the scripts:

c.1) sh greetings.sh
Will test if the server is online. A greeting message with a unique id will be sent back to cURL.

c.2) sh add_terminal.sh "card" "transaction"
Examples:
sh add_terminal Visa Savings
sh add_terminal MasterCard Cheque

For each command the server will send back a page with the unique ID associated with the new terminal and the "card" and "transactions" values
This new terminal will be stored in the database.

c.3) sh add_multiple_cards.sh
Usage: sh add_multiple_cards.sh
This script will send a set of commands to add new terminals (with differente combinations of "card" and "transactions")
As before, for each new terminal the server will choose a unique ID and send back to the client its value.

c.4) sh add_terminal_wrong_format.sh
This test will send a set of wrong formatted POST commands.
The server will return a message indicating the terminal(s) were not created.

c.5) sh get_terminal_all.sh
This script will ask the server to send back the list of all terminals stored in the database, including the unique ID associated with each one.

c.6) sh get_terminal.sh
Usage: sh get_terminal.sh 'value' ---> Where value is a numeric representation of a ID.
Example: sh get_terminal.sh 1   (if exist a terminal with this ID in the database its data will be read and sent to the client).
         sh get_terminal.sh 5   

If the terminal is not stored in the database a error message will be sent back to client.

d) Things I did not accomplish:

I did not have time to implement the JSON logic to parse the commands. I have decided to use a KEY=VALUE scheme for both POST and GET.
I know this is not what the requirement asked. However, I took the decision to finish something to show and be analysed. To fully implement
the JSON logic I would need more time to do this.
I thing the actual logic of post processing is close to store in a buffer the raw JSON data sent by a POST. Once the data is stored in a buffer,
my plan would be using an external JSON library, for example JSSON, to decode and acess the raw data. The same library would be used
to create the messages to send back to the clients.

I also did not write any unit-tests. Again, as the JSON parser I decided to focus 100% on the code and have something working to be analysed and not write unit-tests. On the other hand I tried to write modular functions to make the process of unit-testing them less painfull.

I have focused more on the software engineering aspect of this solution. Tried to write a code that was simple and easy to read and maintain.
Also tried to write modular functions to make the process of adding JSON parsing less traumatic and painfull.

Basically is this: If I had more time I would certainly invest heavilly in unit-tests. I also would prefer to use an stable and minimal JSON library to do the parsing, instead of doing this parsing manually. The input data seems to not be difficult, however the points of potential error during the parser are high and using an open-source library, already tested by the community, should add benefits to the project (of course the question of the library license and its footprint should be carefully analysed).







