#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/select.h>

#define STDIN 0

char ipstr[INET_ADDRSTRLEN];
char newClientIP[INET_ADDRSTRLEN];
char myHostname[1024];
char service[50];
char serverIPList[500];
int sizeOfList;
int totalConnected;


void *get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct serverIPList {
	char pNo[10];
	char address[50];
	int connID;
	char hostname[50];
};

struct serverIPList list[5];

#define N sizeof list / sizeof list[0]

void getPublicIP(){
	struct addrinfo hints, *res;
	int status, sock;
	struct sockaddr_in current;
	socklen_t len = sizeof current;
	//char *name = (char *) malloc(32);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	if(status = getaddrinfo("8.8.8.8", "53", &hints, &res) !=0){
		displayHelp("Can't get public IP");
	}
	void *addr;
	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sock == -1)
		perror("socket");
	if(connect(sock, res->ai_addr, res->ai_addrlen) == -1)
		perror("connect");
	if(getsockname(sock, (struct sockaddr*)&current, &len) == -1)
		perror("getsockname");
	addr = &(current.sin_addr);
	if(inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr)) == NULL)
		perror("inet_ntop");
	ipstr[strlen(ipstr)] = 0;
	getnameinfo((struct sockaddr*)&current, sizeof current, myHostname, sizeof myHostname, service, sizeof service, 0);
	// printf("%s", ipstr);
	//printf("\n");

}

main(int argc, char **argv)
{
	char cArg[] = "c";
	char sArg[] = "s";
	char command[50];
	int i;

	if(argc < 3)
	{
		invalidArgs();
		exit(-1);
	}

	if(strcasecmp (cArg,argv[1]) == 0)
	{
		printf("In client\n");
		printf("Enter a command: (Type HELP for usage)\n >>> \n");
		fd_set master;
		fd_set read_fds;
		int fdmax;

		int listener;
		int newfd;
		struct sockaddr_storage remoteaddr;
		socklen_t addrlen;

		char buf[256];
		int nbytes;

		char remoteIP[INET6_ADDRSTRLEN];
		getPublicIP();

		int yes = 1;
		int i, j, rv;

		struct addrinfo hints, *ai, *p;

		FD_ZERO(&master);
		FD_ZERO(&read_fds);
		
		
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;
		
		if((rv = getaddrinfo(NULL, argv[2], &hints, &ai)) != 0)
		{
			perror("getaddrinfo");
			displayHelp();
		}
		for(p = ai; p != NULL; p = p->ai_next)
		{
			listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
			if(listener<0){
				perror("socket");
				continue;
			}

			setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
			
			if(bind(listener, p->ai_addr, p->ai_addrlen)<0)
			{
				perror("bind");
				close(listener);
				continue;
			}

			break;
		}
		
		if(p == NULL){
			printf("Can't bind server\n");
		}
		
		freeaddrinfo(ai);
		
		//Listening
		
		if(listen(listener, 10) == -1)
		{
			displayHelp();
		}

		FD_SET(STDIN, &master);
		FD_SET(listener, &master);

		fdmax = listener;

		for(;;) {
	        read_fds = master; // copy it
	        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
	            perror("select");
	            exit(4);
        	}

        	// run through the existing connections looking for data to read
	        for(i = 0; i <= fdmax; i++) {
	            if (FD_ISSET(i, &read_fds)) {
	                if(i == STDIN)
					{
						// printf("input from command line\n");
						char command[50];
						fgets(command, sizeof(command), stdin);
						char * newOne;
						char * newTwo;
						
						// pch = strtok (command," ");

						char * pch;
						pch = strtok (command," ,.-\n\t");
						//printf ("%s\n",pch);
						pch[strlen(pch)] = '\0';


						if(strcasecmp("register", pch) == 0)
						{
							//printf("In register\n");
							newOne = strtok (NULL, " ");
							// newOne[strlen(newOne)-1] = '\0'; 
							newTwo = strtok (NULL, " ");
							newTwo[strlen(newTwo)-1] = '\0'; 
							printf("%s and %s \n", newOne, newTwo);

							int sockfd, numbytes;  
							char buf[500];
							struct addrinfo hints, *servinfo, *p;
							int rv;
							char s[INET6_ADDRSTRLEN];

							memset(&hints, 0, sizeof hints);
							hints.ai_family = AF_UNSPEC;
							hints.ai_socktype = SOCK_STREAM;

							if ((rv = getaddrinfo(newOne, newTwo, &hints, &servinfo)) != 0) {
								fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
								return 1;
							}

							// loop through all the results and connect to the first we can
							for(p = servinfo; p != NULL; p = p->ai_next) {
								if ((sockfd = socket(p->ai_family, p->ai_socktype,
										p->ai_protocol)) == -1) {
									perror("client: socket");
									continue;
								}

								if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
									close(sockfd);
									perror("client: connect");
									continue;
								}

								break;
							}

							if (p == NULL) {
								fprintf(stderr, "client: failed to connect\n");
								return 2;
							}

							inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
									s, sizeof s);
							printf("client: connecting to %s\n", s);

							freeaddrinfo(servinfo); // all done with this structure

							char sendBuffer[256];
							strcpy(sendBuffer,myHostname);
							strcat(sendBuffer,"\t");
							strcat(sendBuffer, argv[2]);
  							send(sockfd,sendBuffer,sizeof(sendBuffer),0);
  							for(;;)
  							{
								if ((numbytes = recv(sockfd, buf, 500-1, 0)) == -1) {
								    perror("recv");
								    exit(1);
								}
								else
								{
									// sizeOfList=0;
									// printf("%s", buf);
		                            char * hName;
			                        char tempConnID[1];
			                        char * hPort;
			                        sizeOfList = 0;
			                        char * temp197;

			                        hName = strtok (buf,"\n");
									while (hName != NULL)
									{
										temp197 = strdup(hName);
									    // printf("%s\n", temp197);

									    hName = strtok (NULL, " \n");
									    addToServerIPList(temp197);	
									    sizeOfList++;							}

									// displayList();
								}
							}


							// buf[numbytes] = '\0';

							// printf("client: received '%s'\n",buf);

							// close(sockfd);
						}

						else if(strcasecmp ("help", pch) == 0)
						{
							displayClientHelp();
						}

						else if(strcasecmp ("list", pch) == 0)
						{
							displayList();
						}
						
						else if(strcasecmp ("creator", pch) == 0)
						{
							displayCreator();
						}

						else if(strcasecmp ("display", pch) == 0)
						{
							printf("Listening on IP: %s and port: %s \n", ipstr, argv[2]);
						}

						else if(strcasecmp ("connect", pch) == 0)
						{
							printf("Not implemented yet");
						}


						else if(strcasecmp ("terminate", pch) == 0)
						{
							printf("Not implemented yet");
						}

						else if(strcasecmp ("quit", pch) == 0)
						{
							// printf("Listening on IP: %s and port: %s \n", ipstr, argv[2]);
							exit(0);
						}

						else if(strcasecmp ("get", pch) == 0)
						{
							printf("Not implemented yet");
						}

						else if(strcasecmp ("put", pch) == 0)
						{
							printf("Not implemented yet");
						}
						else if(strcasecmp ("sync", pch) == 0)
						{
							printf("Not implemented yet");
						}
						else
						{
							printf("Incorrect usage");
							displayHelp();
						}
					}
					else if (i == listener) {
	                    // handle new connections
	                    addrlen = sizeof remoteaddr;
						newfd = accept(listener,
							(struct sockaddr *)&remoteaddr,
							&addrlen);

						if (newfd == -1) {
	                        perror("accept");
	                    } else {
	                        FD_SET(newfd, &master); // add to master set
	                        if (newfd > fdmax) {    // keep track of the max
	                            fdmax = newfd;
	                        }
	                        printf("Connected to from %s on "
	                            "socket %d\n",
								inet_ntop(remoteaddr.ss_family,
									get_in_addr((struct sockaddr*)&remoteaddr),
									remoteIP, INET6_ADDRSTRLEN),
								newfd);
	                    }
	                } 
	                else {
	                    // handle data from a client
	                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
	                        // got error or connection closed by client
	                        if (nbytes == 0) {
	                            // connection closed
	                            printf("Socket %d disconnected\n", i);
	                        } else {
	                            perror("recv");
	                        }
	                        close(i); // bye!
	                        FD_CLR(i, &master); // remove from master set
	                    } else {
	                        // we got some data from a client
	                        // if (send(j, buf, nbytes, 0) == -1) {
                         //        perror("send");
                         //    }
                         //    else
                            
							// tempConnID = strtok (buf,"\t");
							// tempConnID = strtok (NULL,"\t");
							// tempConnID = strtok (NULL,"\t");
							// // printf("%s\n", hName);
							// hPort = strtok (NULL," ");
							// // printf("%s\n", hPort);
							// int z = i-3;
							// // printf("%d\n", z);
							// strcpy(list[z-1].pNo,hPort);
							// strcpy(list[z-1].address,newClientIP);
							// list[z-1].connID = z;
							// strcpy(list[z-1].hostname,hName);
	                        // for(j = 0; j <= fdmax; j++) {
	                        //     // send to everyone!
	                        //     if (FD_ISSET(j, &master)) {
	                        //         // except the listener and ourselves
	                        //         if (j != listener && j != i) {
	                        //             if (send(j, buf, nbytes, 0) == -1) {
	                        //                 perror("send");
	                        //             }
	                        //         }
	                        //     }
	                        // }
	                    }
	                } 
	            } 
	        } 
    	} 
	}

	else if(strcasecmp (sArg,argv[1]) == 0)
	{
		//printf("In server\n");
		printf("Enter a command: (Type HELP for usage)\n>>>\n");
		//printf("Hello\n");
		//All server stuff
		fd_set master;
		fd_set read_fds;
		int fdmax;

		int listener;
		int newfd;
		struct sockaddr_storage remoteaddr;
		socklen_t addrlen;

		char buf[256];
		int nbytes;

		char remoteIP[INET6_ADDRSTRLEN];
		getPublicIP();

		int yes = 1;
		int i, j, rv;

		struct addrinfo hints, *ai, *p;

		FD_ZERO(&master);
		FD_ZERO(&read_fds);
		
		//Get a socket and bind it
		
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;
		
		if((rv = getaddrinfo(NULL, argv[2], &hints, &ai)) != 0)
		{
			perror("getaddrinfo");
			displayHelp();
		}
		for(p = ai; p != NULL; p = p->ai_next)
		{
			listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
			if(listener<0){
				perror("socket");
				continue;
			}

			setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
			
			if(bind(listener, p->ai_addr, p->ai_addrlen)<0)
			{
				perror("bind");
				close(listener);
				continue;
			}

			break;
		}
		
		if(p == NULL){
			printf("Can't bind server\n");
		}
		totalConnected=0;
		freeaddrinfo(ai);
		strcpy(list[0].pNo,argv[2]);
		strcpy(list[0].address,ipstr);
		list[0].connID = 1;
		strcpy(list[0].hostname,myHostname);
		//Listening
		if(listen(listener, 10) == -1)
		{
			displayHelp();
		}

		FD_SET(STDIN, &master);
		FD_SET(listener, &master);

		fdmax = listener;

		for(;;) 
		{
	        read_fds = master; // copy it
	        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
	            perror("select");
	            exit(4);
        	}

        	// run through the existing connections looking for data to read
	        for(i = 0; i <= fdmax; i++) {
	            if (FD_ISSET(i, &read_fds)) {
	                if(i == STDIN)
					{
						// printf("input from command line\n");
						char command[50];
						fgets(command, sizeof(command), stdin);
						char * pch;
						
						pch = strtok (command," \t");

						pch[strlen(pch)-1] = '\0';

						if(strcasecmp ("help", pch) == 0)
						{
							displayHelp();
						}
						
						else if(strcasecmp ("creator", pch) == 0)
						{
							displayCreator();
						}

						else if(strcasecmp ("list", pch) == 0)
						{
							displayList();
						}

						else if(strcasecmp ("display", pch) == 0)
						{
							printf("Listening on IP: %s and port: %s \n", ipstr, argv[2]);
						}

						else if(strcasecmp ("display", pch) == 0)
						{
							printf("Not implemented yet");
						}

						else
						{
							printf("incorrect commands");
							displayHelp();
						}
					}
					else if (i == listener) {
	                    // handle new connections
	                    addrlen = sizeof remoteaddr;
						newfd = accept(listener,
							(struct sockaddr *)&remoteaddr,
							&addrlen);

						if (newfd == -1) {
	                        perror("accept");
	                    } else {
	                        FD_SET(newfd, &master); // add to master set
	                        // printf("%d\n", newfd);
	                        if (newfd > fdmax) {    // keep track of the max
	                            fdmax = newfd;
	                        }
	                        printf("Connected to %s on "
	                            "socket %d\n",
								inet_ntop(remoteaddr.ss_family,
									get_in_addr((struct sockaddr*)&remoteaddr),
									remoteIP, INET6_ADDRSTRLEN),
								newfd);
	                        totalConnected++;
	                        struct sockaddr_in *s = (struct sockaddr_in *)&remoteaddr;
						    inet_ntop(AF_INET, &s->sin_addr, newClientIP, sizeof newClientIP);
	                    }
	                } else {
	                    // handle data from a client
	                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
	                        // got error or connection closed by client
	                        if (nbytes == 0) {
	                            // connection closed
	                            printf("Client %d disconnected\n", i-3);
	                            int deleteAt = -1;
	                            int el;
	                            // printf(" %d \n", totalConnected);

	                            for(el=0;el<totalConnected+1;el++)
	                            {
	                            	if(list[el].connID == (i-3))
	                            		deleteAt = el;
	                            	//printf("%d\t%d\n", deleteAt,el);
	                            }
	                            if(deleteAt >= 0){
	                            	int c;
		                            for ( c = deleteAt; c <= totalConnected ; c++ )
		                            	list[c] = list[c+1];
	                        	}
	                        	totalConnected--;
	                        	int x;
	                        	char tempConnID[1];
		                        // displayList();
		                        char serverIPBuffer[1024] ={0};
		                        for (x=0; x <=totalConnected; x++){
		                        	// strcat(serverIPBuffer, "%d\t%s\t%s\t%s\n", list[x].connID, list[x].hostname, list[x].address, list[x].pNo);
		                        	sprintf(tempConnID, "%d", list[x].connID);
		                        	strcat(serverIPBuffer,tempConnID);
		                        	strcat(serverIPBuffer, "\t");
		                        	strcat(serverIPBuffer,list[x].hostname);
		                        	strcat(serverIPBuffer, "\t");
		                        	strcat(serverIPBuffer,list[x].address);
		                        	strcat(serverIPBuffer, "\t");
		                        	strcat(serverIPBuffer,list[x].pNo);
		                        	strcat(serverIPBuffer, "\n");
		      					}
		                        int bufflen = sizeof serverIPBuffer;
		                        //printf("%d\n", fdmax);
		                        displayList();
		                        for(j = 2; j <= fdmax; j++) {
		                            // // send to everyone!
		                            if (FD_ISSET(j, &master)) {
		                            //     // except the listener and ourselves
		                                if (j != listener) {
		                                    if (send(j, serverIPBuffer, bufflen, 0) == -1) {
		                                        perror("send");
		                                    }
		                                }
		                            }
		                        }
	                        } else {
	                            perror("recv");
	                        }
	                        close(i); // bye!
	                        // totalConnected--;
	                        FD_CLR(i, &master); // remove from master set
	                    } else {
	                        // we got some data from a client
	                        // printf("We got %s from client\n", buf);
	                        // printf("%s\n", newClientIP);
	                        char * hName;
	                        char tempConnID[1];
	                        char * hPort;
							hName = strtok (buf,"\t");
							// printf("%s\n", hName);
							hPort = strtok (NULL," ");
							// printf("%s\n", hPort);
							int z = i-3;
							// printf("%d\n", z);
							strcpy(list[z-1].pNo,hPort);
							strcpy(list[z-1].address,newClientIP);
							list[z-1].connID = z;
							strcpy(list[z-1].hostname,hName);
							// printf("%s\t%s\t%s\t%d\t\n",list[z].pNo,list[z].hostname,list[z].address,list[z].connID );
	                        int x;
	                        // displayList();
	                        char serverIPBuffer[1024] ={0};
	                        for (x=0; x <=totalConnected; x++){
	                        	// strcat(serverIPBuffer, "%d\t%s\t%s\t%s\n", list[x].connID, list[x].hostname, list[x].address, list[x].pNo);
	                        	sprintf(tempConnID, "%d", list[x].connID);
	                        	strcat(serverIPBuffer,tempConnID);
	                        	strcat(serverIPBuffer, "\t");
	                        	strcat(serverIPBuffer,list[x].hostname);
	                        	strcat(serverIPBuffer, "\t");
	                        	strcat(serverIPBuffer,list[x].address);
	                        	strcat(serverIPBuffer, "\t");
	                        	strcat(serverIPBuffer,list[x].pNo);
	                        	strcat(serverIPBuffer, "\n");
	      					}
	                        int bufflen = sizeof serverIPBuffer;
	                        // printf("%d\n", fdmax);
	                        displayList();
	                        for(j = 2; j <= fdmax; j++) {
	                            // // send to everyone!
	                            if (FD_ISSET(j, &master)) {
	                            //     // except the listener and ourselves
	                                if (j != listener) {
	                                    if (send(j, serverIPBuffer, bufflen, 0) == -1) {
	                                        perror("send");
	                                    }
	                                }
	                            }
	                        }
	                    }
	                } // END handle data from client
	            } // END got new incoming connection
	        } // END looping through file descriptors
    	} // END for(;;)--and you thought it would never end!
	}

	else
	{
		invalidArgs();
	}
}

invalidArgs()
{
	printf("Correct usage: ./filename <mode> <port>\n");
}

displayHelp()
{
	printf("\tHelp Manual\n");
	printf("HELP\t\t\t displays help options");
	printf("CREATOR \t\t\t displays name UBITID and email of creator");
	printf("DISPLAY\t\t\t displays the port and ip address the program is listening on");
	printf("REGISTER <SERVERIP> <PORT>\t connects client to a listen server(CLIENTS ONLY)");
	printf("CONNECT <DESTINATION> <PORT>\t connects a client to another client on the server list(CLIENTS ONLY)");
	printf("LIST\t\t\t lists all clients and server connected to the system");
	printf("TERMINATE <CONNECTION ID>\t terminates the connection between two clients(CLIENTS ONLY)");
	printf("QUIT\t\t\t Disconnects all connection to itself(CLIENTS ONLY)");
	printf("GET <CONNECTIONID> <FILE>\t gets a file from the directory of the connected client(CLIENTS ONLY)");
	printf("PUT <CONNECTIONID> <FILE>\t transfers a file from local directory to another client(CLIENTS ONLY)");
	printf("SYNC\t\t\t command that ensures all connected clients have a copy of every unique file");
}

displayCreator()
{
	printf("Name: \t\tElroy Alva\n");
	printf("UBIT ID: \t50168107\n");
	printf("Email: \t\telroypre@buffalo.edu\n");
}

displayClientHelp()
{
	printf("Help Manual\n");
}

displayList()
{
	int f;
	printf("id\tHostname\tIP address\tPort No\n");
	for(f=0;f<N;f++){
		 if(list[f].connID != 0)
			printf("%d\t%s\t\t%s\t%s\n",list[f].connID,list[f].hostname,list[f].address,list[f].pNo);
	}
}

addToServerIPList(char *temp){
	char * hName;
    char * tempConnID;
    char * hPort;
    char * hAddress;

    tempConnID = strtok (temp,"\t");
    hName = strtok(NULL,"\t");
    hAddress = strtok(NULL,"\t");
    hPort = strtok(NULL,"\t");
	// printf("%s\t%s\t%s\t%s\n", tempConnID, hName,hAddress,hPort);

	strcpy(list[sizeOfList].pNo,hPort);
	strcpy(list[sizeOfList].address,hAddress);
	list[sizeOfList].connID = atoi(tempConnID);
	strcpy(list[sizeOfList].hostname,hName);

	// sizeOfList++;

}