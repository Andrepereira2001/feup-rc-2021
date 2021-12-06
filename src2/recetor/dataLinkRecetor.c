#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "dataLinkRecetor.h"
#include "../VAR.h"

struct termios oldtio,newtio;

unsigned int frameSequenceNumber = 0; // number that must be received switch between 0 and 1

int openSerialPort(char * port) {
    /*
        Open serial port device for reading and writing and not as controlling tty
        because we don't want to get killed if linenoise sends CTRL-C.
    */

    int fd = open(port, O_RDWR | O_NOCTTY );
    if (fd <0) {
        perror(port); 
        exit(-1); 
    }
    
    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

    /* 
        VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
        leitura do(s) pr髕imo(s) caracter(es)
    */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    printf("New termios structure set\n");

    return fd;
}

int sendMessage(int fd,unsigned char* message, int size){
    int res;
    res = write(fd,message,size);
    printf("sent: %s, %u\n",message, res);
    return 0;
}

int receiveMessage(int fd,unsigned char * buf){
    return read(fd,buf,1);
}

int dataLinkState(unsigned char data, enum FrameState *frameState, int globalState, Frame * frame){
    switch(*frameState){
        case START:
            if(data == FLAG){
                *frameState = FLAG_RCV;
                frame->frame[0]=data;
                frame->sizeFrame = 1;
            }
            break;
        case FLAG_RCV:
            if(data == FLAG){
                *frameState = FLAG_RCV;
                frame->frame[0]=data;
                frame->sizeFrame = 1;
            }
            else if(data == AE_SENT){
                *frameState = A_RCV;
                frame->frame[frame->sizeFrame]=data;
                frame->sizeFrame = frame->sizeFrame+1;
            }
            else{
                *frameState=START;
                frame->sizeFrame = 0;
            }
            break;
        case A_RCV:
            if(data == FLAG){
                *frameState = FLAG_RCV;
                frame->frame[0]=data;
                frame->sizeFrame = 1;
            }
            else if(data == C_SET && globalState == ESTABLISH){
                *frameState=C_RCV;
                frame->frame[frame->sizeFrame]=data;
                frame->sizeFrame = frame->sizeFrame+1;
            }
            else if((data == C_DISC || data == C_UA) && globalState==TERMINATE){
                *frameState=C_RCV;
                frame->frame[frame->sizeFrame]=data;
                frame->sizeFrame = frame->sizeFrame+1;
            }
            else if((data == C_S0 || data == C_S1) && globalState == TRANSFER){
                *frameState=C_RCV;
                frame->frame[frame->sizeFrame]=data;
                frame->sizeFrame = frame->sizeFrame+1;
            }
            else{
                *frameState=START;
                frame->sizeFrame = 0;
            }
            break;
        case C_RCV:
            if(data == FLAG){
                *frameState = FLAG_RCV;
                frame->frame[0]=data;
                frame->sizeFrame = 1;
            }
            else if(data == frame->frame[1]^frame->frame[2]){
                *frameState = BCC_OK;
                frame->frame[frame->sizeFrame]=data;
                frame->sizeFrame = frame->sizeFrame+1;
            }
            else{
                *frameState=START;
                frame->sizeFrame = 0;
            }
            break;
        case BCC_OK:
            if(data==FLAG){
                *frameState=END;
                frame->frame[frame->sizeFrame]=data;
            }
            else if (globalState == TRANSFER){
                *frameState = DATA_RCV;
                frame->frame[frame->sizeFrame]=data;
                frame->sizeFrame = frame->sizeFrame+1;
            }
            else{
                *frameState=START;
                frame->sizeFrame = 0;
            }
            break;
        case DATA_RCV:
            if(data==FLAG){
                *frameState=END;
                frame->frame[frame->sizeFrame]=data;
            }
            else {
                frame->frame[frame->sizeFrame]=data;
                frame->sizeFrame = frame->sizeFrame+1;
            }
            break;
        case END:
            break;
    }
}

int sendControlFrame(int fd,unsigned char control){
    unsigned char CONTROL[5];
    
    CONTROL[0] = FLAG;
    CONTROL[1] = AE_SENT;
    CONTROL[2] = control;
    CONTROL[3] = CONTROL[1]^CONTROL[2]; //xor 
    CONTROL[4] = FLAG;
    
    sendMessage(fd,CONTROL,5);
    
    return 0;
}

int establish(int fd){
    unsigned char buf[255];
    enum FrameState frameState = START;

    Frame frameResponse;
    frameResponse.frame = malloc (255 * sizeof (unsigned char));
    frameResponse.sizeFrame = 0;

    //Read control frame sent by transmitter (C_SET)
    while(frameState != END){
        receiveMessage(fd,buf);
        dataLinkState(buf[0], &frameState, ESTABLISH, &frameResponse);
        printf("%x ",buf[0]);
    }

    sendControlFrame(fd, C_UA);
    
    free(frameResponse.frame);
    return 0;

}

int llopenRecetor(char* port){
    int fd = openSerialPort(port);
    if(fd <= 0){
        perror("openSerialPort");
        exit(-1);
    }

    if(establish(fd)!= 0){
        perror("establish");
        exit(-1);
    }

    return fd;
}

int destuff(Frame *frame, unsigned char *data, int *dataSize){
    int i = 4;
    unsigned char bcc;
    *dataSize = 0;
    while(i < frame->sizeFrame){
        if(frame->frame[i]==O_ESC){
            i++;
            if(frame->frame[i]==O_FST){
                data[*dataSize]=FLAG;
            }
            else if (frame->frame[i]==O_SND){
                data[*dataSize]=O_ESC;
            }
        }
        else {
            data[*dataSize]=frame->frame[i];
        }
        i++;
        *dataSize = *dataSize+1;
    }

    *dataSize = *dataSize - 1;
    bcc = data[0];
    for(int i=1; i<(*dataSize); i++){
        bcc = data[i]^bcc;
    }
    if(bcc != data[*dataSize]){
        return -1;
    }
    return 0;
}

int llread(int fd, unsigned char *data, int *dataSize){

    int end = FALSE;

    unsigned char buf[255];
    enum FrameState frameResponseState = START;
    Frame frameResponse;
    frameResponse.frame = malloc (255 * sizeof (unsigned char));
    frameResponse.sizeFrame = 0;

    while( !end ){
        frameResponseState = START;
        frameResponse.sizeFrame = 0;

        while(frameResponseState != END){
            
            receiveMessage(fd,buf);
            dataLinkState(buf[0], &frameResponseState, TRANSFER, &frameResponse);
            printf("%x ",buf[0]);
        }

        if(destuff(&frameResponse,data,dataSize) == -1){
            if(frameSequenceNumber == 0){
                sendControlFrame(fd, C_REJ0);
            }else if(frameSequenceNumber == 1){
                sendControlFrame(fd, C_REJ1);
            }
        }else if(frameResponse.frame[2] == C_S0){
            if(frameSequenceNumber == 0){
                frameSequenceNumber = 1;
                sendControlFrame(fd, C_RR1);
                end = TRUE;
            } else if(frameSequenceNumber == 1){
                sendControlFrame(fd, C_RR1);
            }
        }else if(frameResponse.frame[2] == C_S1){
            if(frameSequenceNumber == 1){
                frameSequenceNumber = 0;
                sendControlFrame(fd, C_RR0);
                end = TRUE;
            } else if(frameSequenceNumber == 0){
                sendControlFrame(fd, C_RR0);
            }
        }
    }

    return *dataSize;
}

int llclose(int fd){
    unsigned char buf[255];
    enum FrameState frameState = START;

    Frame frameResponse;
    frameResponse.frame = malloc (255 * sizeof (unsigned char));
    frameResponse.sizeFrame = 0;


    //Read control frame sent by receiver (C_DISC))
    while(frameState != END){
        receiveMessage(fd,buf);
        dataLinkState(buf[0], &frameState, TERMINATE, &frameResponse);
        printf("%x ",buf[0]);
    }

    sendControlFrame(fd, C_DISC);
    free(frameResponse.frame);   


    sleep(2);
   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}
