/*Non-Canonical Input Processing*/

//sudo socat -d  -d  PTY,link=/dev/ttyS0,mode=777   PTY,link=/dev/ttyS1,mode=777

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "emissor.h"
#include "../VAR.h"

enum DataState currDataState=START;

enum GlobalState currGlobalState=ESTABLISH;

unsigned char *fileName = "./pinguim1.gif";
unsigned char *fileNameOrg = "./pinguim.gif";

unsigned int alarmCalls = 0, flag = 1;

unsigned char lastMessage[255];
unsigned int lastMessageSize = 0;

int fd; // Serial port file descriptor

void alarmCall()                   // atende alarme
{
	printf("alarme # %d\n", alarmCalls);
	flag=1;
	alarmCalls++;
    sendMessage(fd,lastMessage,lastMessageSize);
    currDataState=START;
}

int dataLinkState(unsigned char data,unsigned char* word, int* curr){
    switch(currDataState){
        case START:
            if(data == FLAG){
                currDataState = FLAG_RCV;
                word[*curr]=data;
                *curr = *curr+1;
            }
            break;
        case FLAG_RCV:
            if(data == FLAG){
                currDataState = FLAG_RCV;
                word[0]=data;
                *curr = 1;
            }
            else if(data == AE_SENT){
                currDataState = A_RCV;
                word[*curr]=data;
                *curr = *curr+1;
            }
            else{
                currDataState=START;
                *curr = 0;
            }
            break;
        case A_RCV:
            if(data == FLAG){
                currDataState = FLAG_RCV;
                word[0]=data;
                *curr = 1;
            }
            else if(currGlobalState == ESTABLISH && data == C_UA){
                currDataState=C_RCV;
                word[*curr]=data;
                *curr = *curr+1;
            }
            else if(currGlobalState == TERMINATE && data == C_DISC){
                currDataState=C_RCV;
                word[*curr]=data;
                *curr = *curr+1;
            }else if(currGlobalState == TRANSFER && (data == C_REJ0 || data == C_REJ1 || data == C_RR1 || data == C_RR0)){
                currDataState=C_RCV;
                word[*curr]=data;
                *curr = *curr+1;
            }
            else{
                currDataState=START;
                *curr = 0;
            }
            break;
        case C_RCV:
            if(data == FLAG){
                currDataState = FLAG_RCV;
                word[0]=data;
                *curr = 1;
            }
            else if(data == word[1]^word[2]){
                currDataState = BCC_OK;
                word[*curr]=data;
                *curr = *curr+1;
            }
            else{
                currDataState=START;
                *curr = 0;
            }
            break;
        case BCC_OK:
            if(data==FLAG){
                currDataState=END;
                word[*curr]=data;
                *curr = 0;
                if(currGlobalState == ESTABLISH && word[2] == C_UA) currGlobalState = TRANSFER; 
                else if(currGlobalState == TERMINATE && word[2] == C_DISC) currGlobalState = GLOBAL_END;
            }
            else{
                currDataState=START;
                *curr = 0;
            }
            break;
        case END:
            break;
    }
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

int sendLatestFrame(int fd, FrameBackup *frameBackup){
    sendMessage(fd,frameBackup->latestframe, frameBackup->sizeFrame);
}

int sendFrame(int fd, unsigned char * packet, int size, FrameBackup *frameBackup){
    unsigned char FRAME[255];
    unsigned int currFrame = 0;
    unsigned char bcc;
    
    FRAME[0] = FLAG;
    FRAME[1] = AE_SENT;
    if(frameBackup->frameToSend == 0){
       FRAME[2] = C_S0;
    } else if (frameBackup->frameToSend == 1){
       FRAME[2] = C_S1; 
    }
    FRAME[3] = FRAME[1]^FRAME[2]; //xor 
    currFrame = 4;
    for(int i = 0; i < size; i++){
        if(i == 0){
            bcc = packet[i];
        }
        else {
           bcc = bcc ^ packet[i];
        }
        
        if(packet[i] == FLAG){
            FRAME[currFrame] = O_ESC;
            currFrame++;
            FRAME[currFrame] = O_FST;
            currFrame++;
        }
        else if ( packet[i] == O_ESC){
            FRAME[currFrame] = O_ESC;
            currFrame++;
            FRAME[currFrame] = O_SND;
            currFrame++;
        }
        else {
            FRAME[currFrame] = packet[i];
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

    FRAME[currFrame] = FLAG;
    currFrame++;

    frameBackup->latestframe = FRAME;
    frameBackup->sizeFrame = currFrame;

    sendMessage(fd,FRAME,currFrame);
    
    return 0;
}

int sendControl(int fd,unsigned char control){
    unsigned char CONTROL[5];
    
    CONTROL[0] = FLAG;
    CONTROL[1] = AE_SENT;
    CONTROL[2] = control;
    CONTROL[3] = CONTROL[1]^CONTROL[2]; //xor 
    CONTROL[4] = FLAG;
    
    sendMessage(fd,CONTROL,5);
    
    return 0;
}

int createPacket(AppPacket * appPacket, unsigned char * data, int dataSize){
    unsigned char *PACKET = malloc (255 * sizeof (unsigned char));
    PACKET[0] = 0x01;
    PACKET[1] = appPacket->sequenceNumber;
    PACKET[2] = (0x0ff00 & dataSize) >> 2;
    PACKET[3] = 0xff & dataSize;
    for(int i = 0; i < dataSize; i++){
        PACKET[i+4] = data[i];
    }

    appPacket->pSize = dataSize + 4;
    appPacket->packet = PACKET;
    appPacket->sequenceNumber = appPacket->sequenceNumber + 1;
    return 0;
}

int readFile(int fileFd, unsigned char * buf, AppPacket * appPacket){
    if (appPacket->packetState == P_END){
        return 1;
    }
    else if(appPacket->packetState == P_START){
        appPacket->packet[0] = 0x02;
        appPacket->packet[1] = 0x01;
        appPacket->packet[2] = strlen(fileName) + 1;
        for (int i = 0; i <  strlen(fileName); i++){
            appPacket->packet[i + 3] = fileName[i];
        }
        appPacket->packet[strlen(fileName) + 3] = '\0';
        appPacket->pSize = strlen(fileName) + 4;

        appPacket->sequenceNumber = 0;
        appPacket->packetState = P_DATA;

    } else if (appPacket->packetState == P_DATA){
        int res = 0;
        res = read(fileFd,buf,100);
        if(res == -1){
            printf("Error reading file\n");
            return -1;
        } else if(res == 0){
            appPacket->packetState = P_END;
            printf("End of file reach\n");
        } else {
            createPacket(appPacket,buf,res);
        }
    }
    
    if (appPacket->packetState == P_END){
        appPacket->packet[0] = 0x03;
    }
    return 0;
}

int main(int argc, char** argv)
{
    int c,res, fileFd;
    struct termios oldtio,newtio;
    unsigned char buf[255];
    int i, sum = 0, speed = 0;
    
    (void) signal(SIGALRM, alarmCall); 
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }
    
    fileFd = open(fileNameOrg, O_RDONLY);
    if (fileFd < 0) {perror("Erro while opening test.txt"); exit(-1);}

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


    //data link variables
    unsigned char word[255];
    int curr = 0;
    FrameBackup frameBackup;
    frameBackup.frameToSend = 0;
    frameBackup.acceptedFrame = TRUE;
    
    //application variables
    AppPacket appPacket;
    appPacket.pSize = 0;
    appPacket.sequenceNumber = 0;
    appPacket.packetState = P_START;
    appPacket.packet = malloc (255 * sizeof (unsigned char));

    while(currGlobalState != GLOBAL_END) {
        alarmCalls = 0;
        
        if(currGlobalState == ESTABLISH){ 
            sendControl(fd, C_SET);
        }
        else if(currGlobalState == TRANSFER){
            if(frameBackup.acceptedFrame == TRUE){
                int res = readFile(fileFd, buf, &appPacket);
                if (res == -1){
                    printf("Error building packet\n");
                } else if(res == 1){
                    currGlobalState = TERMINATE; 
                }
                else {
                    sendFrame(fd, appPacket.packet, appPacket.pSize, &frameBackup);
                }
            }
            else if(frameBackup.acceptedFrame == FALSE){
                sendLatestFrame(fd, &frameBackup);
            }
        }

        if(currGlobalState == TERMINATE){
            sendControl(fd, C_DISC);
        }

        //Read byte sent by receiver
        while(currDataState != END && alarmCalls <= 3){
            res = receiveMessage(fd,buf);
            dataLinkState(buf[0],word,&curr);
            printf("%x ",buf[0]);
        }
        printf("\n");

        if(currGlobalState == TRANSFER){
            if( word[2] == C_RR0 ){
                frameBackup.frameToSend = 0;
                frameBackup.acceptedFrame = TRUE;
            } else if (word[2] == C_RR1){
                frameBackup.frameToSend = 1;
                frameBackup.acceptedFrame = TRUE;
            } else if (word[2] == C_REJ0 || word[2] == C_REJ1){ 
                frameBackup.acceptedFrame = FALSE;
            }
        }
        
        currDataState = START; 
        printf("\n");
        
    }

    sendControl(fd, C_UA);
    alarm(0);


    sleep(2);
   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }


    close(fd);
    return 0;
}
