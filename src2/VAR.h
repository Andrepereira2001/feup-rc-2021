#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

//Data Link Action 
#define ESTABLISH 0
#define TRANSFER 1
#define TERMINATE 2

#define FLAG 0x7e
#define AE_SENT 0x03 // Comandos enviados por emissor / resposta recetor 
#define AR_SENT 0x01 // Comandos enviados por recetor / resposta emissor

#define C_SET 0x03 // control set up
#define C_DISC 0x0B // control disconnect
#define C_UA 0x07 // control unnumbered acknowlegement
#define C_RR0 0x05 // control receiver ready 0
#define C_RR1 0x85 // control receiver ready 1
#define C_REJ0 0x03 // control reject 0
#define C_REJ1 0x83 // control reject 1
#define C_S0 0x00 // control sent 0 - information frame
#define C_S1 0x40 // control sent 1 - information frame

#define O_ESC 0x7d // escape octet
#define O_FST 0x5e // substitui byte FLAG
#define O_SND 0x5d // substitui byte O_ESC
