// by Yueh-Ting Chen (eopXD)
// 2018 Computer Network Final Project -- Chatroom

/*
This file is of communication between client and server.
Functions including ...

Server linkage:
	gogo_server ();
	hook_server ();

Packet I/O:
	Packet* recv_pkt ( int sd );
	int send_pkt ( int sd, char type, long len, char *buf );
	void free_pkt ( Packet *pkt );
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>


#include "common.h"
/*----------------------------------------*/

/* 
gogo_server ();
Starts server.
Argument:
	port = port number

Result: 
	return (sd)	= success
	return -1	= fail
*/
/*---------Packet operations--------------*/
int gogo_server ( int port ) {
	int socket_fd;
	struct sockaddr_in addr;
	int tmp=1;
// create socket
	if ( (socket_fd = socket ( AF_INET, SOCK_STREAM, 0)) < 0 ) {
		fprintf (stderr, "gogo_server: create socket\n" );
    	return (-1);
	}
	if ( setsockopt ( socket_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&tmp, sizeof(tmp)) < 0 ) {
    	fprintf (stderr, "gogo_server: set socket option\n" );
		return (-1);
    }
// bind socket
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons ( port );
    if ( bind ( socket_fd, (struct sockaddr *)&addr, sizeof(addr) ) < 0 )    {   
    	fprintf (stderr, "gogo_server: bind\n" );
		return (-1);
    } 
// listen
    if ( listen ( socket_fd, MAX_CLIENT ) < 0 ) {   
        fprintf ( stderr, "gogo_server: listen\n" );
        return (-1);   
    } 
// list out all interface
    fprintf ( stdout, "List of interface: \n" );
    struct ifaddrs *addrs, *addr_ptr;
    getifaddrs(&addrs);
	addr_ptr=addrs;
	while ( addr_ptr ) {
	    if (addr_ptr->ifa_addr && addr_ptr->ifa_addr->sa_family == AF_INET) {
	        struct sockaddr_in *pAddr = (struct sockaddr_in *)addr_ptr->ifa_addr;
	        fprintf ( stdout, "%s: %s\n", addr_ptr->ifa_name, inet_ntoa(pAddr->sin_addr) );
	    }
	    addr_ptr=addr_ptr->ifa_next;
	}
	fprintf ( stdout, "==================================================\n" );
	freeifaddrs ( addrs );
	return (socket_fd);
}
/*
hook_server ();
Hooks server.
No argument:

Result: 
	return sd 	= success
	return 0	= connection fail
	return -1	= other failure
*/
int hook_server ( char *host, int port ) {
// DNS lookup
	struct hostent *hname;
	if ( (hname = gethostbyname ( host )) == NULL ) {
		fprintf (stderr, "hook_server: gethostbyname\n" );
		return (-1);
	}
	struct sockaddr_in addr;
	addr.sin_family=hname->h_addrtype;
	addr.sin_port=htons ( port );
	addr.sin_addr.s_addr=*(long*) hname->h_addr;
// create socket
	int socket_fd;
	int tmp=1;

	if ( (socket_fd = socket ( AF_INET, SOCK_STREAM, 0)) < 0 ) {
		fprintf (stderr, "hook_server: create socket\n" );
    	return (-1);
	}
	if ( setsockopt ( socket_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&tmp, sizeof(tmp)) < 0 ) {
    	fprintf (stderr, "hook_server: set socket option\n" );
		return (-1);
    }
	if ( connect ( socket_fd, (struct sockaddr *) &addr, sizeof(struct sockaddr) ) != 0 ) {
		fprintf (stderr, "hook_server: %s connect fail\n",host);
		return (0);
	}		
	return (socket_fd);
}
/*
int readn ( int sd, char *buf, int n );
Reads n bytes into buffer from socket descriptor

Argument:
	sd 			= socket descriptor
	buf 		= buffer
	n 			= bytes to be read 
Result:
	return 1 	= success
	return 0	= fail
*/
int readn ( int sd, char *buf, int n ) {
	int nn;
	char *ptr;
	nn=n, ptr=buf;
	while ( nn>0 ) {
		int bytesread;
		bytesread=read ( sd, ptr, n );
		if ( bytesread<=0 ) {
			fprintf ( stderr, "readn: read\n" );
			return (0);
		}
		nn-=bytesread;
		ptr+=bytesread;
	}
	return (1);
}
/*
Packet* recv_pkt ( int sd );
Receives packet from socket descriptor
(Refer to "common.h" for packet structure)

Argument:
	sd 			= socket descriptor
Result:
	return received packet pointer
*/
Packet* recv_pkt ( int sd ) {
	Packet *pkt;
	pkt= (Packet *) calloc ( 1, sizeof(Packet) );
	if ( !pkt ) {
		fprintf ( stderr, "recv_pkt: calloc\n" );
		return (NULL);
	}
// reads type, len, text respectively
	if ( !readn ( sd, (char *) &pkt->type, sizeof(pkt->type) ) ) {
		fprintf ( stderr, "recv_pkt: read type\n" );
		free ( pkt );
		return (NULL);
	}
	if ( !readn ( sd, (char *) &pkt->len, sizeof(pkt->len) ) ) {
		fprintf ( stderr, "recv_pkt: read length\n" );
		free ( pkt );
		return (NULL);
	}
	// allocate for text
	pkt->len=ntohl ( pkt->len ); // vs. htonl()
	if ( pkt->len>0 ) {
		pkt->text=(char *) malloc ( pkt->len );
		for ( int i=0; i<pkt->len; ++i ) pkt->text[i]=0; // the importance to turn everything to ZER0
		if ( !pkt ) {
			fprintf ( stderr, "recv_pkt: malloc\n" );
			free ( pkt );
			return (NULL);
		}
		if ( !readn ( sd, pkt->text, pkt->len ) ) { // found bug lah ~~~
			fprintf ( stderr, "recv_pkt: read text\n" );
			free ( pkt->text );
			free ( pkt );
			return (NULL);
		}
	}
//	fprintf ( stderr, "recv_pkt: len %ld, %s\n", pkt->len, pkt->text );
	return (pkt);
}
/*
int send_pkt ( int sd, char type, long len, char *buf );
Sends packet to specified socket descriptor

Argument:
	sd 			= socket descriptor
	type 		= Packet->type
	len 		= Packet->len
	buf 		= Packet->buf 
Result:
	return 1 	= success
	return 0	= fail
*/
int send_pkt ( int sd, char type, long len, char *buf ) {
	char tmp[8];
	long sz;
	bcopy ( &type, tmp, sizeof(type) );
	sz=htonl ( len ); // vs. ntohl()
	bcopy ( (char *) &sz, tmp+sizeof(type), sizeof(sz) );
	if ( write ( sd, tmp, sizeof(type)+sizeof(sz) ) < 0 ) {
		fprintf ( stderr, "send_pkt: send type+len\n" );
		return (0);
	}
	if ( len>0 ) {
//		fprintf ( stderr, "send_pkt: len %ld, %s\n", len, buf );
		if ( write ( sd, buf, len ) < 0 ) {
			fprintf ( stderr, "send_pkt: send buffer\n" );
			return (0);
		}
	}
	return (1);
}
/*
void free_pkt ( Packet *pkt );
Frees packet from eternity.

Argument: 
	pkt 		= packet pointer
No result.
*/
void free_pkt ( Packet *pkt ) {
	free ( pkt->text );
	free ( pkt );
}

/*--------------User operations-------------*/
int trans_number ( char* s ) {
	int res = 0;
	for ( int i=0; i<strlen(s); i++ ) {
		if ( !('9'>=s[i] && s[i]>='0') ) return -1;
		else res = res*10 + s[i] - '0';
	}
	return res;
}
User* user_parse ( int len, char *buf ) {
// format: "usr:[USR_LENGTH]:[USR]:pwd:[PWD_LENGTH]:[PWD]:"
	char parse[10][256];
	int x=0,y=0;
	for ( int i=0; i<len; ++i ) {
		if ( buf[i]==':' ) {
			parse[x][y++]='\0';
			++x, y=0;
			if ( x>5 ) break;
		}
		else parse[x][y++]=buf[i];
	}
	if ( x!=6 ) {
		fprintf ( stderr, "user_parse: does not receive 6 blocks\n" );
		return (NULL);
	}
	int usr_len,pwd_len;
	if ( strcmp(parse[0],"usr")==0 && strcmp(parse[3],"pwd")==0 ) {
		if ( (usr_len=trans_number ( parse[1] ))<0 ) {
			fprintf ( stderr, "user_parse: usr_len\n" );
			return (NULL);
		}
		if ( (pwd_len=trans_number ( parse[4] ))<0 ) {
			fprintf ( stderr, "user_parse: pwd_len\n" );
			return (NULL);
		}
		User *usr_ptr;

		usr_ptr= (User *) calloc ( 1, sizeof(User) );
		usr_ptr->usr=(char *) malloc ( usr_len+2 );
		usr_ptr->pwd=(char *) malloc ( pwd_len+2 );
		strcpy ( usr_ptr->usr, parse[2] );
		strcpy ( usr_ptr->pwd, parse[5] );
		fprintf ( stderr, "user_parse: parse success\n" );
		return usr_ptr;
	}
	else {
		fprintf ( stderr, "block name not right\n" );
		return (NULL);
	}
}
void free_usr ( User *usr ) {
	free ( usr->usr );
	free ( usr->pwd );
	free ( usr );
}
/*------------------------------------------*/