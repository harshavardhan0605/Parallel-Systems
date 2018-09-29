/*Single Author info:
mreddy2 Muppidi Harshavardhan Reddy */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "my_mpi.h"

int sockfd,sockfd_for_gather,sockfd_bar;                // Global Listening Ports 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}


int MPI_Init(int *argc, char **argv[] )      /* Opening Ports and Writing to Files and Populating COMM_WORLD structure with All Host Names */
{
    FILE *fp;
    int i,j,n;
    char **argv_param = *argv;
    MPI_COMM_WORLD.my_rank = atoi(argv_param[1]);
    struct sockaddr_in serv_addr,my_addr,serv_addr_root;

    MPI_COMM_WORLD.hostnames = (char *)malloc(8* MAXLEN * sizeof(char));  
    MPI_COMM_WORLD.all_hosts = (char **)malloc(8 * sizeof(char*));
      for(i = 0; i <8; i++)
    {
        MPI_COMM_WORLD.all_hosts[i] = &MPI_COMM_WORLD.hostnames[i * MAXLEN];
    }

    strcat(argv_param[2],"/nodefile.txt");                 //Reading from nodefile and Getting all Hosts for all the Ranks
    fp = fopen(argv_param[2], "r");
    if(fp == NULL){
    	printf("No nodefile\n");
    	exit(0);
    }
    i = 0;
    while(fscanf(fp, "%s", MPI_COMM_WORLD.all_hosts[i])) 
    { i++; }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);            //  Creating and Binding a Listening Port at each Rank
     if (sockfd < 0)
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = 0;
     if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)      
        perror("error binding tcp proxy client: ");
    listen(sockfd,5);

    bzero(&my_addr, sizeof(my_addr));                   // Reading the Port Number Binded and writing to a file at each Rank
     int len = sizeof(my_addr);
     char IP_addr[16];
     if(getsockname(sockfd, (struct sockaddr *) &my_addr, &len)<0)  
     error("getsockname error");
     int assigned_port = ntohs(my_addr.sin_port);
     char file_to_open[20];
     sprintf(file_to_open, "%d", MPI_COMM_WORLD.my_rank);
     strcat(file_to_open,".txt");
     FILE *file = fopen(file_to_open, "w");
     fprintf(file, "%d",assigned_port);
     fclose(file);

//Ports for Gather Calls
     if(MPI_COMM_WORLD.my_rank == 0)
    {
	sockfd_for_gather = socket(AF_INET, SOCK_STREAM, 0);   //  Creating and Binding a Listening Port(Gather) at each Rank
	if (sockfd_for_gather < 0)
	        error("ERROR opening socket");
	bzero((char *) &serv_addr_root, sizeof(serv_addr));
	serv_addr_root.sin_family = AF_INET;
        serv_addr_root.sin_addr.s_addr = INADDR_ANY;
        serv_addr_root.sin_port = 0;
	if (bind(sockfd_for_gather, (struct sockaddr *) &serv_addr_root,sizeof(serv_addr_root)) < 0)
	        perror("error binding tcp proxy client: ");
	listen(sockfd_for_gather,5);

	bzero(&my_addr, sizeof(my_addr));                       // Reading the Port Number Binded and writing to a file at each Rank
	int len = sizeof(my_addr);
    char IP_addr[16];
   	if(getsockname(sockfd_for_gather, (struct sockaddr *) &my_addr, &len)<0) error("getsockname error");
        int assigned_portg = ntohs(my_addr.sin_port);
        char bgfile_to_open[20];
        sprintf(bgfile_to_open, "%d", MPI_COMM_WORLD.my_rank);
        strcat(bgfile_to_open,"_1.txt");
        FILE *file1 = fopen(bgfile_to_open, "w");
        fprintf(file1, "%d",assigned_portg);
        fclose(file1); 
    }
        

// Ports for Barrier Calls
     if(MPI_COMM_WORLD.my_rank == 0)
    {
	sockfd_bar = socket(AF_INET, SOCK_STREAM, 0);       //  Creating and Binding a Listening Port(Barrier) at each Rank
	if (sockfd_for_gather < 0)
	        error("ERROR opening socket");
	bzero((char *) &serv_addr_root, sizeof(serv_addr));
	serv_addr_root.sin_family = AF_INET;
    serv_addr_root.sin_addr.s_addr = INADDR_ANY;
    serv_addr_root.sin_port = 0;
	if (bind(sockfd_bar, (struct sockaddr *) &serv_addr_root,sizeof(serv_addr_root)) < 0) 
        perror("error binding tcp proxy client: ");
	listen(sockfd_bar,5);

	bzero(&my_addr, sizeof(my_addr));                   // Reading the Port Number Binded and writing to a file at each Rank
	int len = sizeof(my_addr);
    char IP_addr[16];
   	if(getsockname(sockfd_bar, (struct sockaddr *) &my_addr, &len)<0) error("getsockname error");
        int assigned_portg = ntohs(my_addr.sin_port);
        char bgfile_to_open[20];
        sprintf(bgfile_to_open, "%d", MPI_COMM_WORLD.my_rank);
        strcat(bgfile_to_open,"_2.txt");
        FILE *file2 = fopen(bgfile_to_open, "w");
        fprintf(file2, "%d",assigned_portg);
        fclose(file2);
     }
    return 0;
}

int MPI_Comm_rank(MPI_Comm comm, int *my_rank)   // Returnig the Rank stored during the Init call in comm.world Struct
{
    *my_rank = comm.my_rank;
    return 0;
}

int MPI_Comm_size(MPI_Comm comm, int *p)    // Just Returning 0 
{
    *p = 0;
    return 0;
}

int MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag,MPI_Comm comm)
{
    int sockfd1, port, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
  
    FILE *file;
    char file_to_open[20];
    sprintf(file_to_open, "%d", dest);
    if(tag ==100){strcat(file_to_open,"_1.txt");}    // Based on Tag Deciding the file to open for Port Number(Gather/Barrier/Receive)
    else if(tag == 10){strcat(file_to_open,"_2.txt");}
    else{ strcat(file_to_open,".txt");}

    file = fopen(file_to_open, "r");
    while(file == NULL) {                    // Loop until it can find that the Destination has a listening port opened by checking files
    file = fopen(file_to_open, "r");}
    fscanf(file, "%d", &port);
    fclose(file);

    sockfd1 = socket(AF_INET, SOCK_STREAM, 0);                  // Opening a Socket and Getting the server address based on port to connect 
    if (sockfd1 < 0) error("ERROR opening socket");
    server = gethostbyname(comm.all_hosts[dest]);
    if (server == NULL)  { fprintf(stderr,"ERROR, no such host\n"); exit(0);}
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(port);
    
    if (connect(sockfd1,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) error("ERROR connecting");
    if(tag == 50 || tag == 10)  n = send(sockfd1,buf,count,0);  
    else    n = send(sockfd1,buf,count*sizeof(double),0);  // Send Call Handling for Different Datatype and size in case because of Gathering
    if (n < 0)   error("ERROR writing to socket");
    close(sockfd1);
    return 0;
}



int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{    
     int newsockfd, n, z=0;                         
     socklen_t clilen;
     struct sockaddr_in cli_addr;
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr , &clilen);  // Accepting the Client Connection and Receving the Message
     if (newsockfd < 0)   error("ERROR on accept");
     n= recv(newsockfd,buf,count,MSG_WAITALL);
     if (n < 0) error("ERROR reading from socket");
     close(newsockfd);
     return 0;
}


int MPI_Gather(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
     int newsockfd,newsockfd3;
     socklen_t clilen,clilen2;
     struct sockaddr_in serv_addr ,my_addr,cli_addr,cli_addr2;
     struct hostent *server;
     int n,caller = -1;
     clilen = sizeof(cli_addr);
     clilen2 = sizeof(cli_addr2);

     if(tag==100)                           // For Gathering all values at Root 
     {
     newsockfd = accept(sockfd_for_gather, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) error("ERROR on accept");

     for(int i = 0;i<8;i++)             // Loop to find which rank got connected 
    {
        server = gethostbyname(comm.all_hosts[i]);       
        if (server == NULL) {
                fprintf(stderr,"ERROR, no such host\n");
                exit(0);
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));
        bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
        if(serv_addr.sin_addr.s_addr == cli_addr.sin_addr.s_addr )
        {
            caller = i;
            break;
        }
    }

     if(caller!=source)           // Based on the the Rank connected deciding the index in the buffer to store based on specific Pair of Ranks
    {                
	    buf = buf+((caller-source)/2)*(count*sizeof(double));
     }
     n = recv(newsockfd,buf,count*sizeof(double),MSG_WAITALL);
     if (n < 0) error("ERROR reading from socket");
     close(newsockfd);
     return 0;
    }
     
     else                       // For Barrier Calls
    {
     newsockfd3 = accept(sockfd_bar, (struct sockaddr *) &cli_addr2, &clilen2);  // Accepting Barrier Calls
     if (newsockfd3 < 0)  error("ERROR on accept"); 
     n = recv(newsockfd3,buf,count,MSG_WAITALL);
     if (n < 0) error("ERROR reading from socket");
     close(newsockfd3);
     return 0;
    }
}

int MPI_Barrier ( MPI_Comm comm )               // All Ranks sending data to Rank 0 and Rank 0 Receiving from everyone
{
    int i;
    char send = 'a';
    char recv;
    MPI_Status Status;

    if(comm.my_rank == 0){
        for(i = 1; i < 8; i++){
		    MPI_Gather(&recv, 1, MPI_CHAR, i, 10, MPI_COMM_WORLD, &Status);
        }
	}
	else{	    
		MPI_Send(&send, 1, MPI_CHAR, 0, 10, MPI_COMM_WORLD);
	}
}

int MPI_Finalize(MPI_Comm comm)        //Closing the Sockets          
{
    close(sockfd);
    close(sockfd_for_gather);
    close(sockfd_bar);
}
