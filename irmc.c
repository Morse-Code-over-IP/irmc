/* irmc - internet relay morsecode client */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <math.h>
#include <fcntl.h>
#include <morse/beep.h>
#ifdef __MACH__
#define LIBOSS_INTERNAL
#include <liboss/soundcard.h> //will not be used for audio any more
#else
#include <linux/ioctl.h>
#include <asm-generic/ioctl.h>
#include <asm-generic/termios.h>
#endif 
#include <signal.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
 
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif
 

#define MAXDATASIZE 1024 // max number of bytes we can get at once 

#define DIS 0x0002
#define DAT 0x0003
#define CON 0x0004
#define ACK 0x0005

#define DEBUG 0

struct command_packet_format{
    	unsigned short command;
    	unsigned short channel;
};
struct data_packet_format{
    	unsigned short command;
    	unsigned short length;
    	char id[128];
    	char a1[4];
    	unsigned int sequence;
	unsigned int a21;  
	unsigned int a22;
	unsigned int a23;
    	signed int code[51];
    	unsigned int n;
    	char status[128];  /* This is called version in MorseKob */
    	char a4[8];
    
};
struct code_packet_format{
    	unsigned short command;
    	unsigned short length;
    	char id[128];
    	char a1[4];
    	unsigned int sequence;
	unsigned int a21;
	unsigned int a22;
	unsigned int a23;
    	signed int code[51];
    	unsigned int n;
    	char a3[128];
    	char a4[8];
};

struct command_packet_format connect_packet;
struct command_packet_format disconnect_packet = {DIS, 0};
struct data_packet_format id_packet;
struct data_packet_format rx_data_packet;
struct data_packet_format tx_data_packet;
int serial_status = 0, fd_speaker, fd_serial, fd_socket, numbytes;
int tx_sequence = 0, rx_sequence;

double tx_timeout = 0;
long tx_timer = 0;
#define TX_WAIT  5000
#define TX_TIMEOUT 240.0 
#define KEEPALIVE_CYCLE 100

long key_press_t1;
long key_release_t1;
int last_message = 0;
char last_sender[16];

/* settings */
int translate = 0;
int audio_status = 1;

/* portable time, as listed in https://gist.github.com/jbenet/1087739  */
void current_utc_time(struct timespec *ts) {
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  ts->tv_sec = mts.tv_sec;
  ts->tv_nsec = mts.tv_nsec;
#else
  clock_gettime(CLOCK_REALTIME, ts);
#endif
 
}

/* a better clock() in milliseconds */
long 
fastclock(void)
{
	struct timespec t;
	long r;

	current_utc_time (&t);
	r = t.tv_sec * 1000;
	r = r + t.tv_nsec / 1000000;
	return r;
}

int kbhit (void)
{
  	struct timeval tv;
  	fd_set rdfs;

  	tv.tv_sec = 0;
  	tv.tv_usec = 0;

  	FD_ZERO(&rdfs);
  	FD_SET (STDIN_FILENO, &rdfs);

  	select (STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
  	return FD_ISSET(STDIN_FILENO, &rdfs);

}


/* get sockaddr, IPv4 or IPv6: */
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void
identifyclient(void)
{
	tx_sequence++;
	id_packet.sequence = tx_sequence;
	send(fd_socket, &connect_packet, sizeof(connect_packet), 0);
	send(fd_socket, &id_packet, 496, 0);
}

void
inthandler(int sig)
{
	signal(sig, SIG_IGN);
	send(fd_socket, &disconnect_packet, sizeof(disconnect_packet), 0);	
	close(fd_socket);
	close(fd_speaker);
	close(fd_serial);
	exit(1);
}

void
txloop (void)
{
	key_press_t1 = fastclock();
	tx_timeout = 0;
	for(;;){
		tx_data_packet.n++;
		tx_data_packet.code[tx_data_packet.n - 1] = 
			(int) ((key_press_t1 - key_release_t1) * -1);

		//printf("space: %i\n", tx_data_packet.code[tx_data_packet.n -1]);
		while(serial_status & TIOCM_DSR) ioctl(fd_serial, TIOCMGET, &serial_status);
		key_release_t1 = fastclock();

		tx_data_packet.n++;	
		tx_data_packet.code[tx_data_packet.n - 1] =
			(int) ((key_release_t1 - key_press_t1) * 1);
		
		//printf("mark: %i\n", tx_data_packet.code[tx_data_packet.n -1]);
		while(1){
			ioctl(fd_serial, TIOCMGET, &serial_status);
			if(serial_status & TIOCM_DSR) break;
			tx_timeout = fastclock() - key_release_t1;
			if(tx_timeout > TX_TIMEOUT) return;
		}
		key_press_t1 = fastclock();	
		if(tx_data_packet.n == 50) {
			printf("irmc: warning packet is full.\n");
			return;
		}
	}
}

int
commandmode(void)
{
	char cmd[32];
	int i;

	last_message = 0; /* reset status message */
	printf(".");
	fgets(cmd, 32, stdin);
	if(strncmp(cmd, ".", 1) == 0){
		printf("\n");
		return 1;
	}
	if((strncmp(cmd, "latch", 3)) == 0){
		tx_sequence++;
		tx_data_packet.sequence = tx_sequence;
		tx_data_packet.code[0] = -1;
		tx_data_packet.code[1] = 1;
		tx_data_packet.n = 2;
		for(i = 0; i < 5; i++) send(fd_socket, &tx_data_packet, 496, 0);
		tx_data_packet.n = 0;
		return 0;
	}

	if((strncmp(cmd, "unlatch", 3)) == 0){
		tx_sequence++;
		tx_data_packet.sequence = tx_sequence;
		tx_data_packet.code[0] = -1;
		tx_data_packet.code[1] = 2;
		tx_data_packet.n = 2;
		for(i = 0; i < 5; i++) send(fd_socket, &tx_data_packet, 496, 0);
		tx_data_packet.n = 0;
		return 0;
	}
	if((strncmp(cmd, "ton", 3)) == 0){
		translate = 1;
		return 0;
	}
	if((strncmp(cmd, "toff", 3)) == 0){
		translate = 0;
		return 0;
	}
	
	if((strncmp(cmd, "aon", 3)) == 0){
		audio_status = 1;
		return 0;
	}
	if((strncmp(cmd, "aoff", 3)) == 0){
		audio_status = 0;
		return 0;
	}
	printf("?\n");
	return 0;
}


void
message(int msg)
{       
	switch(msg){
	case 1:
		if(last_message == msg) return;
		if(last_message == 2) printf("\n");
		last_message = msg;
		printf("irmc: transmitting...\n");
		break;
	case 2:
		if(last_message == msg && strncmp(last_sender, rx_data_packet.id, 3) == 0) return;
		else {
			if(last_message == 2) printf("\n");
			last_message = msg;
			strncpy(last_sender, rx_data_packet.id, 3);
			printf("irmc: receiving...(%s)\n", rx_data_packet.id);
		}
		break;
	case 3:
		printf("irmc: circuit was latched by %s.\n", rx_data_packet.id);
		break;
	case 4:
		printf("irmc: circuit was unlatched by %s.\n", rx_data_packet.id);
		break;
	default:
		break;
	}
	fflush(0);
}
	 
int main(int argc, char *argv[])
{
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv, i;
	char s[INET6_ADDRSTRLEN];
	int keepalive_t = 0;
	char hostname[64];
	char port[16];
	int channel;
	char id[128];
	char serialport[64];

	if (argc < 4) {
	    fprintf(stderr," %i usage: irmc [hostname] [port] [channel] [id] [serialport]\n", argc);
	    exit(1);
	}

	snprintf(hostname, 64, argv[1], "%s");
	snprintf(port, 16, argv[2], "%s");
	channel = atoi(argv[3]);
	if(argc > 4) snprintf(id, 128, argv[4], "%s");
	else snprintf(id, 128, "irmc"); 
	if(argc > 5) snprintf(serialport, 64, argv[5], "%s");

	id_packet.command = DAT;
	id_packet.length = 492;
	snprintf(id_packet.id, 128, id, "%s");
	id_packet.sequence = 0;
	id_packet.n = 0;
	snprintf(id_packet.status, 128, "irmc v0.02");
	id_packet.a21 = 1;     /* These magic numbers was provided by Les Kerr */
	id_packet.a22 = 755;
	id_packet.a23 = 65535;
	
	tx_data_packet.command = DAT;
	tx_data_packet.length = 492;
	snprintf(tx_data_packet.id, 128,  id, "%s");
	tx_data_packet.sequence = 0;
	tx_data_packet.n = 0;
	for(i = 1; i < 51; i++)tx_data_packet.code[i] = 0;
	tx_data_packet.a21 = 0; /* These magic numbers was provided by Les Kerr */
	tx_data_packet.a22 = 755;
	tx_data_packet.a23 = 16777215;
	snprintf(tx_data_packet.status, 128, "?");
	
	connect_packet.command = CON;
	connect_packet.channel = channel; 
		
	
	signal(SIGINT, inthandler);
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; /* ipv4 or ipv6 */
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	/* Find the first free socket */
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((fd_socket = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("irmc: socket");
			continue;
		}

		if (connect(fd_socket, p->ai_addr, p->ai_addrlen) == -1) {
			close(fd_socket);
			perror("irmc: connect");
			continue;
		}

		break;
	}
    
    
    	fcntl(fd_socket, F_SETFL, O_NONBLOCK);
	if (p == NULL) {
		fprintf(stderr, "irmc: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("irmc: connected to %s\n", s);
	beep_init();
	fd_serial = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);
	if(fd_serial == -1) {
    		printf("irmc: unable to open serial port.\n");
    	}
	freeaddrinfo(servinfo); /* all done with this structure */

	key_release_t1 = fastclock();
	identifyclient();
    
	/* Main Loop */
	for(;;) {
  		if(tx_timer == 0) 
			if((numbytes = recv(fd_socket, buf, MAXDATASIZE-1, 0)) == -1)
				usleep(250);
		if(numbytes == 496 && tx_timer == 0){
			memcpy(&rx_data_packet, buf, 496);
		#if DEBUG  
			printf("length: %i\n", rx_data_packet.length);
			printf("id: %s\n", rx_data_packet.id);
			printf("sequence no.: %i\n", rx_data_packet.sequence);
			printf("version: %s\n", rx_data_packet.status);
			printf("n: %i\n", rx_data_packet.n);
			printf("code:\n");
			for(i = 0; i < 51; i++)printf("%i ", rx_data_packet.code[i]); printf("\n");
		#endif
			if(rx_data_packet.n > 0 && rx_sequence != rx_data_packet.sequence){
				message(2);
				if(translate == 1){
					printf("%s", rx_data_packet.status);
					fflush(0);
				}
				rx_sequence = rx_data_packet.sequence;
				for(i = 0; i < rx_data_packet.n; i++){
					switch(rx_data_packet.code[i]){
					case 1:
						message(3);
						break;
					case 2:
						message(4);
						break;
					default:
						if(audio_status == 1)
						{ 

int length = rx_data_packet.code[i];
if(length == 0 || abs(length) > 2000) {
}
else
{
if(length < 0) {
beep(0.0, abs(length)/1000.);
}
else
{
beep(1000.0, length/1000.);
}
}
						}
						break;
					}
				}
			}
		}
	
		if(tx_timer > 0) tx_timer--;
		if(tx_data_packet.n > 1 ){
			tx_sequence++;
			tx_data_packet.sequence = tx_sequence;
			for(i = 0; i < 5; i++) send(fd_socket, &tx_data_packet, 496, 0);
		#if DEBUG		
			printf("irmc: sent data packet.\n");
		#endif
			tx_data_packet.n = 0;
		}
		ioctl(fd_serial,TIOCMGET, &serial_status);
		if(serial_status & TIOCM_DSR){
			txloop();
			tx_timer = TX_WAIT;
			message(1);
		}
		
		if(keepalive_t < 0 && tx_timer == 0){
		#if DEBUG
			printf("keep alive sent.\n");
		#endif
			identifyclient();
			keepalive_t = KEEPALIVE_CYCLE;
		}
		if(tx_timer == 0) {
			keepalive_t--;
			usleep(50);	
		}		

		if(kbhit() && tx_timer == 0){
			getchar(); /* flush the buffer */
			if(commandmode()== 1)break;
		}
	} /* End of mainloop */

	send(fd_socket, &disconnect_packet, sizeof(disconnect_packet), 0);	
	close(fd_socket);
	close(fd_serial);

	exit(0); 
}

