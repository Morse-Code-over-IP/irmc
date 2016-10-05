#include <stdio.h>
#include <sys/socket.h>
#include <ctype.h>

#include "cwprotocol.h"

/* Global variables */
struct command_packet_format connect_packet = {CON, DEFAULT_CHANNEL}; 
struct command_packet_format disconnect_packet = {DIS, 0};
struct data_packet_format id_packet;
struct data_packet_format rx_data_packet;
struct data_packet_format tx_data_packet;

int tx_sequence = 0, rx_sequence;
int fd_socket;

struct morse_timig_format morse_timing;

int prepare_id (struct data_packet_format *id_packet, char *id)
{
	id_packet->command = DAT;
	id_packet->length = SIZE_DATA_PACKET_PAYLOAD;
	snprintf(id_packet->id, SIZE_ID, id, "%s");
	id_packet->sequence = 0;
	id_packet->n = 0;
	snprintf(id_packet->status, SIZE_ID, INTERFACE_VERSION);
	id_packet->a21 = 1;     /* These magic numbers was provided by Les Kerr */
	id_packet->a22 = 755;
	id_packet->a23 = 65535;

	return 0;
}


int prepare_tx (struct data_packet_format *tx_packet, char *id)
{
	int i;

	tx_packet->command = DAT;
	tx_packet->length = SIZE_DATA_PACKET_PAYLOAD;
	snprintf(tx_packet->id, SIZE_ID,  id, "%s");
	tx_packet->sequence = 0;
	tx_packet->n = 0;
	for(i = 1; i < 51; i++)tx_packet->code[i] = 0;
	tx_packet->a21 = 0; /* These magic numbers was provided by Les Kerr */
	tx_packet->a22 = 755;
	tx_packet->a23 = 16777215;
	snprintf(tx_packet->status, SIZE_STATUS, "?"); // this shall include the sent character
	
	return 0;
}


// connect to server and send my id.
void identifyclient(void)
{
	tx_sequence++;
	id_packet.sequence = tx_sequence;
	send(fd_socket, &connect_packet, SIZE_COMMAND_PACKET, 0);
	send(fd_socket, &id_packet, SIZE_DATA_PACKET, 0);
}

int send_latch (void)
{
	int i;
	tx_sequence++;
	tx_data_packet.sequence = tx_sequence;
	tx_data_packet.code[0] = -1;
	tx_data_packet.code[1] = 1;
	tx_data_packet.n = 2;
	for(i = 0; i < 5; i++) send(fd_socket, &tx_data_packet, SIZE_DATA_PACKET, 0);
	tx_data_packet.n = 0;
	return 0;
}

int send_unlatch (void)
{
	int i;
	tx_sequence++;
	tx_data_packet.sequence = tx_sequence;
	tx_data_packet.code[0] = -1;
	tx_data_packet.code[1] = 2;
	tx_data_packet.n = 2;
	for(i = 0; i < 5; i++) send(fd_socket, &tx_data_packet, SIZE_DATA_PACKET, 0);
	tx_data_packet.n = 0;
	return 0;
}

int prepare_text2morse (int wpm)
{
	morse_timing.wpm = wpm;
	morse_timing.dot_len = (int) (1200/wpm); // in ms - PARIS standard
	morse_timing.dash_len = 3*morse_timing.dot_len;
	morse_timing.charspace_len = 3*morse_timing.dot_len;
	morse_timing.wordspace_len = 7*morse_timing.dot_len;

	return 0;
}

int char2morse(void)
{
	char c='c', d;
	// why? because!!!
	// http://stackoverflow.com/questions/1352587/convert-a-string-into-morse-code/1355594^
	for(;c= c?c:(c=toupper(getchar())-32)?c<0?1:"\x95#\x8CKa`^ZRBCEIQiw#S#nx(37+$6-2&@/4)'18=,*%.:0;?5" [c-12]-34:-3;c/=2)
	{
		d=(c/2?46-c%2:32);
		putchar (d);
	}
	return 0;
}
