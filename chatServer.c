#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <ctype.h>

#define MAXCHARACTER 4096
#define BACKLOG 30



void * connectionHandler(void *);
void messageParcer(char client_message[MAXCHARACTER]);

int main(int argc, char *argv[])
{
        int svr_sockfd;
        int svr_portno;
        struct sockaddr_in svr_addr, cli_addr;
        socklen_t clilen;
        int cli_sockfd;
        int *new_sockfd;
	    int too_many = 0;	// too many clients connected

        if (argc != 2)
        {
                fprintf(stderr, "usage: %s <svr_port>\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        svr_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (svr_sockfd < 0)
        {
                perror("socket");
                exit(EXIT_FAILURE);
        }

        svr_portno = atoi(argv[1]);

        bzero((char *) &svr_addr, sizeof(svr_addr));
        svr_addr.sin_family = AF_INET;
        svr_addr.sin_addr.s_addr = INADDR_ANY;
        svr_addr.sin_port = htons(svr_portno);

        // set sock for reuse of the same socket.
        int on = 1;
	    setsockopt(svr_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

        if (bind(svr_sockfd, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) < 0)
        {
                perror("bind");
                exit(EXIT_FAILURE);
        }

        listen(svr_sockfd, BACKLOG);

        puts("Waiting for Incoming Connections...");

        clilen = sizeof(cli_addr);

        while (cli_sockfd = accept(svr_sockfd, (struct sockaddr *) &cli_addr, &clilen))
        {
                pthread_t client_thread;
                new_sockfd = malloc(sizeof(int));
                *new_sockfd = cli_sockfd;

		//printf("new connection, socket fd is %d\n", cli_sockfd);

		if (pthread_create(&client_thread, NULL, connectionHandler, (void *) new_sockfd) < 0)
		{
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}

		
        }

        if (cli_sockfd < 0)
        { 
                perror("accept");
                exit(EXIT_FAILURE);
        }

        free(new_sockfd);
        return 0;
}

void * connectionHandler(void * sockfd)
{
        int cli_sockfd = *(int *) sockfd;
        int nread;
        char * ip; 
        char * port; 
        
        char * tok[50];
        char client_message[MAXCHARACTER];
        char welcome_message[] = {"Welcome to the 3600 Chat Room \n"};
        int listSize = 0;
        char * euid[listSize];

        
        bzero(client_message, MAXCHARACTER);
        // welcome message to client
        write(cli_sockfd, welcome_message, strlen(welcome_message));
        recv(cli_sockfd, client_message, MAXCHARACTER, 0);

        int i = 0;
        tok[0] = strtok(client_message, " ");
        
        while(tok[i] != NULL) {
            ++i;
            tok[i] = strtok(NULL, " ");
        }
        ip = tok[i-3]; port = tok[i-2]; euid[listSize] = tok[i-1];
        //printing out the information when a new connection is made will only be called if a new thread and connection is made.
        
        printf("\n New connection, socket fd is %d, ip is %s, port is %s\n", cli_sockfd, ip, port);
        printf( "User %s has logged in\n", euid[listSize]);
        ++listSize;
        // bzero(client_message, MAXCHARACTER);

        while((nread = recv(cli_sockfd, client_message, MAXCHARACTER, 0)) > 0)
        {

            
            char * tok[50];
            int i = 0;
            tok[0] = strtok(client_message, ":\n");
        
            while(tok[i] != NULL) {
                ++i;
                tok[i] = strtok(NULL, ":\n");
            }
            char * command1 = tok[0];
            char * command2;
            char * command3;
            
            if(i > 1) {

                command2 = tok[1];
                command3 = tok[2];
            }

            if(command1 = "LIST") {
                
                for(int x = 0; x < listSize; ++x){

                    write(cli_sockfd, euid[x], listSize);
            }
            
            if(command1 = "QUIT") {

                nread = 0;
                break;
            }
                bzero(client_message, MAXCHARACTER);
        }

	    // client connection can be reset by peer, so just disconnect cleanly
        if (nread <= 0)
        {
		    printf("User %s has logged out\n", euid[listSize]);
            fflush(stdout);
        }

        free(sockfd);

        return 0;
    }
}
