/*
   Simple udp server
   */
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

#define BUFLEN 512  //Max length of buffer

//C structure for UDP Segment
struct UDP_SEGMENT {
	unsigned short int srcPort;
	unsigned short int destPort;
	unsigned short int length;
	unsigned short int checksum;
	char payload[256];
};


void die(char *s)
{
	perror(s);
	exit(1);
}

int main (int argc, char *argv[])
{
	struct sockaddr_in si_me, si_other;
	struct UDP_SEGMENT segment;     
	int sockfd, i, slen = sizeof(si_other) , recv_len;
	char buf[BUFLEN];
	char msg[BUFLEN];   
 	char buffer[BUFLEN];
	int carry = 0; //used to calculate checksum
	//FILE *fp1, *fp2; 
	
	//command line argument test
	if (argc != 2) {
		printf("Enter port number\n");
		die("Port No");
	}

	int PORT = atoi(argv[1]); 
	//create a UDP socket
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		die("socket");
	}

	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	//bind socket to port
	if(bind(sockfd, (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	{
		die("bind");
	}

	//keep listening for data
	while(1)
	{
		printf("Waiting for data...\n");
		fflush(stdout);
		bzero (buf, BUFLEN);        
		bzero(msg, BUFLEN); 
		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(sockfd, msg, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
		{
			die("recvfrom()");
		}


		//print details of the client/peer and the data received
		printf("Received request for port number from a client having port number %d.\n", ntohs(si_other.sin_port));
		sprintf(buf,"%d",ntohs(si_other.sin_port));    

		//now reply the client with the same data
		if (sendto(sockfd, buf, BUFLEN, 0, (struct sockaddr*) &si_other, slen) == -1)
		{
			die("sendto()");
		}
		printf("Port number  sent to the client.\n"); 

		bzero(buf, sizeof(buf));

		//setting segment to 0
		memset(&segment,0, sizeof(segment)); 

		//recieve segment from client
		if ((recv_len = recvfrom(sockfd, &segment, sizeof(segment), 0, (struct sockaddr *) &si_other, &slen)) == -1)
		{
			die("recvfrom()");
		}
		else{
			printf("Segment recieved\n"); 
		} 

		//initializing the values in UDP segment to calculate its checksum
		unsigned short int temp = segment.checksum; //store value of segment.checksum temporarily 
		segment.checksum = (unsigned short) 0; //Initialize the recieved checksum to be zer 

		//copy
	       sprintf(buffer, "Destination Port = %hu, Source Port = %hu , length of segment  =%hu, Checksum = %hu\nPayload: %s\n", segment.destPort, segment.srcPort, segment.length, temp,segment.payload);

	       //write output to file
		FILE *fp;    	
		fp = fopen("output.txt", "a"); 
                if (fp == NULL) {
                        die("File"); 
             }

	     	fprintf(fp, "%s\n", segment.payload); 
	
                fclose(fp); 
		
		//write server,log file
		FILE *fp1;
                fp1 = fopen("server.log", "a");
                if (fp1 == NULL) {
                        die("File");
                }

                fprintf(fp, "%s", buffer); 
		bzero(buffer, BUFLEN); 
                fclose(fp1);
		
		//print info
		printf("Destination Port = %hu, Source Port = %hu , length of segment  =%hu, Checksum = %hu\n", segment.destPort, segment.srcPort, segment.length, temp);
		printf("Payload: %s\n",segment.payload);  
			
		unsigned short int checksumCalc[segment.length/2];
		//copying the UDP structucure into an array where each element is 16 bits 
		memcpy(&checksumCalc, &segment, sizeof(segment));

		//calculating checksum
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
	
		printf("Calculated Checksum: %hu\n", segment.checksum); 
		if (segment.checksum == temp) {
			printf("No packets were lost\n");
			temp = 0;
		}
		else {
			printf("Packets were lost\n"); 
		}
		
		printf("\n"); 		
	}	

	close(sockfd);
	return 0;
}
