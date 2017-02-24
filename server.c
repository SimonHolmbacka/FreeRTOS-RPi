#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 


int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    char wish;

    long int SrcID;
    long int DestID;
    long int Pver;
    long int Profiles;
    long int Data;
    char Length;
    char bytes_to_receive;
    char received_bytes;
    struct packet
    { 
        long int srcID;
        long int destID;
        long int pver;
        long int profiles;
        char length;
        long int data;
    };

    if (argc < 2) {
        fprintf(stderr,"usage: %s port_number1\n",argv[0]);
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR DETECTED !!! Problem in opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR DETECTED !!! There was a problem in binding");

    listen(sockfd, 10);
    clilen = sizeof(cli_addr);

    printf("Server listening on port number %d...\n", serv_addr.sin_port); 

    newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0) 
        error("ERROR DETECTED !!! the connection request was not accepted");

    char buffer[128];
    struct packet *pkt = (struct packet *) buffer;
    char *payload = buffer + sizeof(struct packet);
    long int packet_size;

    bytes_to_receive = sizeof(pkt);
    received_bytes = 0;

    if (recv(newsockfd, pkt, sizeof(pkt), 0) < 0)
        error("ERROR DETECTED !!! There was a problem in reading the data");
    else
    { 
        do {
            received_bytes += (buffer + received_bytes, bytes_to_receive - received_bytes);
        } while (received_bytes != bytes_to_receive);

        SrcID = pkt->srcID;
        DestID = pkt->destID;
        Pver = pkt->pver ;
        Profiles = pkt->profiles;
        Length = pkt->length;
        Data = pkt->data;
        printf("Data Received from Client_1 are :\n");
        printf("Source ID: %l\n", SrcID);
        printf("Destination ID: %l\n", DestID);
        printf("profile Version: %l\n", Pver);
        printf("No of Profiles: %l\n", Profiles);
        printf("Length: %l\n", Length);
        printf("data : %l\n", Data);
    }
    if (close(newsockfd) == -1) {
        error("Error closing connection with client 1");
    }

    printf("Connection with client 1 has been closed\n");
    return 0; 
}