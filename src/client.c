// by Yueh-Ting Chen (eopXD)
// 2018 Computer Network Final Project -- Chatroom

/*
This file is of client structure. 
Client creates one and only socket. Clients go into some input procedures when commands are set.
Packet will be sent once input procedures is complete.
Logging out will also terminate the client program. (but will take time send BYE packet to server)

*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "client.h"


int socket_fd;
int to_number ( char* s ) {
	int res = 0;
	for ( int i=0; i<strlen(s); i++ ) {
		if ( !('9'>=s[i] && s[i]>='0') ) return -1;
		else res = res*10 + s[i] - '0';
	}
	return res;
}
void int_Handler () {
	fprintf (stderr, "\nrecieved SIGINT, saying goodbye to server\n");
	send_logout ( socket_fd );

	fprintf ( stdout, "\n\n\nHave a nice day ~ \n" );
	exit (0);
}


char buf[MAX_TXT_LENGTH];
//usage: ./chat_client [SERVER_IP] [PORT]
int main ( int argc, char *argv[] ) 
{
// Wrong argument ~ QAQ
#define WRONG_ARG do {\
	fprintf (stderr, "usage: ./chat_client [SERVER_IP] [PORT]\n");\
	exit (1);\
} while ( 0 )

	if ( argc!=3 ) WRONG_ARG;
	if ( to_number ( argv[2] )<0 ) WRONG_ARG;

	if ( (socket_fd=hook_server ( argv[1], to_number ( argv[2] ) ))<=0 ) WRONG_ARG;
	fprintf ( stdout, "connected to server %s:%d\n", argv[1], to_number(argv[2]) );
	fd_set fds;
	FD_ZERO ( &fds );
	FD_SET ( socket_fd, &fds );

	Packet *pkt_ptr;
	User *usr_ptr;

// Catch SIGINT signal
	signal ( SIGINT, int_Handler );

	// register/login/logout: done
	char op[20];
	while ( 1 ) { // not logged in session
		fprintf ( stdout, "\n\n==============================================================================================\n" );
		fprintf ( stdout, "** welcome to the messaging system\n" );
		fprintf ( stdout, "==============================================================================================\n" );	fflush ( stdout );
		
		fprintf ( stdout, "\navalibalbe command: REGISTER, LOGIN, EXIT\n> ");
		fscanf ( stdin, "%s", op );
		if ( strcmp ( op, "REGISTER" )==0 ) {
			usr_ptr=send_register ( socket_fd );
			free_usr ( usr_ptr );
		}
		else if ( strcmp ( op, "LOGIN" )==0 ) {
			usr_ptr=send_login ( socket_fd );
		}
		else if ( strcmp ( op, "EXIT" )==0 ) {
			send_logout ( socket_fd );
			exit (0);
		}
		else {
			fprintf ( stdout, "unknown command\n" );
			continue;
		}
		sleep (1);
		if ( FD_ISSET ( socket_fd, &fds) ) {
			if ( (pkt_ptr=recv_pkt ( socket_fd ))==NULL ) {
				continue;
			}
			else {
				fprintf ( stderr, "receive type %d\n", pkt_ptr->type );
				write ( 1, pkt_ptr->text, pkt_ptr->len );
			}
		}
		if ( pkt_ptr->type==LOGIN_GOOD ) {
			fprintf ( stdout, "ya ~ you logged in\n" );
			free_pkt ( pkt_ptr );
			break;
		}		
		free_pkt ( pkt_ptr );
	}
	while ( 1 ) { // logged in session
		fprintf ( stdout, "\n\n==============================================================================================\n" );
		fprintf ( stdout, "hello %s, you can start using the message system\n", usr_ptr->usr );
		fprintf ( stdout, "==============================================================================================\n" );	fflush ( stdout );
		// file transfer need to be added
		fprintf ( stdout, "\nAvailable command: MSG_SEND, MSG_RECV, MSG_LOG, FILE_SEND, FILE_RECV, WHO_ONLINE, WEATHER\n, GET_QUOTE, CHANGE_PWD, DELETE_LOG, LOGOUT\n\n> ");
		fscanf ( stdin, "%s", op );
		if ( strcmp ( op, "LOGOUT" )==0 ) {
			send_logout ( socket_fd );
			break;
		}
		else if ( strcmp ( op, "MSG_SEND" )==0 ) {
			send_msg ( socket_fd );
		}
		else if ( strcmp ( op, "MSG_RECV" )==0 ) {
			recv_msg ( socket_fd );
		}
		else if ( strcmp ( op, "MSG_LOG" )==0 ) {
			msg_log ( socket_fd );
		}
		else if ( strcmp ( op, "FILE_SEND" )==0 ) {
			send_file ( socket_fd );
		}
		else if ( strcmp ( op, "FILE_RECV" )==0 ) {
			recv_file ( socket_fd );
		}
		else if ( strcmp (op, "WHO_ONLINE" )==0 ){ 		// feature 1: query who is online
			send_who_online ( socket_fd );
		}
		else if ( strcmp ( op, "WEATHER" )==0 ) { 		// feature 2: weather forecast
			get_weather ();
			fflush ( stdout );
			continue;
		}
		else if ( strcmp ( op, "GET_QUOTE" )==0 ) {		// feature 3: daily quote
			get_funny_quote ();
			fflush ( stdout );
			continue;
		}
		else if ( strcmp ( op, "CHANGE_PWD" )==0 ) { 	// feature 4: change password
			change_pwd ( socket_fd );
		}
		else if ( strcmp ( op, "DELETE_LOG" )==0 ) {	// feature 5: delete log history
			log_clear ( socket_fd );
		}
		else {
			fprintf ( stdout, "unknown command\n" );
			fflush ( stdout );
			continue; 
		}
		fflush ( stdout );
		//fprintf ( stdout, "\nwaiting for server response... \n\n" ); fflush ( stdout );
		sleep (2);
		if ( FD_ISSET ( socket_fd, &fds) ) {
			if ( (pkt_ptr=recv_pkt ( socket_fd ))==NULL ) {
				continue;
			}
			else {
				char response_head[120]="\n******************************* Start of response *******************************************\n";
				char response_tail[120]="\n******************************* End of response *********************************************\n";
				write ( 1, response_head, strlen(response_head) );
				write ( 1, pkt_ptr->text, pkt_ptr->len );
				write ( 1, response_tail, strlen(response_tail) );
				fflush ( stdout );
			}
			free_pkt ( pkt_ptr );
		}
		else {
			char response_head[120]="\n******************************* Start of response *******************************************";
			char response_tail[120]="\n******************************* End of response *********************************************\n";	
		}
	}
	exit (0);
}