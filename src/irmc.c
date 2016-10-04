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
#include <sys/ioctl.h>
#include <math.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>

//#define DEBUG 1

#define MAXDATASIZE 1024 // max number of bytes we can get at once 

#include "cwprotocol.h"
#include "beep.h"
#include "util.h"

int serial_status = 0, fd_serial, numbytes;

double tx_timeout = 0;
long tx_timer = 0;
#define TX_WAIT  5000
#define TX_TIMEOUT 240.0 
#define KEEPALIVE_CYCLE 100

/* TX Methods */
#define TX_NONE 0
#define TX_SERIAL 1
#define TX_KEYBOARD 2

long key_press_t1;
long key_release_t1;
int last_message = 0;
char last_sender[16];

/* settings */
int translate = 1;
int audio_status = 1;



// disconnect from the server
void inthandler(int sig)
{
	signal(sig, SIG_IGN);
	send(fd_socket, &disconnect_packet, SIZE_COMMAND_PACKET, 0);	
	close(fd_socket);
	close(fd_serial);
	exit(1);
}

void txloop (void)
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
		if(tx_data_packet.n == SIZE_CODE) {
			printf("irmc: warning packet is full.\n");
			return;
		}
	}
}


void message(int msg)
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


/* Main Loop */
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
	char id[SIZE_ID];
	char serialport[64];
	int tx_method = TX_NONE;

	// Set default values
	snprintf(hostname, 64, "mtc-kob.dyndns.org");
	snprintf(port, 16, "7890");
	channel = 103;
	snprintf(id, SIZE_ID, "irmc-default");
	snprintf(serialport, 64, "");

	// Read commandline
	opterr = 0; 
	int c;
	while ((c = getopt (argc, argv, "h:p:c:i:s:")) != -1)
	{
		switch (c) 
		{
			case 'h':
				snprintf(hostname, 64, "%s", optarg);
				break;
			case 'p':
				snprintf(port, 16, "%s", optarg);
				break;
			case 'c':
				channel = atoi (optarg);
				break;
			case 'i':
				snprintf(id, SIZE_ID, "%s", optarg);
				break;
			case 's':
				snprintf(serialport, 64, "%s", optarg);
				break;
			case '?':
	    			fprintf(stderr, "irmc - Internet Relay Morse Code\n\n");
				fprintf(stderr, "usage: irmc [arguments]\n\n");
				fprintf(stderr, "Arguments:\n\n");
				fprintf(stderr, "   -h [hostname]     Hostname of morsekob server. Default: %s\n", hostname);
				fprintf(stderr, "   -p [port]         Port of morsekob server. Default: %s\n", port);
				fprintf(stderr, "   -c [channel]      Channel. Default: %d\n", channel);
				fprintf(stderr, "   -i [id]           My ID. Default: %s\n", id);
				fprintf(stderr, "   -s [serialport]   Serial port device name. Example: /dev/tty.usbserial Default: \"%s\"\n", serialport);
				return 1;
			default: 
				abort ();
		}
	}

	// Preparing connection
	fprintf(stderr, "irmc - Internet Relay Morse Code\n\n");
	fprintf(stderr, "Connecting to %s:%s on channel %d with ID %s.\n", hostname, port, channel, id); 

	prepare_id (&id_packet, id);
	prepare_tx (&tx_data_packet, id);
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
		fprintf(stderr, "Failed to connect.\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	fprintf(stderr, "Connected to %s.\n", s);
	beep_init();
	if ((strcmp (serialport, "")) != 0) 
		tx_method = TX_SERIAL; 

	if (tx_method == TX_SERIAL) {
		fd_serial = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);
		if(fd_serial == -1) {
    			fprintf(stderr,"Unable to open serial port %s.\n", serialport);
    		}
	}
	freeaddrinfo(servinfo); /* all done with this structure */

	key_release_t1 = fastclock();
	identifyclient();
    
	/* Main Loop */
	for(;;) {
  		if(tx_timer == 0) 
			if((numbytes = recv(fd_socket, buf, MAXDATASIZE-1, 0)) == -1)
				usleep(250);
		if(numbytes == SIZE_DATA_PACKET && tx_timer == 0){
			memcpy(&rx_data_packet, buf, SIZE_DATA_PACKET);

		#if DEBUG  
			printf("length: %i\n", rx_data_packet.length);
			printf("id: %s\n", rx_data_packet.id);
			printf("sequence no.: %i\n", rx_data_packet.sequence);
			printf("version: %s\n", rx_data_packet.status);
			printf("n: %i\n", rx_data_packet.n);
			printf("code:\n");
			for(i = 0; i < SIZE_CODE; i++)printf("%i ", rx_data_packet.code[i]); printf("\n");
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
			for(i = 0; i < 5; i++) send(fd_socket, &tx_data_packet, SIZE_DATA_PACKET, 0);
		#if DEBUG		
			printf("irmc: sent data packet.\n");
		#endif
			tx_data_packet.n = 0;
		}
		if (tx_method == TX_SERIAL) {
			ioctl(fd_serial,TIOCMGET, &serial_status);
			if(serial_status & TIOCM_DSR){
				txloop();
				tx_timer = TX_WAIT;
				message(1);
			}
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
		}
	} /* End of mainloop */

	send(fd_socket, &disconnect_packet, SIZE_COMMAND_PACKET, 0);	
	close(fd_socket);
	close(fd_serial);
	buzzer_stop();

	exit(0); 
}

