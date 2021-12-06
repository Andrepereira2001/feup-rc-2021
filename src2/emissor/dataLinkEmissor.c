#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "dataLinkEmissor.h"
#include "../VAR.h"

unsigned int alarmCalls = 0, flag = 1;

unsigned char lastMessage[255];
unsigned int lastMessageSize = 0;

int fd; // Serial port file descriptor

unsigned int frameSequenceNumber = 0; // last frame sent switch between 0 and 1

struct termios oldtio,newtio;

void alarmCall(){                   // atende alarme
	printf("alarme # %d\n", alarmCalls);
	flag=1;
	alarmCalls++;
    sendMessage(fd,lastMessage,lastMessageSize);
}

int openSerialPort(char * port) {
    /*
        Open serial port device for reading and writing and not as controlling tty
        because we don't want to get killed if linenoise sends CTRL-C.
    */

    fd = open(port, O_RDWR | O_NOCTTY );
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

    return 0;
}

int sendMessage(int fd, unsigned char * message, unsigned size){
    int res;
    
    if(flag){
      alarm(4);                 // activa alarme de 4s
      flag=0;
    }
    for(int i = 0; i <= size; i++){ 
        lastMessage[i] = message[i];
    }
    lastMessageSize = size;
    res = write(fd,message,size);   
    printf("%d bytes written\n", res);
    return res;
}

int receiveMessage(int fd,unsigned char * buf){
    return read(fd,buf,1);
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

int dataLinkState(unsigned char data, enum FrameState *frameState, int globalState, Frame * frame){
    switch(*frameState){
        case START:
            if(data == FLAG){
                *frameState = FLAG_RCV;
                frame->frame[frame->sizeFrame]=data;
                frame->sizeFrame = frame->sizeFrame + 1;
            }
            break;
        case FLAG_RCV:
            if(data == FLAG){
                *frameState = FLAG_RCV;
                frame->frame[0] = data;
                frame->sizeFrame = 1;
            }
            else if(data == AE_SENT){
                *frameState = A_RCV;
                frame->frame[frame->sizeFrame] = data;
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
                frame->frame[0] = data;
                frame->sizeFrame = 1;
            }
            else if(globalState == ESTABLISH && data == C_UA){
                *frameState=C_RCV;
                frame->frame[frame->sizeFrame]=data;
                frame->sizeFrame = frame->sizeFrame+1;
            }
            else if(globalState == TERMINATE && data == C_DISC){
                *frameState=C_RCV;
                frame->frame[frame->sizeFrame]=data;
                frame->sizeFrame = frame->sizeFrame+1;
            }else if(globalState == TRANSFER && (data == C_REJ0 || data == C_REJ1 || data == C_RR1 || data == C_RR0)){
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
                frame->frame[frame->sizeFrame] = data;
                frame->sizeFrame = frame->sizeFrame+1;
            }
            else{
                *frameState = START;
                frame->sizeFrame = 0;
            }
            break;
        case BCC_OK:
            if(data==FLAG){
                *frameState=END;
                frame->frame[frame->sizeFrame]=data;
                frame->sizeFrame = 0;
            }
            else{
                *frameState=START;
                frame->sizeFrame = 0;
            }
            break;
        case END:
            break;
    }
    return 0;
}

int establish(){
    unsigned char buf[255];
    enum FrameState frameState = START;

    Frame frameResponse;
    frameResponse.frame = malloc (255 * sizeof (unsigned char));
    frameResponse.sizeFrame = 0;

    sendControlFrame(fd, C_SET);

    //Read control frame sent by receiver (C_UA)
    while(frameState != END){
        receiveMessage(fd,buf);
        dataLinkState(buf[0], &frameState, ESTABLISH, &frameResponse);
        printf("%x ",buf[0]);
    }

    free(frameResponse.frame);
    return 0;
}

int llopenEmissor(char * port){

    (void) signal(SIGALRM, alarmCall); 
    
    if(openSerialPort(port)!= 0){
        perror("openSerialPort");
        exit(-1);
    }

    if(establish()!= 0){
        perror("establish");
        exit(-1);
    }

    return fd;
}

int buildFrame(unsigned char * data, int size, Frame *frameBackup){
    unsigned char *FRAME = malloc (255 * sizeof (unsigned char));
    unsigned int currFrame = 0;
    unsigned char bcc;
    
    FRAME[0] = FLAG;
    FRAME[1] = AE_SENT;
    if(frameSequenceNumber == 0){
       FRAME[2] = C_S0;
    } else if (frameSequenceNumber == 1){
       FRAME[2] = C_S1; 
    }
    FRAME[3] = FRAME[1]^FRAME[2]; //xor 
    currFrame = 4;

    //start byte stuffing
    for(int i = 0; i < size; i++){
        if(i == 0){
            bcc = data[i];
        }
        else {
           bcc = bcc ^ data[i];
        }
        
        if(data[i] == FLAG){
            FRAME[currFrame] = O_ESC;
            currFrame++;
            FRAME[currFrame] = O_FST;
            currFrame++;
        }
        else if ( data[i] == O_ESC){
            FRAME[currFrame] = O_ESC;
            currFrame++;
            FRAME[currFrame] = O_SND;
            currFrame++;
        }
        else {
            FRAME[currFrame] = data[i];
            currFrame++;
        }
    }

    if(bcc == FLAG){
        FRAME[currFrame] = O_ESC;
        currFrame++;
        FRAME[currFrame] = O_FST;
        currFrame++;
    }
    else if (bcc == O_ESC){
        FRAME[currFrame] = O_ESC;
        currFrame++;
        FRAME[currFrame] = O_SND;
        currFrame++;
    } else {
        FRAME[currFrame] = bcc;
        currFrame++;
    }
    //end byte stuffing

    FRAME[currFrame] = FLAG;
    currFrame++;

    frameBackup->frame = FRAME;
    frameBackup->sizeFrame = currFrame;
    
    return 0;
}

//retorna o numero de caracteres escritos
int llwrite(int fd, unsigned char* data, int dataSize){
    int end = FALSE;
    //frame to be built and sent 
    Frame frameBackup;

    //frame to be received and confirmed
    unsigned char buf[255];
    enum FrameState frameResponseState = START;
    Frame frameResponse;
    frameResponse.frame = malloc (255 * sizeof (unsigned char));
    frameResponse.sizeFrame = 0;
    
    //frame to be built 
    buildFrame(data, dataSize, &frameBackup);

    do{
        frameResponseState = START;
        frameResponse.sizeFrame = 0;
        //send frame to be sent 
        sendMessage(fd, frameBackup.frame, frameBackup.sizeFrame);
        
        //receive and confirm frame sent by recetor
        while(frameResponseState != END){
            receiveMessage(fd,buf);
            dataLinkState(buf[0], &frameResponseState, TRANSFER, &frameResponse);
            printf("%x ",buf[0]);
        }

        if(frameResponse.frame[2] == C_RR1 && frameSequenceNumber == 0){
            printf("accepted RR1\n");
            end = TRUE;
            frameSequenceNumber = 1;
        }else if(frameResponse.frame[2] == C_RR0 && frameSequenceNumber == 1){
            printf("accepted RR0\n");
            end = TRUE;
            frameSequenceNumber = 0;
        }else if(frameResponse.frame[2] == C_REJ1){
            printf("accepted REJ1\n");
            frameSequenceNumber = 1;
        }else if(frameResponse.frame[2] == C_REJ0){
            printf("accepted REJ0\n");
            frameSequenceNumber = 0;
        }
        
    }while( !end );

    free(frameBackup.frame);
    free(frameResponse.frame);
    
    return frameBackup.sizeFrame;
}

int llclose(int fd){
    unsigned char buf[255];
    enum FrameState frameState = START;

    Frame frameResponse;
    frameResponse.frame = malloc (255 * sizeof (unsigned char));
    frameResponse.sizeFrame = 0;

    sendControlFrame(fd, C_DISC);

    //Read control frame sent by receiver (C_UA)
    while(frameState != END){
        receiveMessage(fd,buf);
        dataLinkState(buf[0], &frameState, TERMINATE, &frameResponse);
        printf("%x ",buf[0]);
    }

    sendControlFrame(fd, C_UA);

    alarm(0);
    free(frameResponse.frame);   


    sleep(2);
   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}
