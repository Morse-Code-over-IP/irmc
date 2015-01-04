// Structures for the packets: unsigned short command
#define DIS 0x0002 // disconnect
#define DAT 0x0003 
#define CON 0x0004 // connect
#define ACK 0x0005

// This structure will be used to (dis-)connect to KOB servers
struct command_packet_format{
    	unsigned short command; // CON / DIS
    	unsigned short channel; // Channel number
};
#define SIZE_COMMAND_PACKET 4
// This structure will be used for id, rx and tx packets
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
#define SIZE_DATA_PACKET 496

// Define the packets used
#define DEFAULT_CHANNEL 103
struct command_packet_format connect_packet = {CON, DEFAULT_CHANNEL}; 
struct command_packet_format disconnect_packet = {DIS, 0};
struct data_packet_format id_packet;
struct data_packet_format rx_data_packet;
struct data_packet_format tx_data_packet;


