/*
** talker.c -- a datagram "client" demo
** Originates from http://beej.us/guide/bgnet/examples/talker.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT "4950"	// the port users will be connecting to

int main(int argc, char *argv[])
{
	//!!NOTERA; Övningsuppgiften för UDP är klar här...

	int 			sockfd;			//ID för SocketDescriptor

	struct addrinfo hints,			//För att beskriva vilken typ av Socket vi vill använda; Vilka Protokoll ==> IPv4, udp  
					hints2,			//Visar; Ju mer specifikt vi anger hints, ju kortare kommer den länkade Listan av möjliga socket typer bli;
										//Om vi anger, tex ai_socktype attt vara SOCK_DGRAM så kommer den de länkade entries i servinfo att 
										//endast vara av den typen; Detta medför att endast 1 Möjlig socket kommer att finnas; servinfo.ai_next finns ej! 
					*servinfo, 		//
					*servinfo2,
					*p;	

	int 			rv;
	int 			numbytes;

	if (argc < 3) {
		fprintf(stderr,"usage: talker hostname message [message2] ...\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family 	= AF_UNSPEC;	// AF_UNSPEC; 	Vilket Protokoll som helst i Datalink lagret; IPv4 eller IPv6
	hints.ai_socktype 	= SOCK_DGRAM;	// SOCK_DGRAM;	Används samband med UDP sockets
	
	memset(&hints2, 0, sizeof hints2);
	hints2.ai_family 	= AF_UNSPEC;
	//hints2.ai_socktype	= SOCK_STREAM;	// Eftersom vi anger Socktype så Loopas bara 1 gång.... bortkommenteringen gör att flertalet val visas...

	if ((rv = getaddrinfo(	argv[1],	// argv[1];		Argumentet som skickas med talker programmet, IP adressen till vår server (listener) <-- Alltid samma som datorn, typ
						 	SERVERPORT, // SERVERPORT;	Porten som vår server använder; (Hårdkodad att vara "4950", samma som vår listener använder)
							&hints, 	// hints; 		Beskriver serverinfo baserat på servern som körs på angivna IP och Porten; typ av info
							&servinfo)	// 
							) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	if ((rv = getaddrinfo(	argv[1],	// argv[1];		Argumentet som skickas med talker programmet, IP adressen till vår server (listener) <-- Alltid samma som datorn, typ
						 	SERVERPORT, // SERVERPORT;	Porten som vår server använder; (Hårdkodad att vara "4950", samma som vår listener använder)
							&hints2, 	// hints; 		Beskriver serverinfo baserat på servern som körs på angivna IP och Porten; typ av info
							&servinfo2)	// 
							) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	printf("servinfo som används:\n");
	for(struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
		printf("\tservinfo,hints; \n\t\tFamily/Domain: %d \n\t\tSockType: %d \n\t\tProtocol: %d \n",
							 p->ai_family,		//
							 p->ai_socktype, 	//
							 p->ai_protocol);
		//}
	}
	
	//Printf för Custom test; Innehpllet på servinfo2 (Jämför med servinfo)
	printf("servinfo för test:\n");
	for(struct addrinfo *p = servinfo2; p != NULL; p = p->ai_next) {
		printf("\tservinfo2,hints2; \n\t\tFamily/Domain: %d \n\t\tSockType: %d \n\t\tProtocol: %d \n",
							 p->ai_family,		//
							 p->ai_socktype, 	//
							 p->ai_protocol);
		//}
	}

	// loop through all the results and make a socket
	 for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family,		//
							 p->ai_socktype, 	//
							 p->ai_protocol)	//
							 ) == -1) {
			perror("talker: socket");			// Om Första socket (innan vi går till någon .ai_next) inte går, kommer perror skrivas! 
												// ==> För varje entry vi itererar kommer perror köras... 
			continue;
		}

		//Kommer den valda Socketen fått en 
		printf("\nResultat för Vald Socket\n");
		printf("\tservinfo,hints; \n\t\tFamily/Domain: %d \n\t\tSockType: %d \n\t\tProtocol: %d \n",
							 p->ai_family,		//
							 p->ai_socktype, 	//
							 p->ai_protocol);
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

	char myAddress[20];			// Storleken 20; 
									//Varför? Är det för att allt större än 15  är OK ? 
									//? IPv4 adresser består (som mest) av 15 tecken xxx.xxx.xxx.xxx
									//? IPv6 adresser består (som mest) av ... mer än 15... (vet ej storleken just nu...)
									//Svar;
										// 20 är inte så bra, men vi anger mer än 15 då det ev. kan behövas mer...
										// Dock fungerar detta bara med IPv4 adresser, då IPv6 adresser är längre
	char *myAdd=&myAddress;		

	//int myAdd_len = sizeof(myAddress); 
	socklen_t myAdd_len = sizeof(myAddress); 

	struct sockaddr_in 	local_sin;							// i denna ska vi lagra innehållet av socketen!
	socklen_t 			local_sinlen = sizeof(local_sin);	// Storleken av datatypen för "sockaddr_in"

	//Hämtar information från vår skapta Socket
	getsockname(sockfd,										// id för vår skapta socket
				(struct sockaddr*)&local_sin,				// vi fyller på info om vår socket i "local_sin"; 
				&local_sinlen);								// Adress till Variabeln som lagrar storleken för struct typen 'sockaddr_in'
																//Varför Adress till en storlek?...
																	//Svar: ev. Ändras/uppdateras storleken av vår sockaddr_in struct local_sin 

	//innehållet av vår socket ligger nu lagrad i 'local_sin', där är datan lagrad som "Binary Network"
	//För att hantera datan som kända datatyper (strängar, int) behöver vi konvertera datan; Detta gör 'inet_ntop' 
	//inet_ntop; Från nätverk till 'presentations form'
		// Den behöver information för socketen behöver anges för att översättning ska kunna göras!
	myAdd=inet_ntop(local_sin.sin_family,					// Domän/Familj som vår socket använder
					&local_sin.sin_addr,					// Adressen till IPAdressen som vår socket använder ()
					myAddress,								// En char-array för att lagra IP-adressen som 'sträng'
					myAdd_len);							// Storleken av vår char array.... SKA VARA INTE REFERENS

	printf("Before \tSend>> Sending from  %s:%d \n", myAdd, ntohs(local_sin.sin_port));

	//För varje argument vi skickar till ./talker (antalet beskrivet av argc)
	//gör vi en enskild sendto; Vi kan alltså skicka flera meddelanden genom att separera dem med "Mellanslag"
	for(int q=2;q<argc;q++){
		//När väl Sendto körs så kommer även Port väljas (genom kerneln), därav har vi inget portnummer innan sendto körts
		if ((numbytes = sendto(sockfd,			//Ange socketDescriptor ID  
								argv[q],			//Ange vilken av parametrarna (meddelanderna) vi ska skicka
								strlen(argv[q]), 	//ange storleken av meddelandet
								0,					//Vi skickar INTE med några flagor
								p->ai_addr, 		//Adressen vi ska skicka till, i "Network binary" format
								p->ai_addrlen)		//Storleken av adressen vi ska skicka till, i "Network binary" format
								) == -1) {
			perror("talker: sendto");
			exit(1);
		}

		// Efter att vi skickat meddelandet så skriver vi ut med send igen! 
		// Detta är exakt samma sak som vi gjorde innan send; görs bara för demonstreringsyfte
		// Vi får reda på vad innehållet av en Socket är Före, respektive efter att send/sendto har gjorts! 
			//Vi kommer märka att Porten (och adressen?...) skiljer sig mellan printf före sendto och efter;
			//Före; 	porten är inte vald, denna väljes av processen som skapas av sendto (Något av en gissning...(??))
			//Efter; 	Porten är vald och skrivs därav ut
		getsockname(	sockfd,
						(struct sockaddr*)&local_sin, 
						&local_sinlen);
		
		myAdd=inet_ntop(	local_sin.sin_family,	
							&local_sin.sin_addr,
							myAddress,
							sizeof(myAddress));

		// En skillnad Mellan denna printf och den föregående är att vi skriver ut 'numbytes'; Antalet bytes meddelandet är som vi skicakde! 
		printf("After \tSend>> Sending from  %s:%d, sent %d bytes. \n", myAdd, ntohs(local_sin.sin_port),numbytes);
	  
	}
	
	freeaddrinfo(servinfo);


	close(sockfd);

	return 0;
}
