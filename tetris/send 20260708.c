#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<errno.h>

#include<sys/socket.h>
#include<sys/types.h>
#include<sys/ioctl.h>

#include<net/if.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<netinet/if_ether.h>
#include<netinet/udp.h>

#include<linux/if_packet.h>

#include<arpa/inet.h>

#include<unistd.h>


struct ifreq ifreq_i;
int sock_raw;
unsigned char* sendbuff;
int total_len = 0, send_len;

struct sockaddr_ll sadr_ll;

void send_data_process();

//--------------------------------
char* eth_dev_name = "ens33";
int buff_len = 64;
//--------------------------------

void get_eth_index()
{
	memset(&ifreq_i, 0, sizeof(ifreq_i));
	strncpy(ifreq_i.ifr_name, eth_dev_name, IFNAMSIZ - 1);

	if ((ioctl(sock_raw, SIOCGIFINDEX, &ifreq_i)) < 0)
		printf("error in index ioctl reading");

	printf("index=%d\n", ifreq_i.ifr_ifindex);
}

int send_net(unsigned char* sendbuff, int buff_len)
{
	int send_len = 0;
	send_len = sendto(sock_raw, sendbuff, buff_len, 0, (const struct sockaddr*)&sadr_ll, sizeof(struct sockaddr_ll));
	return send_len;
}


int count = 0;

int main()
{
	sock_raw = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
	if (sock_raw == -1)
		printf("error in socket");

	sendbuff = (unsigned char*)malloc(buff_len); // increase in case of large data.Here data is --> AA  BB  CC  DD  EE
	memset(sendbuff, 0, buff_len);

	get_eth_index();  // interface number

	sadr_ll.sll_ifindex = ifreq_i.ifr_ifindex;
	sadr_ll.sll_halen = ETH_ALEN;

	send_data_process();

}


unsigned short checksum(unsigned short* buff, int len_16)
{
	unsigned long sum = 0;

	for (int i = 0; i < len_16>0; i++)
	{
		sum += htons(*(buff)++);
	}

	do

	{
		sum = ((sum >> 16) + (sum & 0xFFFF));
	} while (sum & 0xFFFF0000);

	return (~sum);
}


//===========================================================

// uint8_t dst_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Destination MAC Address : FF:FF:FF:FF:FF:FF
// uint8_t src_mac[6] = {0xE4, 0xFD, 0x45, 0x4A, 0x51, 0xC8};
// uint16_t type_arp_ip = 0X0800;
// uint16_t hardware_type = 0x0001;
// uint16_t protocol_type = 0x0800;
// uint8_t hardware_length = 6;
// uint8_t protocol_length = 4;
// uint16_t arp_op_code = 1; // 1: request, 2: reply
// uint8_t send_mac[6] = {0x00, 0xFD, 0x45, 0x4A, 0x51, 0xC8};
// uint8_t send_ip[4] = {172, 30, 1, 30};
// uint8_t target_mac[6] = {0xE4, 0xFD, 0x45, 0x4A, 0x0A, 0x88};
// uint8_t target_ip[4] = {172, 30, 1, 7};
enum {
	PROTOCOL_NUM_TCP = 6,
	PROTOCOL_NUM_UDP = 17
};


// IP HEAD
uint8_t dst_mac[6] = { 0x45, 0xFD, 0x45, 0x4A, 0x51, 0xC8 }; // Destination MAC Address : FF:FF:FF:FF:FF:FF
uint8_t src_mac[6] = { 0x00, 0x0C, 0x29, 0x45, 0x48, 0x45 };
uint16_t type_arp_ip = 0X0800;
uint8_t ip_version_head_length = 0x45;
uint8_t tos = 0x00;
uint16_t ip_total_length = 46 - 14;
uint16_t ip_id = 0x00;
uint16_t ip_floag_fragment_offset = 0x00;
uint8_t ip_ttl = 128;
uint8_t ip_protocol = PROTOCOL_NUM_UDP; // 6 : TCP, 17 : UDP
uint8_t source_ip[4] = { 172, 30, 1, 64 };
uint8_t destination_ip[4] = { 172, 30, 1, 30 };

// UDP head	
uint16_t udp_src_port = 3000;
uint16_t udp_dst_port = 3000;
uint16_t udp_length = 8; // UDP header length + data length
uint16_t udp_checksum = 0;
uint16_t udp_total_length = 46 - 14 - 20; // Total length of UDP header and data (46 bytes total - 14 bytes Ethernet header - 20 bytes IP header)
uint8_t udp_data[4] = { 'h', 'a', 'h', 'a' }; // Example data to send in UDP packet


// void buff_init()
// {

// 	for (int i = 0; i <6; i++)
// 	{
// 		sendbuff[i]= dst_mac[i];
// 		sendbuff[i+6]= src_mac[i];

// 	}
// 	// // Destination MAC Address : 00:01:02:03:04:05
// 	// sendbuff[0] = 0xFF;
// 	// sendbuff[1] = 0xFF;
// 	// sendbuff[2] = 0xFF;
// 	// sendbuff[3] = 0xFF;
// 	// sendbuff[4] = 0xFF;
// 	// sendbuff[5] = 0xFF;
// 	// // Source MAC Address : 00:0c:29:92:6a:6c	
// 	// // My Mac Address(linux) : 00:0c:29:45:48:45
// 	// sendbuff[6] = 0xE4;
// 	// sendbuff[7] = 0xFD;
// 	// sendbuff[8] = 0x45;
// 	// sendbuff[9] = 0x4A;
// 	// sendbuff[10] = 0x51;
// 	// sendbuff[11] = 0xC8;

// 	// // Type = ARP : 0x0806, IP : 0x0800
// 	sendbuff[12] = (type_arp_ip >> 8) & 0xff;
// 	sendbuff[13] = type_arp_ip & 0xff;
// 	// //------------------------------

// 	// // ARP Packet
// 	// // Hardware tpye : Protocol Tpye(0X0800)
// 	// // Hardware Len(6) : Protoco Len(4) : Op Code (Req:1, Res:2)
// 	// // Send Mac address(6 Byte : e4:fd:45:4a:51:c8)
// 	// // Send IP Address(4 Byte : 172.30.1.64)
// 	// // Target Mac Address(6 Byte : 00:0c:29:45:48:45)
// 	// // Target IP Address(4 Byte : 172.30.1.254)

// 	sendbuff[14] = (hardware_type >>8) & 0xff; // Hardware Type : upper
// 	sendbuff[15] = hardware_type & 0xff; // Hardware Type : lower
// 	sendbuff[16] = (protocol_type >> 8) & 0xff; // Protocol Type : upper
// 	sendbuff[17] = protocol_type & 0xff; // Protocol Type : lower
// 	sendbuff[18] = hardware_length; // Hardware Length	
// 	sendbuff[19] = protocol_length; // Protocol Length
// 	sendbuff[20] = (arp_op_code >> 8) & 0xff; // Op Code : upper
// 	sendbuff[21] = arp_op_code & 0xff; // Op Code : lower

// 	for (int i = 0; i < 6; i++)
// 	{
// 		sendbuff[i+22] = send_ip[i]; // Sender Mac Address
// 		sendbuff[i+32] = target_ip[i];

// 	}

// 		for (int i = 0; i < 6; i++)
// 	{
// 		sendbuff[i+28] = send_ip[i]; // Sender Mac Address
// 		sendbuff[i+38] = target_ip[i];

// 	}

// 	// // sender Mac Address 00:0c:29:45:48:45
// 	// sendbuff[22] = 0xE4;
// 	// sendbuff[23] = 0xFD;
// 	// sendbuff[24] = 0x45;
// 	// sendbuff[25] = 0x4A;
// 	// sendbuff[26] = 0x51;
// 	// sendbuff[27] = 0xC8;
// 	// // sender IP Address

// 	// sendbuff[28] = 172;
// 	// sendbuff[29] = 30;
// 	// sendbuff[30] = 1;
// 	// sendbuff[31] = 30;


// 	// // Target Mac Address
// 	// sendbuff[32] = 0xe4;
// 	// sendbuff[33] = 0xfd;
// 	// sendbuff[34] = 0x45;
// 	// sendbuff[35] = 0x4a;
// 	// sendbuff[36] = 0x0a;
// 	// sendbuff[37] = 0x88;

// 	// // Target IP Address
// 	// sendbuff[38] = 172;
// 	// sendbuff[39] = 30;
// 	// sendbuff[40] = 1;
// 	// sendbuff[41] = 7;


// 	// //

// 	buff_len = 42;
// }


void buff_init()
{

	//Ethernet head
	for (int i = 0; i < 6; i++)
	{

		sendbuff[i] = dst_mac[i];
		sendbuff[i + 6] = src_mac[i];


	}


	// //Ethernet
	// //Destination MAC Address : 00:01:02:03:04:05
	// sendbuff[0] = 0xE4;
	// sendbuff[1] = 0xFD;
	// sendbuff[2] = 0x45;
	// sendbuff[3] = 0x4A;
	// sendbuff[4] = 0x51;
	// sendbuff[5] = 0xC8;
	// // // Source MAC Address : 00:0c:29:92:6a:6c	
	// // // My Mac Address(linux) : 00:0c:29:45:48:45
	// sendbuff[6] = 0x00;
	// sendbuff[7] = 0x0C;
	// sendbuff[8] = 0x29;
	// sendbuff[9] = 0x45;
	// sendbuff[10] = 0x48;
	// sendbuff[11] = 0x45;

	// // Type = ARP : 0x0806, IP : 0x0800
	sendbuff[12] = (type_arp_ip >> 8) & 0xff;
	sendbuff[13] = type_arp_ip & 0xff;

	// ============================= IP head 
	sendbuff[14] = 0x45; // Version(4) + Header Length(5)
	// TOS
	sendbuff[15] = tos; // Type of Service
	// Total Length
	sendbuff[16] = (ip_total_length & 0xff00) >> 8;
	sendbuff[17] = ip_total_length & 0x00ff;
	//ID
	sendbuff[18] = (ip_id & 0xff00) >> 8;
	sendbuff[19] = ip_id & 0x00ff;
	// Flag . Fragment Offset
	sendbuff[20] = (ip_floag_fragment_offset >> 8) & 0xff;
	sendbuff[21] = ip_floag_fragment_offset & 0x00ff;
	// TTL
	sendbuff[22] = ip_ttl;
	// Protocol
	sendbuff[23] = ip_protocol; // 6 : TCP, 17 : UDP

	//checksum

	sendbuff[24] == 0;
	sendbuff[25] == 0;


	for (int i = 0; i < 4; i++)
	{
		sendbuff[i + 26] = source_ip[i]; // source ip : Linux
		sendbuff[i + 30] = destination_ip[i]; // Destination ip : Windows
	}
	// // source IP (Linux) 
	// sendbuff[26] = 172;
	// sendbuff[27] = 30;
	// sendbuff[28] = 1;
	// sendbuff[29] = 64;
	// // Destination IP (Windows) 
	// sendbuff[30] = 172;
	// sendbuff[31] = 30;
	// sendbuff[32] = 1;
	// sendbuff[33] = 30;


	unsigned short header_checksum = checksum((unsigned short*)(&sendbuff[14]), 20 / 2);  // 20 bytes
	sendbuff[24] = (header_checksum & 0xff00) >> 8;
	sendbuff[25] = header_checksum & 0xff;


	//=====================UDP haed
	//source port
	sendbuff[34] = (udp_src_port >> 8) & 0xff; // (0x0000 >> 8) & 0xff
	sendbuff[35] = udp_src_port & 0xff;
	//destination port
	sendbuff[36] = (udp_dst_port >> 8) & 0xff;
	sendbuff[37] = udp_dst_port & 0xff;
	// totoal length
	sendbuff[38] = udp_total_length >> 8 & 0xff;
	sendbuff[39] = udp_total_length & 0xff;
	// checksum
	 //sendbuff[40] = 0;
	 //sendbuff[41] = 0;


	sendbuff[40] = (udp_checksum >> 8) & 0xff;
	sendbuff[41] = udp_checksum & 0xff;


	for (int i = 0; i < 4; i++)
	{
		sendbuff[i + 42] = udp_data[i]; // Example data to send in UDP packet
	}

	// sendbuff[42] = 'h';
	// sendbuff[43] = 'a';
	// sendbuff[44] = 'h';
	// sendbuff[45] = 'a';

	buff_len = 46;

}







void send_data_process()
{

	buff_init();

	printf("sending...\n");
	while (1)
	{
		send_len = send_net(sendbuff, buff_len);
		if (send_len < 0)
		{
			printf("sendlen=%d....errno=%d\n", send_len, errno);
			return -1;
		}

		printf("count = %d\n", count++);
		usleep(1000000);
	}
}
