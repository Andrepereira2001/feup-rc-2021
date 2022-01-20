#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

int parseArguments(char *commandArgs, Args *args){
    char * data = strtok(commandArgs,"//");

    //checking protocol
    if(strcmp(data,"ftp:") != 0){
        fprintf(stderr, "Error parsing string\n");
        return -1;
    }

    //checking host
    data = strtok(NULL, "/");
    if(data == NULL){
        fprintf(stderr, "Error parsing string (host)\n");
        return -1;
    }
    char * host = data;
    memset(args->host, '\0', 255);
    strcpy(args->host, data);

    //checking path
    data = strtok(NULL, "\0");
    if(data == NULL){
        fprintf(stderr, "Error parsing string (path)\n");
        return -1;
    }
    memset(args->path, '\0', 255);
    strcpy(args->path, data);

    char path[255];
    strcpy(path, args->path);
    char* file = strtok(path, "/");
    while(file != NULL){
        memset(args->filename, '\0', 255);
        strcpy(args->filename, file);
        file = strtok(NULL, "/");
    }

    //Checking if credentials where given
    data = strtok(host, ":");
    if (strcmp(data,args->host) == 0){
        printf("No credentials given\n");
        memset(args->user, '\0', 255);
        strcpy(args->user, "anonymous");
        memset(args->password, '\0', 255);
        strcpy(args->password, "123");
        return 0;
    }

    //checking credentials - user
    if(data == NULL){
        fprintf(stderr, "Error parsing string (user)\n");
        return -1;
    }
    memset(args->user, '\0', 255);
    strcpy(args->user, data);

    //checking credentials - password
    data = strtok(NULL, "@");
    if(data == NULL){
        fprintf(stderr, "Error parsing string (password)\n");
        return -1;
    }
    memset(args->password, '\0', 255);
    strcpy(args->password, data);

    data = strtok(NULL, "\0");
    if(data == NULL){
        fprintf(stderr, "Error parsing string (host)\n");
        return -1;
    }
    memset(args->host, '\0', 255);
    strcpy(args->host, data);
    
    
    return 0;
}

int getIp(Args * args){
    struct hostent *h;

    if ((h = gethostbyname(args->host)) == NULL) {
        fprintf(stderr, "Error geting ip for %s", args->host);
        return -1;
    }

    memset(args->ip, '\0', 255);
    strcpy(args->ip, inet_ntoa(*((struct in_addr *) h->h_addr)));

    args->port = 21;

    return 0;
}

int connectSocket(Args * args, int * socketfd){
    struct sockaddr_in server_addr;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(args->ip);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(args->port);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if (((*socketfd) = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
         fprintf(stderr, "Error creating socket\n");
         return -1;
    }
    
     /*connect to the server*/
    if (connect((*socketfd),(struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Error connecting socket\n");
        return -1;
    }
    
    return 0;
}

int closeSocket(int *socketfd){
    if (close((*socketfd))<0) {
        fprintf(stderr, "Error closing socket\n");
        return -1;
    }
    return 0;
}


int writeSocket(int *socketfd, char* buf) {
    size_t bytes;

    /*send a string to the server*/
    bytes = write((*socketfd), buf, strlen(buf));
    if (bytes > 0){
        printf("%s", buf);
        //printf("Bytes escritos %ld\n", bytes);
        return 0;
    }
    else {
        fprintf(stderr, "Error writing to socket\n");
        return -1;
    }
}

int readSocket(int *socketfd, char * buf) {
    size_t bytes;
    bzero(buf, 255);

    bytes = read((*socketfd),buf ,254);
    
    if (bytes < 0) {
        fprintf(stderr, "Error reading from socket\n");
        return -1;
    }
    return bytes;
}

int checkCode(char * buf, char * code){
    char *data = strtok(buf, "\n");
    
    while(data != NULL){
        for(int i=0; i<3; i++){
            if(code[i] != data[i]){
                return -1;
            }
        }
        if(data[3] != '-') return 1; 
        data = strtok(NULL,"\n"); 
    }

    return 0;
}

int readText(int *socketfd, char * code){
    char buf[255];
    int val = 0;
    do{
        if(readSocket(socketfd, buf) < 0){
            return -1;
        }
        printf("%s",buf);

        val = checkCode(buf,code);
        if(val == -1) return -1;
    }while(val == 0);
    return 0;
}

int parsePassivePort(char * buf){
    int port = 0;
    strtok(buf, ",");
    strtok(NULL, ",");
    strtok(NULL, ",");
    strtok(NULL, ",");
    port = atoi(strtok(NULL, ","))*256;
    port += atoi(strtok(NULL, ")"));

    return port;
}