#include <stdio.h>
#include "cwprotocol.h"

int prepare_id (struct data_packet_format *id_packet, char *id)
{
	id_packet->command = DAT;
	id_packet->length = SIZE_DATA_PACKET_PAYLOAD;
	snprintf(id_packet->id, SIZE_ID, id, "%s");
	id_packet->sequence = 0;
	id_packet->n = 0;
	snprintf(id_packet->status, SIZE_ID, "irmc v0.02");
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
	snprintf(tx_packet->status, SIZE_STATUS, "?");
	
	return 0;
}
