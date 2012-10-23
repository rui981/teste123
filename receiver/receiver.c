#include "receiver.h"

int llopen(char *arg, int fd) {

	struct termios oldtio, newtio;

	fd = open(arg, O_RDWR | O_NOCTTY);
	// fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY );
	if (fd < 0) {
		perror("Can't open serial port config\n");
		return -1;
	}

	if (tcgetattr(fd, &oldtio) == -1) { /* save current port settings */
		perror("tcgetattr");
		return -1;
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
	newtio.c_cc[VMIN] = 4; /* blocking read until 4 chars received */

	/*
	 VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
	 leitura do(s) próximo(s) caracter(es)
	 */

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
		perror("tcsetattr");
		return -1;
	}

	printf("New termios structure set\n");
	unsigned char * temp=NULL;
	int t=-1;
	if (llread(fd, temp) > 0) {
		printf("TRAMA RECEIVED\n");
		t=verTramaS(temp, fd) ;
		printf("type %i\n", t);
		temp=setTrama(t);
		if(t>0){
			llwrite(fd, temp);
		printf("UA SENT\n");
		printf("Open receiver side done!\n");
		}else{
			printf("li lodo");
		}
	}

	return fd;
}
int llwrite(int fd, unsigned char * buffer) {
	unsigned int nr;
	nr = sizeof(buffer);
	int res = 0;
	res = write(fd, buffer, nr);
	printf("write return: %d\n", res);
	return res;
}

int llread(int fd, unsigned char * buffer) {
	unsigned int nr;
	int res;
	nr = sizeof(buffer);
	res = read(fd, buffer, nr);
	printf("%i",nr);
	printf("read return: %d\n", res);
	return res;
}

int llclose(int fd) {

	unsigned char * ctempR=NULL;

	if (llread(fd, ctempR) > 0) {
		verTramaS(ctempR, fd);
		unsigned char * t=NULL;

		if (llread(fd, t) > 0) {
			if (verTramaS(t, fd) > 0) {
				printf("Disconnected\n");
			}
		}
	}

	close(fd);
	return 0;
}

int main(int argc, char *argv[]) {

	int fd, c, res, serialPort;
	struct termios oldtio, newtio;
	unsigned char buff[5] = "";

	unsigned char buf[255]; //write/read

	if ((argc < 2)
			|| ((strcmp("/dev/ttyS0", argv[1]) != 0)
					&& (strcmp("/dev/ttyS1", argv[1]) != 0))) {
		printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
		exit(1);
	}

	//Abrir ligação
	serialPort = llopen(argv[1], fd);
	//llclose(fd);
	//llread(serialPort, buff);

}

unsigned char* setTrama(int type) {

	unsigned char buff[5];

	buff[0] = FLAG;
	buff[1] = A;
	buff[4] = FLAG;

	switch (type) {
//O: SET
//1: DISC
//2: UA
//3: RR
//4: REJ
	case 0: {
		buff[2] = SET;
		buff[3] = buff[2] ^ A;
	}
		break;
	case 1: {
		buff[2] = DISC;
		buff[3] = buff[2] ^ A;
	}
		break;
	case 2: {
		buff[2] = UA;
		buff[3] = buff[2] ^ A;
	}
		break;
	}
	return buff;
}

int verTramaS(unsigned char *set, int fd) {
	char state = 'I';
	int processed = 0;
	int answer=-1;
	//case FLAG
	//case A
	//case C
	//case BCC1
	//case FLAG
	while (!processed) {

		switch (state) {
		case 'I': {
			if (set[0] == FLAG) {
				state = 'F';
			}
		}
			break;

		case 'F': {
			if (set[1] == A) {
				state = 'C';
			}
		}
			break;

		case 'C': {
			if (set[2] == SET) {
				printf("RECEBI SET");
				answer=2;
				//llwrite(fd, setTrama(2));
				state = 'B';
			}

			if (set[2] == DISC) {
				answer=1;
				//llwrite(fd, setTrama(1));
				state = 'B';
			}

			if (set[2] == UA) {
				printf("Disconnected\n");
				state='B';
			}
			else{
				answer=-1;
			}
		}break;
		case 'B': {
			unsigned char at = set[1];
			unsigned char ct = set[2];
			unsigned char bcc = at ^ ct;
			if (bcc == set[3]) {
				printf("Message received");
				state = 'Q';
			}else{
				answer=-1;
			}
		}
			break;
		case 'Q': {
			if (set[4] == FLAG) {
				processed = 1;
			}
		}break;
		}
	}
	return answer;
}
