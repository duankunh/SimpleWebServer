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


    // Set up the socket to which the clients will connect
    // Create and set up a socket for a server to listen on.
    listenfd = setupServerSocket(port);
    // ***** listening server
    // create max number of client
    initClients(client, MAXCLIENTS);


    while (count < MAXCLIENTS) {
        char msg1[MAXLINE];
        char msg2[MAXLINE];
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        unsigned int addrlen = sizeof(struct sockaddr_in);
        int pipe_fd;
        int max_fd;
        fd_set read_fds;
        FD_ZERO(&read_fds);
        // add client_descriptor to socket

        FD_SET(listenfd, &read_fds);
        max_fd = listenfd + 1;
        if(select(max_fd, &read_fds, NULL, NULL, NULL) == -1){
            perror("select");
            exit(1);
        }
        int client_socket = accept(listenfd, (struct sockaddr *) &addr, &addrlen);
        if (client_socket == -1) {
            perror("accept");
            return -1;
        }
        FD_SET(client_socket, &read_fds);

        client[count].sock = client_socket;
        // max_fd hasn't full implemented, I will change it later.
        max_fd = client_socket + 1;
        // return the result of pipe_fd

        while (1) {
            // to see if select runs in order
            if (select(max_fd, &read_fds, NULL, NULL, NULL) == -1){
                perror("select:");
                exit(1);
            }
            if (FD_ISSET(client_socket, &read_fds)) {
                read(client_socket, msg1, MAXLINE);
                while (handleClient(&client[count], msg1) == 0) {
                    read(client_socket, msg2, MAXLINE);
                    strcat(msg1, msg2);
                    fprintf(stderr, "curr input: %s\n", msg1);
                }
                break;
            }
            fprintf(stderr, "waiting for input\n");
            sleep(500);
        }

        if ((pipe_fd = processRequest(&client[count])) == -1){
            perror("processRequest:");
        }


        // select to see if process_value have something to write
        if (pipe_fd != -1) {
            // deal with max fd later;
            int max_fd_tep = pipe_fd + 1;
            FD_SET(pipe_fd, &read_fds);
            select(max_fd_tep, &read_fds, NULL, NULL, NULL);
            if (FD_ISSET(pipe_fd, &read_fds)) {
                char msg3[MAXLINE];
                int i = read(pipe_fd, msg3, MAXLINE);
                fprintf(stderr, "%s\n", msg3);
                client[count].output = msg3;
                printOK(client[count].sock, client[count].output, i);
            }
        }


        Close(client[count].sock);
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


    // TODO: Complete this function
    if (strstr(line, "\r\n\r\n") == NULL && strstr(line, "\n\n") == NULL){
        return 0;
    }
    else if (getPath(line) == NULL || getQuery(line) == NULL){
        return -1;
    }
    else{
        cs->request = line;
        cs->path = getPath(line);
        cs->query_string = getQuery(line);
        cs->output = cs->output;
        cs->optr = cs->optr;
        return 1;
    }



    // If the resource is favicon.ico we will ignore the request
    if(strcmp("favicon.ico", cs->path) == 0){
        // A suggestion for debugging output
        fprintf(stderr, "Client: sock = %d\n", cs->sock);
        fprintf(stderr, "        path = %s (ignoring)\n", cs->path);
        printNotFound(cs->sock);
        return -1;
    }


    // A suggestion for printing some information about each client.
    // You are welcome to modify or remove these print statements
    fprintf(stderr, "Client: sock = %d\n", cs->sock);
    fprintf(stderr, "        path = %s\n", cs->path);
    fprintf(stderr, "        query_string = %s\n", cs->query_string);

    return 1;
}
//
// Created by 段焜涵 on 2022-04-08.
//

