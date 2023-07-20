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




    //set up select
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(listenfd, &read_fds);
    //FD_SET(pipe_child1[0], &read_fds);
    //FD_SET(pipe_child2[0], &read_fds);

    //message read to client
    char msg1[MAXLINE];
    char msg2[MAXLINE];
    char msg3[MAXLINE];
    // used for select and I will change it later;
    int max_fd = 0;
    // set up a counter to see how many client we have arrived.
    // handle many client late, int count;
    //set up accept for client;
    struct sockaddr_in client_addr;
    unsigned int client_len = sizeof(struct sockaddr_in);
    client_addr.sin_family = AF_INET;
    //存疑, 这里应该要用select;
    // change it later;
    int client_socket = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);
    if (client_socket == -1) {
        perror("accept");
        return -1;
    }
    //2. use select to see if socket fd is ready.
    if (select(client_socket + 1, &read_fds, NULL, NULL, NULL) == -1){
        perror("select:");
        exit(1);
    }
    // 3. if yes, read http request.
    if(FD_ISSET(client_socket, &read_fds)){
        int read_byte;
        int pipe_fd;
        char* slice;
        int i;
        // extract http before \r\n\r\n.
        read(client_socket, msg1, MAXLINE);
        // 4. extract http request before \r\n.

        // remember to add while loop for read.
        // TODO: add while loop and create different clients;
        slice = strstr(msg1, "\r\n");
        strncpy(msg1, msg1, strlen(msg1) - strlen(slice));
        msg1[msg1 - slice - 1] = '\0';
        // 5. add request to cs->request
        client ->request = msg1;
        // the case is get.
        if ((i = handleClient(&client[0], msg1)) == 1){

            client[0]. sock = client_socket;
            // then we process the request
            if ((pipe_fd = processRequest(&client[0])) == -1){
                perror("processRequest:");
            }
            // send client state to sever;
            // set malloc for client later;
            // &client[0] = malloc(sizeof(struct clientstate));
            FD_SET(pipe_fd, &read_fds);
            // ******  send client status to server
            // write(listenfd, client[0], MAXLINE);
            if (select(max_fd, &read_fds,NULL,NULL,NULL) == -1){
                perror("select");
                exit(1);
            }
            if(FD_ISSET(pipe_fd, &read_fds)){
                read(pipe_fd, msg2, MAXLINE);
                msg2[MAXLINE - 1] = '\0';
            }

            // TODO: check termination status, update client status and write client status to server, then remove
            //       client_fd and pipe_fd then close client socket;
            //Hang on
            // hand many client late handleClient(&client[count], msg);
            write(client_socket, msg2, MAXLINE);
        }
        else if ( i == -1){
            close(client_socket);
            if(client -> request != NULL){
                free(client -> request);
                client -> request = NULL;
            }
        }
        // handle i == o case later

    }
    else{ perror("select");}
    // TODO: complete this function

    // ***** client socket





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

    // If the resource is favicon.ico we will ignore the request
    if(strcmp("favicon.ico", cs->path) == 0){
        // A suggestion for debugging output
        fprintf(stderr, "Client: sock = %d\n", cs->sock);
        fprintf(stderr, "        path = %s (ignoring)\n", cs->path);
        printNotFound(cs->sock);
        return -1;
    }

    if (strstr(line, "\r\n\r\n") == NULL && strstr(line, "\n\n") == NULL){
        return 0;
    }
    else if (strstr(line, "GET") == NULL){
        return -1;
    }
    else{
        cs->request = line;
        cs->path = getPath(line);
        cs->query_string = getQuery(line);
        // cs->output = cs->output;
        // cs->optr = cs->optr;
        fprintf(stderr, "Client: sock = %d\n", cs->sock);
        fprintf(stderr, "        path = %s\n", cs->path);
        fprintf(stderr, "        query_string = %s\n", cs->query_string);
        return 1;
    }

}
