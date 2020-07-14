// by Yueh-Ting Chen (eopXD)
// 2018 Computer Network Final Project -- Chatroom


/*
This file is of server structure. 
The basic structure supports file descriptor to be well handled by fd_set.
User initialization by reading user data file, "asset/userprofile".
Creates a folder with username  for newly registered user.


*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server.h"

int socket_fd;
char buf[MAX_TXT_LENGTH];

int to_number ( char* s ) {
	int res = 0;
	for ( int i=0; i<strlen(s); i++ ) {
		if ( !('9'>=s[i] && s[i]>='0') ) return -1;
		else res = res*10 + s[i] - '0';
	}
	return res;
}
	
// usage: ./chat_server [PORT]
int main ( int argc, char **argv ) 
{

// Wrong argument ~ QAQ
#define WRONG_ARG do {\
	fprintf (stderr, "usage: ./chat_server [PORT_NUM]\n");\
	exit (1);\
} while ( 0 )
	if ( argc != 2 ) WRONG_ARG;
	if ( to_number ( argv[1] ) < 0 ) WRONG_ARG;

	User_list all_user;
	all_user.user_cnt=0;
	init ( &all_user );

	socket_fd=gogo_server ( to_number( argv[1] ) );
	if ( socket_fd==-1 ) {
		fprintf ( stderr, "server: fail to start server\n" );
		exit (1);
	}
// server is now listening...
	fprintf ( stdout, "Now listening on port %d\n", to_number ( argv[1] ) );
	fprintf ( stdout, "I'm a good guy with a good heart, and I have good intentions.\n\n\n" );

	int conn_fd;
	struct sockaddr_in client_addr;				// new client address
	int addr_len;
	addr_len=sizeof ( client_addr );

	char hostnames[MAX_CLIENT][128];			// hostname table
	int ports[MAX_CLIENT];						// port_num table
	User *online_user[MAX_CLIENT];				// online user table

	
	for ( int i=0; i<MAX_CLIENT; ++i ) 
		online_user[i]=NULL;
	
	fd_set master_fds, read_fds;				// read_fds is a copy of current master_fds
	FD_ZERO ( &master_fds );
  	FD_SET ( socket_fd, &master_fds);

  	Packet *pkt_ptr;
  	User *usr_ptr;
  	while ( 1 ) {
  		read_fds=master_fds;
  		if ( select ( MAX_CLIENT, &read_fds, NULL, NULL, NULL)<0 ) {
  			fprintf (stderr, "server: select\n" );
  			exit (1);
  		}
  		for ( int i=0; i<MAX_CLIENT; ++i ) {
  			if ( FD_ISSET ( i, &read_fds) ) {
  				if ( i==socket_fd ) {			// conneciton request on master socket
  					if ( (conn_fd=accept ( socket_fd, (struct sockaddr*)&client_addr, (socklen_t*)&addr_len))<0 ) {
		   				fprintf ( stderr, "server: accept\n" );
            			exit (1);
					}
					fprintf ( stdout, "connection: %s:%d\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port) );
					strcpy ( hostnames[conn_fd], inet_ntoa (client_addr.sin_addr) );
	                ports[conn_fd]=ntohs ( client_addr.sin_port );
	            	FD_SET ( conn_fd, &master_fds );
  				}
  				else {
  					if ( (pkt_ptr=recv_pkt ( i ))==NULL ) {
  						fprintf ( stdout, "GG ~~~ QAQ\n" );
  						exit (1);
  					}
  					fprintf ( stdout, "==================================================\n" );
            fprintf ( stdout, "packet: receive type %d from %s:%d\n", pkt_ptr->type, hostnames[i], ports[i] );
  					fprintf ( stdout, "len: %ld, text: ", pkt_ptr->len ); fflush ( stdout );
  					write ( 1, pkt_ptr->text, pkt_ptr->len ); fflush ( stdout );
  					fprintf ( stdout, "\n\n" ); fflush ( stdout );
  					
  					// BYE BYE
  					if ( pkt_ptr->type==BYE_BYE ) {
  						if ( online_user[i]!=NULL ) fprintf ( stdout, "bye: %s\n", online_user[i]->usr );
  						online_user[i]=NULL;
  						close ( i );
  						FD_CLR ( i, &master_fds );
  					}
  					else if ( pkt_ptr->type==REGISTER ) {
  						fprintf ( stdout, "register: start register\n" );
  						usr_ptr=user_parse ( pkt_ptr->len, pkt_ptr->text );
  						if ( usr_ptr==NULL ) {
  							fprintf ( stdout, "register: fail\n" );
  							send_pkt ( i, REGISTER_FAIL, strlen(register_fail), register_fail );
  						}
  						else {
  							add ( &all_user, usr_ptr );
  							fprintf ( stdout, "register: success\n" );
  							send_pkt ( i, REGISTER_GOOD, strlen(register_good), register_good );
  						}
//  					free_usr ( usr_ptr ); this is not needed or else memory in need will be free OAO
  						fprintf ( stdout, "resgister: end register\n" );
  					}
  					else if ( pkt_ptr->type==LOGIN ) {
  						fprintf ( stdout, "login: start login\n" );
  						usr_ptr=user_parse ( pkt_ptr->len, pkt_ptr->text );
  						if ( find ( &all_user, usr_ptr )==NULL ) {
  							fprintf ( stdout, "login: fail\n" );
  							send_pkt ( i, LOGIN_FAIL, strlen(login_fail), login_fail );
  						}
  						else {
  							online_user[i]=find ( &all_user, usr_ptr );
  							fprintf ( stdout, "login: success %s\n", online_user[i]->usr );
  							send_pkt ( i, LOGIN_GOOD, strlen(login_good), login_good );
  						}
  						free_usr ( usr_ptr );
  						fprintf ( stdout, "login: end login\n" );
  					}
  					else if ( pkt_ptr->type==ONLINE_LIST ) {
  						fprintf ( stdout, "online: start query\n" );
  						send_online_user ( i, online_user ); // make sure to insert the right socket = =
  						fprintf ( stdout, "online: finish query\n" );
  					}
  					else if ( pkt_ptr->type==MSG_SEND ) {
  						fprintf ( stdout, "msg_send: start send\n" );
  						send_msg_to ( i, online_user[i], pkt_ptr->len, pkt_ptr->text );
  						fprintf ( stdout, "msg_send: end send\n" );
  					}
  					else if ( pkt_ptr->type==MSG_RECV ) {
  						fprintf ( stdout, "msg_recv: start recv\n" );
  						recv_msg_to ( i, online_user[i], pkt_ptr->len, pkt_ptr->text );
  						fprintf ( stdout, "msg_recv: end recv\n" );
  					}
  					else if ( pkt_ptr->type==MSG_LOG ) {
  						fprintf ( stdout, "msg_log: start log\n" );
  						view_log ( i, online_user[i], pkt_ptr->len, pkt_ptr->text );
  						fprintf ( stdout, "msg_log: end log\n" );
  					}
  					else if ( pkt_ptr->type==FILE_NAME ) {
  						fprintf ( stdout, "file_send: start send\n" );
  						recv_file ( i, online_user[i], pkt_ptr->len, pkt_ptr->text );
  						fprintf ( stdout, "file_send: end send\n" );
  					}
  				  else if ( pkt_ptr->type==FILE_DOWNLOAD || pkt_ptr->type==FILE_DOWNLOAD_END ) {
  						fprintf ( stdout, "file_recv: start send\n" );
  						send_file ( i, online_user[i], pkt_ptr->len, pkt_ptr->text, pkt_ptr->type );
  						fprintf ( stdout, "file_recv: end send\n" );
  					}
            else if ( pkt_ptr->type==CHANGE_PWD ) {
              fprintf ( stdout, "change_pwd: start change\n" );
              change_pwd ( i, online_user[i], &all_user, pkt_ptr->len, pkt_ptr->text );
              fprintf ( stdout, "change_pwd: end change\n" );
            }
            else if ( pkt_ptr->type==CLEAR_LOG ) {
              fprintf ( stdout, "clear_log: start change\n" );
              clear_log ( i, online_user[i] );
              fprintf ( stdout, "clear_log: end change\n" );
            }
  					else {
  						fprintf ( stdout, "receive strange scary file OAO ~~~\n" );
  					}
  					fprintf ( stdout, "packet: end of packet\n" );
  					fflush ( stdout );
  					free_pkt ( pkt_ptr );
  				}
  			}
  		}
  	}
	exit (0);
}