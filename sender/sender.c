#include "sender.h"

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
	newtio.c_cc[VMIN] = 5; /* blocking read until 5 chars received */

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
	int t = -1;

	unsigned char tr[5]={FLAG,A,A,A,A};
	unsigned char * trame= tr;
	setTrama(0, tr, 5);
	
	imprimeTrama(tr);
	if (llwrite(fd, tr) > 0) {
		printf("Enviei um set\n");
		
		if (llread(fd, tr) > 0) {
			t = verTramaS(tr, fd);

			if (t > 0) {
				printf("recebi um pacote inteiro\n");
				setTrama(t, tr, 5);
				llwrite(fd,tr);
				printf("enviei um pacote tipo %i\n", t);

			}
		}
	}

	return fd;
}

llwrite(int fd, unsigned char * buffer) {
	unsigned int nr;
	nr = sizeof(buffer);
	int res = 0;
	res = write(fd, buffer, 5);
	printf("write return: %d\n", res);
	return res;
}

llread(int fd, unsigned char * buffer) {
	unsigned int nr;
	int res;
	nr = sizeof(buffer);
	res = read(fd, buffer, 5);
	printf("read return: %d\n", res);
	return res;
}

llclose(int fd) {
/*
	unsigned char * ctempW;
	unsigned char * ctempR;
	ctempW = setTrama(1);
	llwrite(fd, ctempW);

	if (llread(fd, ctempR) > 0) {
		verTramaS(ctempR, fd);
		unsigned char * t = setTrama(2);

		if (llwrite(fd, t) > 0) {
			printf("Disconnected\n");
		}
	}*/

	close(fd);
	return 0;
}

int main(int argc, char *argv[]) {

	int fd, c, res, serialPort;
	struct termios oldtio, newtio;
	unsigned char buff[5] = "ola";
	unsigned char set = SET;

	unsigned char buf[255]; //write/read

	if ((argc < 2)
			|| ((strcmp("/dev/ttyS0", argv[1]) != 0)
					&& (strcmp("/dev/ttyS1", argv[1]) != 0))) {
		printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
		exit(1);
	}

	//Abrir ligação
	serialPort = llopen(argv[1], fd);
	llclose(fd);
	//llwrite(serialPort, set); 

}

void setTrama(int type, unsigned char *buff, unsigned int length) {

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
}

int verTramaS(unsigned char *set, int fd) {
	char state = 'I';
	int processed = 0;
	int answer = -1;
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
				answer = 0;
				//llwrite(fd, setTrama(2));
				state = 'B';
			} break;

			if (set[2] == DISC) {
				printf("recebi um disc\n");
				answer = 2;
				//llwrite(fd, setTrama(1));
				state = 'B';
			}break;

			if (set[2] == UA) {
				printf("start the data\n");
				state = 'B';
			} else {
				answer = -1;
			}break;
		}
			break;
		case 'B': {
			unsigned char at = set[1];
			unsigned char ct = set[2];
			unsigned char bcc = at ^ ct;
			if (bcc == set[3]) {
				printf("Message received");
				state = 'Q';
			} else {
				answer = -1;
			}
		}
			break;
		case 'Q': {
			if (set[4] == FLAG) {
				processed = 1;
				answer=1;
			}
		}
			break;
		}
	}
	return answer;
}


void imprimeTrama(unsigned char * tr){
	printf("F = %u\n", tr[0]);
	printf("A = %u\n", tr[1]);
	printf("C = %u\n", tr[2]);
	printf("BCC = %u\n", tr[3]);
	printf("F = %u\n", tr[4]);
}
