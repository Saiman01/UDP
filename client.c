/*
   Simple udp client
   */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER "129.120.151.95"
#define BUFLEN 512  //Max length of buffer

void die(char *s)
{
	perror(s);
	exit(1);
}
//C structure for UDP Segment
struct UDP_SEGMENT {
	unsigned short int srcPort;
	unsigned short int destPort;
	unsigned short int length;
	unsigned short int checksum; 
	char payload[256]; 
};

int main (int argc, char *argv[])
{
	struct sockaddr_in si_other;
	struct UDP_SEGMENT segment;
	int sockfd, i=0, slen=sizeof(si_other);
	char buf[BUFLEN];
	char message[BUFLEN];
	char file[256];
	char buffer[BUFLEN]; 

	FILE *fp;//file pointer
	int len; 
	//error check
	if (argc != 3) {
		printf("Enter port number and filename.\n");
		exit(1);
	}

	//Port number for Server
	int PORT = atoi(argv[1]);  

	//opening file
	fp = fopen(argv[2], "r"); 
	if (fp == NULL) {
		die("file"); 
	}
	fscanf(fp,"%s",segment.payload); 
	fclose(fp);  

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		die("socket");
	}

	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);

	if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	//Send request message to the client for port 
	bzero(message, sizeof(message));
	sprintf(message, "Request for port"); 

	//send the control message
	if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == -1)
	{
		die("sendto()");
	}

	//clear the buffer by filling null, it might have previously received data
	bzero(buf, sizeof(buf));

	//try to receive some data, this is a blocking call
	if (recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
	{
		die("recvfrom()");
	}

	//initializing the UDP segment   
	segment.srcPort = (unsigned short) atoi(buf);
	segment.destPort =  PORT; 
	segment.length = 264; //8 bytes for srcPort, dstPort, length and checksum combined. 256 byte of payload data. 256 + 8 = 264 bytes 
	segment.checksum = (unsigned short) 0; //set initially to 0
	segment.payload[255] = '\0'; //setting the last character to a null character
  
	//copying the UDP structucure into an array where each element is 16 bits 
	unsigned short int checksumCalc[segment.length/2];
	memcpy(&checksumCalc, &segment, sizeof(segment));

	//calculating checksum
	int carry = 0; 

	for (int i =0; i<segment.length/2; i++){
		//maximum int stored in 16 bit unsigned short is 65,535
		if((checksumCalc[i] + segment.checksum) > 65535){
			carry = 1;
		}
		else {
			carry = 0; 
		}

		segment.checksum += checksumCalc[i]+carry; 
	}    
	 
	//send the message
	if (sendto(sockfd, &segment, sizeof(segment), 0, (struct sockaddr *) &si_other, slen) == -1)
	{
		die("sendto()");
	}
	//print info
	printf("Destination Port = %hu, Source Port = %hu , length of segment  =%hu, Checksum = %hu\n", segment.destPort, segment.srcPort, segment.length, segment.checksum);
	printf("Payload: %s\n", segment.payload);
	sprintf(buffer, "Destination Port = %hu, Source Port = %hu , length of segment  =%hu, Checksum = %hu\nPayload: %s\n", segment.destPort, segment.srcPort, segment.length, segment.checksum ,segment.payload);

	//write to client.log file
	FILE *fp1;
        fp1 = fopen("client.log", "a");
        if (fp1 == NULL) {
        	die("File");
        }

        fprintf(fp, "%s", buffer);
	bzero(buffer, BUFLEN);
	fclose(fp1);

	close(sockfd); 
	return 0;
}
