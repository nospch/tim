/*
**  tim (Terminal Instant Messenger)
**
**  Written by nospch
**  Nov 2020
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

//for close() on linux
#include <unistd.h>

int main(int argc, char** argv);

//create connecting socket and attempt to connect to external socket
int connect_to(char* target_ip, char* port);
//create listening socket and wait on connection from external socket
int listen_on(char* port);

//once connection is established, split program into two threads:
//one to receive messages, and the other to send messages
int open_chat(int socketfd);

void* receive_message(void* socketfd);
void* send_message(void* socketfd);

int main(int argc, char** argv) {

    //generic argument error message
    char* arg_err = "Error handling arguments. Please try again. "
                    "Ending program.\n\n";

    //if more or fewer arguments than expected are passed
    if(argc != 3) {
        fprintf(stderr, "%s", arg_err);
        return EXIT_FAILURE;
    }
    else {
        //if the first arg is "connect"
        if(strcmp(argv[1], "connect") == 0) {
            //connect to peer
            fprintf(stdout, "Connecting to %s via TCP...\n", argv[2]);

            //initialize temporary arg, ip address and port variables
            char* temp_arg = argv[2],
                * ip_addr,
                * port;

            //isolate ip address from port number
            ip_addr = strtok(temp_arg, ":");
            port = strtok(NULL, ":");

            //call connect_to() and check for error
            if(connect_to(ip_addr, port) == 8) {
                return EXIT_FAILURE;
            }
        }
        //if the first arg is "listen"
        else if(strcmp(argv[1], "listen") == 0) {
            //listen for connection
            fprintf(stdout, "Listening on port %s...\n", argv[2]);

            //initialize port variable
            char* port = argv[2];

            //call listen_on() and check for error
            if(listen_on(port) == 8) {
                return EXIT_FAILURE;
            }
        }
        //if the first arg is other than expected
        else {
            fprintf(stderr, "%s", arg_err);
            return EXIT_FAILURE;
        }
    }

    fprintf(stdout, "Program ended successfully.\n");
    return EXIT_SUCCESS;
}

int connect_to(char* target_ip, char* port) {

    //create connecting socket
    int csock = socket(AF_INET, SOCK_STREAM, 0);

    //bind the connecting socket to a sockaddr_in structure
    struct sockaddr_in target_addr;
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(atoi(port));
    inet_aton(target_ip, &target_addr.sin_addr.s_addr);
    
    //attempt to connect to the ip/port pair passed as arguments
    int connection_status = connect(csock, (struct sockaddr*)&target_addr, sizeof(target_addr));
    if(connection_status == -1) {
        //connection was unsuccessful, exit function
        fprintf(stderr, "Error connecting to %s\n", target_ip);
        return EXIT_FAILURE;
    }

    //inform user that the connection was successful
    fprintf(stdout, "You have successfully connected to %s\n", target_ip);

    //call open_chat() and check for error
    if(open_chat(csock) == 8) {
        return EXIT_FAILURE;
    }

    //close socket and exit function successfully
    close(csock);
    return EXIT_SUCCESS;
}

int listen_on(char* port) {

    //create listening socket
    int lsock = socket(AF_INET, SOCK_STREAM, 0);

    //create sockaddr_in structure to bind to listening socket
    struct sockaddr_in host_addr;
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(atoi(port));
    host_addr.sin_addr.s_addr = INADDR_ANY;

    //create sockaddr_in structure to hold ip address of connecting peer
    struct sockaddr_in peer_addr;
    int peer_addr_len;

    //bind listening socket to host_addr
    bind(lsock, (struct sockaddr*)&host_addr, sizeof(host_addr));

    //begin listening
    int clisten = listen(lsock, 3);
    if(clisten == -1) {
        fprintf(stderr, "Error listening for connection.\n");
    }

    //accept connecting socket and check for error
    int csock = accept(lsock, (struct sockaddr*)&peer_addr, &peer_addr_len);
    if(csock == -1) {
        fprintf(stderr, "Error connecting to peer.\n");
        return EXIT_FAILURE;
    }

    //inform user that the connection was successful
    fprintf(stdout, "You have successfully connected to %s\n", inet_ntoa(peer_addr.sin_addr));

    //call open_chat() and check for error
    if(open_chat(csock) == 8) {
        return EXIT_FAILURE;
    }

    //close socket and exit function successfully
    close(lsock);
    return EXIT_SUCCESS;
}

int open_chat(int socketfd) {

    fprintf(stdout, "Opened chat. Say hello.\n\n");

    //create two threads: one to listen for messages, and the other to send messages
    pthread_t receive,
              send;
    
    pthread_create(&receive, NULL, receive_message, &socketfd);
    pthread_create(&send, NULL, send_message, &socketfd);

    //wait for receive thread to terminate to close function
    pthread_join(receive, NULL);
    pthread_cancel(send);

    return EXIT_SUCCESS;
}

void* receive_message(void* socketfd) {

    int* psockfd = (int*)socketfd;

    //variable to hold message received
    char message_in[280];

    //receive loop
    while(1) {
        //write received message to message_in
        recv(*psockfd, message_in, sizeof(message_in), 0);

        //check if message received is the quit code
        if(strstr(message_in, "^Q") != NULL) {
            fprintf(stdout, "Peer disconnected.\n");
            send(*psockfd, "^Q", sizeof("^Q"), 0);
            return NULL;
        }

        fprintf(stdout, "Message received:\n%s\n", message_in);
    }

    return NULL;
}

void* send_message(void* socketfd) {
    
    int* psockfd = (int*)socketfd;

    //variable to hold message to be sent
    char message_out[280];

    //send loop
    while(1) {
        //write input from user to message_out
        fgets(&message_out, 280, stdin);
        fprintf(stdout, "\n");

        //check for quit code
        if(strstr(message_out, "^Q") != NULL) {
            send(*psockfd, "^Q", sizeof("^Q"), 0);
            return NULL;
        }

        send(*psockfd, message_out, sizeof(message_out), 0);
    }

    return NULL;
}