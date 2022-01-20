
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "utils.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int dowloadFile(int downloadfd, char* file){
    //writing dowloaded data to file
    char buf[255];

    int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    int size;
    while ((size = readSocket(&downloadfd, buf)) > 0){
        write(fd,&buf,size);
    }
    close(fd);

    if (closeSocket(&downloadfd) != 0){
        return -1;
    };
    return 0;
}

int main(int argc, char *argv[]){
    struct hostent *h;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        exit(-1);
    } 

    Args args;
    if( parseArguments(argv[1], &args) != 0){
        return -1;
    }

    if( getIp(&args) != 0){
        return -1;
    }

    printf("Host: %s\n",args.host);
    printf("Path: %s\n",args.path);
    printf("FileName: %s\n",args.filename);
    printf("user: %s\n",args.user);
    printf("password: %s\n",args.password);
    printf("ip: %s\n",args.ip);

    int socketfd;
    char buf[255];
    char command[255] ="";
    int val = 0;
    int port; 
    int downloadfd; 


    //create and connect socket
    if (connectSocket(&args, &socketfd) != 0){
        fprintf(stderr, "Error connecting socket\n");
        return -1;
    }

    //read begin of connection
    if(readText(&socketfd, "220")!= 0){
        fprintf(stderr, "Error first message (220)\n");
        return -1;
    }

    //giving user credential
    strcpy(command, "user ");
    strcat(command, args.user);
    strcat(command, "\n");
    writeSocket(&socketfd, command);
    if(readText(&socketfd, "331")!= 0){
        fprintf(stderr, "Error in login user (331)\n");
        return -1;
    }

    //giving password for user credential
    strcpy(command, "pass ");
    strcat(command, args.password);
    strcat(command, "\n");
    writeSocket(&socketfd, command);
    if(readText(&socketfd, "230")!= 0){
        fprintf(stderr, "Error in login password (230)\n");
        return -1;
    }

    //passive mode selection
    writeSocket(&socketfd, "pasv\n");
    do{
        if(readSocket(&socketfd, buf) < 0){
            return -1;
        }
        printf("%s",buf);

        val = checkCode(buf,"227");
        if(val == -1){
            fprintf(stderr, "Error entering passive mode (227)\n");
            return -1;
        }
    }while(val == 0);

    //calculating the port for the other tux
    if((port = parsePassivePort(buf)) == -1){
        fprintf(stderr, "Error discovering port (227)\n");
        return -1;
    }
    args.port = port;

    //creating new socket to receive file data
    if (connectSocket(&args, &downloadfd) != 0){
        fprintf(stderr, "Error connecting socket\n");
        return -1;
    }
    
    //passing retrive file command
    strcpy(command, "retr ");
    strcat(command, args.path);
    strcat(command, "\n");
    writeSocket(&socketfd, command);
    if(readText(&socketfd, "150")!= 0){
        fprintf(stderr, "Error opening transfer (150)\n");
        return -1;
    }

    //reads and writes the data on the file
    if( dowloadFile(downloadfd, args.filename) != 0){
        fprintf(stderr, "Error downloading file\n");
        return -1;
    }
    
    //waiting for transference complete
    if(readText(&socketfd, "226")!= 0){
        fprintf(stderr, "Error transfer completed (226)\n");
        return -1;
    }

    //quiting socket
    writeSocket(&socketfd, "quit\n");
    if(readText(&socketfd, "221")!= 0){
        fprintf(stderr, "Error quiting (221)\n");
        return -1;
    }

    if (closeSocket(&socketfd) != 0){
        return -1;
    };
    
}