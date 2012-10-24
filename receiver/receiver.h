
#ifndef RECEIVER_H
#define RECEIVER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>


#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7d
#define ENDFLAG 0x7a
#define A 0x01
#define I 0x00
#define TRANSMITER 0
#define RECEIVER 1
#define SET 0x03
#define DISC 0x0b
#define UA 0x07
#define RR 0x05
#define REJ 0x01
#define BCC 0xB1
#define BCC2 0xB

int llopen(char *arg, int fd); //Open connection
int llwrite(int fd, unsigned char * buffer);  
int llread(int fd, unsigned char * buffer);
int llclose(int fd);
void setTrama(int type, unsigned char *buff, unsigned int length);
int verTramaS(unsigned char *set, int fd);
void imprimeTrama(unsigned char * tr);
#endif


