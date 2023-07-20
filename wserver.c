#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>    /* Internet domain header */

#include "wrapsock.h"
#include "ws_helpers.h"

#define MAXCLIENTS 10

int handleClient(struct clientstate *cs, char *line);

// You may want to use this function for initial testing
//void write_page(int fd);

int main(int argc, char **argv) {

    if(argc != 2) {
        fprintf(stderr, "Usage: wserver <port>\n");
        exit(1);
    }
    unsigned short port = (unsigned short)atoi(argv[1]);
    int listenfd;
    struct clientstate client[MAXCLIENTS] ;
    int count = 0;
    // time_interval aim to set select wait for 5 minutes.
    struct timeval time_interval;
    time_interval.tv_usec = 0;
    time_interval.tv_sec = 300;


    // Set up the socket to which the clients will connect
    // Create and set up a socket for a server to listen on.
    listenfd = setupServerSocket(port);
    // ***** listening server
    // create max number of client
    initClients(client, MAXCLIENTS);

    // after there are more than 10 connection, the while loop end and program exit.
    while (count < MAXCLIENTS) {
        // msg1 is a var that reads request from client.
        char msg1[MAXLINE];
        // msg2
        char msg2[MAXLINE];
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        unsigned int addrlen = sizeof(struct sockaddr_in);
        int pipe_fd;
        int max_fd;
        int handle;
        fd_set read_fds;
        FD_ZERO(&read_fds);
        // add client_descriptor to socket

        FD_SET(listenfd, &read_fds);
        // find max file descriptor.
        max_fd = listenfd + 1;
        // check if select is over 5 minutes and do the error checking.
        if(Select(max_fd, &read_fds, NULL, NULL, &time_interval) == 0){
            exit(0);
        }
        //to see if listenfd is ready
        if(FD_ISSET(listenfd, &read_fds)){
            // begin to build connection
            int client_socket = accept(listenfd, (struct sockaddr *) &addr, &addrlen);
            // check connection error
            if (client_socket == -1) {
                perror("accept");
                exit(1);
            }
            // add client_socket to select set.
            FD_SET(client_socket, &read_fds);
            // set sock of client
            client[count].sock = client_socket;
            // set max number of file descriptor for select
            max_fd = client_socket + 1;
            // create endless loop for receiving information from web.
            while (1) {
                // to see if select runs in order
                if (Select(max_fd, &read_fds, NULL, NULL, &time_interval) == 0){
                    exit(0);
                }
                if (FD_ISSET(client_socket, &read_fds)) {
                    // get http page from web
                    read(client_socket, msg1, MAXLINE);
                    while ((handle = handleClient(&client[count], msg1)) == 0) {
                        read(client_socket, msg2, MAXLINE);
                        strcat(msg1, msg2);
                        fprintf(stderr, "current input: %s\n", msg1);
                    }
                    break;
                }

            }

            // check pipe error
            if ((pipe_fd = processRequest(&client[count])) == -1){
                perror("processRequest:");
            }
            // deal the special case with term;
            if (strcmp(client[count] . path, "term")== 0){
                printServerError(client[count].sock);
                pipe_fd = -1;

            }
            if (pipe_fd != -1) {
                // deal with max fd ;
                int max_fd_tep = pipe_fd + 1;
                // the var read_byte is used to store the number of bytes that read call returns
                int read_byte;
                // add pipe descriptor to select list
                FD_SET(pipe_fd, &read_fds);
                // to see if the select is time out
                if(Select(max_fd_tep, &read_fds, NULL, NULL, &time_interval)
                == 0){
                    exit(0);
                }
                // to see if pipe_fd is ready to read
                if (FD_ISSET(pipe_fd, &read_fds)) {
                    // msg3 is the vat that reads from pipe_fd
                    char msg3[MAXPAGE];
                    //msg4 collects the byte that msg3 has
                    char msg4[MAXPAGE];
                    // there are the same idea from a3 to deal with large file.
                    while ((read_byte = read(pipe_fd, msg3, MAXLINE)) > 0){
                        strncat(msg4, msg3, read_byte);
                    }
                    // set null terminator to msg4
                    strncat(msg4, "\0", MAXPAGE);
                    fprintf(stderr, "%s\n", msg4);
                    client[count].output = msg4;
                    // send response back to client
                    printOK(client[count].sock, client[count].output, strlen(msg4));
                    // clean up the resources in order to avoid duplicated response when
                    strcpy(msg3, "");
                    strcpy(msg4, "");
                }
            }




        }

        // clean up the resources
        strcpy(msg1, "");
        strcpy(msg2, "");
        // close the sock after using it.
        Close(client[count].sock);
        // then turn to next client
        count += 1;
    }


    return 0;
}


/* Update the client state cs with the request input in line.
 * Intializes cs->request if this is the first read call from the socket.
 * Note that line must be null-terminated string.
 *
 * Return 0 if the get request message is not complete and we need to wait for
 *     more data
 * Return -1 if there is an error and the socket should be closed
 *     - Request is not a GET request
 *     - The first line of the GET request is poorly formatted (getPath, getQuery)
 * 
 * Return 1 if the get request message is complete and ready for processing
 *     cs->request will hold the complete request
 *     cs->path will hold the executable path for the CGI program
 *     cs->query will hold the query string
 *     cs->output will be allocated to hold the output of the CGI program
 *     cs->optr will point to the beginning of cs->output
 */
int handleClient(struct clientstate *cs, char *line) {


    if (strstr(line, "\r\n\r\n") == NULL && strstr(line, "\n\n") == NULL){
        return 0;
    }
    else if (getPath(line) == NULL || getQuery(line) == NULL){
        return -1;
    }
    else{
        cs->request = line;
        cs->path = getPath(line);
        // If the resource is favicon.ico we will ignore the request
        if(strcmp("favicon.ico", cs->path) == 0){
            // A suggestion for debugging output
            fprintf(stderr, "Client: sock = %d\n", cs->sock);
            fprintf(stderr, "        path = %s (ignoring)\n", cs->path);
            printNotFound(cs->sock);
            return -1;
        }
        if(!validResource(cs -> path)){
            printNotFound(cs -> sock);
            exit(-1);
        }

        cs ->query_string = getQuery(line);

        // A suggestion for printing some information about each client.
        // You are welcome to modify or remove these print statements
        fprintf(stderr, "Client: sock = %d\n", cs->sock);
        fprintf(stderr, "        path = %s\n", cs->path);
        fprintf(stderr, "        query_string = %s\n", cs->query_string);

        return 1;

    }


}

