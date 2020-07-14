// by Yueh-Ting Chen (eopXD)
// 2018 Computer Network Final Project -- Chatroom

/*
This file is of structures and functions needed by the server.
Structures such as:
	1. User structure
	2. Chatroom structure
Functions such as:
	a. User
		1. Register
		2. Login
		3. Logout
	b. Message
		1. Message send
		2. Message receive
		3. Message log
	c. File Transfer
		1. Upload
		2. Download

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
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"

char bigbuf[MAX_TXT_LENGTH];

/*---------------Stucture-------------------*/
typedef struct _user_list {
	int user_cnt;
	User *users[MAX_USER];
} User_list;
/*typedef struct _chatroom {
	char *roomname;
	User_list *members[MAX_CHATROOM_USER];
	int member_cnt;
} Chatroom;*/
/*------------------------------------------*/

/*-------------Static Dialogues-------------*/
static char register_good[15]="register good\n";
static char register_fail[15]="register fail\n";

static char login_good[15]="login good\n";
static char login_fail[15]="login fail\n";
static char password_changed[60]="password successfully changed\n";
static char log_cleared[60]="your log is all cleared\n";
/*------------------------------------------*/

/*--------------User operations-------------*/
int dump ( User_list *usr_list ) {
	fprintf ( stderr, "start dump\n" );
	// delete original file
	unlink ( "asset/userprofile" );
	int saved_stdout, f;
	saved_stdout=dup ( fileno(stdout) );
	f=open ( "asset/userprofile", O_RDWR|O_CREAT, 0777 );
	if ( f==-1 ) {
		fprintf ( stderr, "User_list.dump: open\n" );
		return (-1);
	}
	dup2 ( f, fileno(stdout) );

	for ( int i=0; i<usr_list->user_cnt; ++i ) {
		fprintf ( stdout, "usr:%.3ld:%s:pwd:%.3ld:%s:\n", strlen(usr_list->users[i]->usr), usr_list->users[i]->usr, strlen(usr_list->users[i]->pwd), usr_list->users[i]->pwd);
	}

	fflush ( stdout );
	close ( f );
	dup2 ( saved_stdout, fileno(stdout) );
	close ( saved_stdout );
	fprintf ( stderr, "end dump\n" );
	return 0;
}
int init ( User_list *usr_list ) {
	fprintf ( stderr, "start init\n" );	
	for ( int i=0; i<usr_list->user_cnt; ++i ) { //first release all memory by original array
		free_usr ( usr_list->users[i] );
	}
	int saved_stdin, f;
	saved_stdin=dup ( fileno(stdin) );
	f=open ("asset/userprofile", O_RDWR, 0777 );
	if ( f==-1 ) {
		fprintf ( stderr, "User_list.init: open\n");
		return (-1);
	}
	dup2 ( f, fileno(stdin) );

	usr_list->user_cnt=0;
	char buf[1000]={};
	while ( fscanf ( stdin, "%s", buf )!=EOF ) {
		usr_list->users[usr_list->user_cnt]=user_parse ( strlen(buf), buf );
		if ( usr_list->users[usr_list->user_cnt]==NULL ) {
			fprintf ( stderr, "User_list.init: user_parse" );
			return (-1);
		}
		usr_list->user_cnt++;
	}

	fflush ( stdin );
	close ( f );
	dup2 ( saved_stdin, fileno(stdin) );
	close ( saved_stdin );
	fprintf ( stderr, "end init\n" );
	return 0;
}
User* find ( User_list *usr_list, User *user ) {
	for ( int i=0; i<usr_list->user_cnt; ++i ) {
		if ( strcmp ( user->usr, usr_list->users[i]->usr )==0 && strcmp ( user->pwd, usr_list->users[i]->pwd )==0 ) {
			return usr_list->users[i];
		}
	}
	return (NULL);
}
int add ( User_list *usr_list, User* user ) {
	usr_list->users[usr_list->user_cnt++]=user;
// create folder for user (message storage)
	char buf[100];
	sprintf ( buf, "asset/%s", user->usr );
	mkdir ( buf , 0777 );
	dump ( usr_list );
}
/*------------------------------------------*/

/*---------------Server response------------*/
void send_online_user ( int socket_fd, User **online_user ) {
	char buf[3000]={};
	strcpy ( buf, "\n\nOnline user:\n" );
	for ( int i=0; i<MAX_CLIENT; i++ ) {
		if ( online_user[i]!=NULL ) {
			fprintf ( stderr, "online: %s|\n", online_user[i]->usr );
			strcat ( buf, online_user[i]->usr );
			strcat ( buf, "\n" );
		}
	}
	strcat ( buf, "\n\n\nEnd of list\n\0" );
//	fprintf ( stderr, "%ld, %s", strlen(buf), buf );	
	send_pkt ( socket_fd, ONLINE_LIST, strlen(buf), buf );
}

int send_msg_to ( int socket_fd, User *sender, int len, char *text ) {
	char receiver[MAX_USR_LENGTH]={};
	int msg_start=0;
	fprintf ( stderr, "send_msg_to: start\n" );
	fprintf ( stderr, "send_msg_to: message len %d\n", len );
	while ( text[msg_start]!='\n' ) { receiver[msg_start]=text[msg_start]; msg_start++; } receiver[msg_start++]='\0';

	fprintf ( stderr, "send_msg_to: sender \"%s\", receiver \"%s\"\n", sender->usr, receiver );


// open log files to write
	char sender_file[300]={};
	sprintf ( sender_file, "asset/%s/to_%s", sender->usr, receiver );
	fprintf ( stderr, "send_msg_to: sender file, %s\n", sender_file );
	int sender_log=open ( sender_file, O_APPEND | O_WRONLY | O_CREAT, 0777 );
	if ( sender_log==-1 ) {
		fprintf ( stderr, "send_msg: open sender file\n" );
		return (-1);
	}
	
	char receiver_file[300]={};
	sprintf ( receiver_file, "asset/%s/from_%s.new", receiver, sender->usr );
	int receiver_log=open ( receiver_file, O_APPEND | O_WRONLY | O_CREAT, 0777 ); // file opening is so vulnerable OAO~~
	if ( receiver_log==-1 ) {
		fprintf ( stderr, "send_msg: open receiver file\n" );
		return (-1);
	}

	write ( sender_log, text+msg_start, len-msg_start );
	write ( receiver_log, text+msg_start, len-msg_start-1 );

	char line[60]="\n\\\\\\END OF MESSAGE\n";
	write ( sender_log, line, strlen(line) );
	write ( receiver_log, line, strlen(line) );

	
	close ( sender_log );
	close ( receiver_log );

	char buf[30]="server received message\n";
	send_pkt ( socket_fd, MSG_SEND, strlen(buf), buf );

	fprintf ( stderr, "send_msg_to: end\n" );
}
int recv_msg_to ( int socket_fd, User *receiver, int len, char *text ) {
	char receive_from[MAX_USR_LENGTH];
	fprintf ( stderr, "len, %d; text: %s\n", len, text );
	for ( int i=0; i<len; ++i ) receive_from[i]=text[i]; receive_from[len]='\0';

	fprintf ( stderr, "recv_msg_to: sender \"%s\", receiver \"%s\"\n", receiver->usr, receive_from );

	char msg_file[MAX_USR_LENGTH*2];
	sprintf ( msg_file, "asset/%s/from_%s.new", receiver->usr, receive_from );
	fprintf ( stderr, "recv_msg_to: msg_file, %s\n", msg_file ); 
	int f=open ( msg_file, O_RDONLY, 0777 );
	if ( f<0 ) {
		fprintf ( stderr, "recv_msg_to: file does not exist OAO\n" );
		char buf[80]="you don't have any new messages.\nyou can use MSG_LOG to view logs.\n\0";
		send_pkt ( socket_fd, MSG_RECV, strlen(buf), buf );
		return (0);
	}

	memset ( bigbuf, 0, sizeof(bigbuf) );
	char buf[300]={};
	while ( read ( f, buf, sizeof(buf) ) ) strcat ( bigbuf, buf );
	strcat ( bigbuf, "\0" );
	
	close ( f );

	char receiver_file[300]={};
	sprintf ( receiver_file, "asset/%s/from_%s", receiver->usr, receive_from );
	int receiver_log=open ( receiver_file, O_APPEND | O_WRONLY | O_CREAT, 0777 );
	if ( receiver_log==-1 ) {
		fprintf ( stderr, "recv_msg_to: open\n" );
		return (-1);
	}
	write ( receiver_log, bigbuf, strlen(bigbuf) );
	
	unlink ( msg_file );
	close ( receiver_log );
	send_pkt ( socket_fd, MSG_RECV, strlen(bigbuf), bigbuf );
}
int view_log ( int socket_fd, User *receiver, int len, char *text ) {
// user already send which file to look at
// if exist read it all ad send it back, otherwise return error messages.
	char log_file[300]={};
	for ( int i=0; i<len; ++i ) log_file[i]=text[i]; log_file[len]='\0';
	
	char msg_file[300]={};
	sprintf ( msg_file, "asset/%s/%s", receiver->usr, log_file );
	fprintf ( stderr, "view_log: msg_file, %s\n", msg_file );
	
	int f=open (msg_file, O_RDONLY, 0777 );
	if ( f<0 ) {
		char buf[120]="there is no log between you two.\nmaybe it is still a new message\nyou can use MSG_RECV to view logs.\n";
		send_pkt ( socket_fd, MSG_LOG, strlen(buf), buf );
		return (0);
	}

	memset ( bigbuf, 0, sizeof (bigbuf) );
	char buf[300]={};
	//read ( f, buf, sizeof(buf) );
	//strcpy ( bigbuf, buf );
	while ( read ( f, buf, sizeof(buf) ) ) strcat ( bigbuf, buf );
	strcat ( bigbuf, "\0" );
	
	close ( f );
	send_pkt ( socket_fd, MSG_LOG, strlen(bigbuf), bigbuf );
}
int recv_file ( int socket_fd, User *receiver, int len, char *text ) { // server receives file
	char file[300]={};
	for ( int i=0; i<len; i++ ) file[i]=text[i]; file[len]='\0';
	char filename[300]={}; sprintf ( filename, "file/%s", file );
	fprintf ( stderr, "recv_file: %s\n", filename );
	sleep (1);
	Packet *pkt_ptr;
	if ( (pkt_ptr=recv_pkt ( socket_fd ))==NULL ) {
  		fprintf ( stdout, "recv_file: GG ~~~ QAQ\n" );
  		return (-1);
  	}
  	if ( pkt_ptr->type==FILE_FILE ) {
  		fprintf ( stderr, "recv_file: receive file packet, start write\n" );
  	}
  	if ( pkt_ptr->type==FILE_FILE_END ) {
  		fprintf ( stderr, "recv_file: this should be the last file\n" );
  	}
  	// write file
  	FILE *fp=fopen ( filename, "wb" );
	int byteswrite=fwrite ( pkt_ptr->text, sizeof(char), pkt_ptr->len, fp );
	if ( byteswrite<=0 ) {
		fprintf ( stdout, "recv_file: fail to write file\n" );
		fclose ( fp );
		return (-1);
	}
	fflush ( stdout );
	char buf[300]="received file, saved on server\n";
	if ( pkt_ptr->type==FILE_FILE_END ) {
		sleep (1);
		fprintf ( stderr, "recv_file: waving goodbye\n" );
		send_pkt ( socket_fd, FILE_NAME, strlen(buf), buf );
	}
	free_pkt (pkt_ptr);
	fclose ( fp );
	fflush ( stdout );
	fprintf ( stdout, "recv_file: write success\n" );
}
int send_file ( int socket_fd, User *sender, int len, char *text, char type ) { // server sends file
	char file[300]={};
	for ( int i=0; i<len; i++ ) file[i]=text[i]; file[len]='\0';
	char filename[300]={}; sprintf ( filename, "file/%s", file );
	fprintf ( stderr, "send_file: %s\n", filename );
	// send file
	FILE *fp=fopen ( filename, "rb" );
	memset ( bigbuf, 0, sizeof(bigbuf) );
	int bytesread=fread ( bigbuf, sizeof(char), sizeof(bigbuf), fp );
	if ( bytesread<=0 ) {
		fprintf ( stdout, "send_file: fail to read file\n" );
		fclose ( fp );
		return (-1);
	}
	fclose ( fp );
	fprintf ( stderr, "send_file: finish reading, file_size %d\n", bytesread ); fflush ( stdout );
	//sleep (1);
	send_pkt ( socket_fd, FILE_FILE, bytesread, bigbuf );
	fprintf ( stdout, "send_file: success\n" ); fflush ( stdout );
	char buf[300]="sent file, hope you get it\n";
	if ( type==FILE_DOWNLOAD_END ) {
		sleep (2);
		fprintf ( stderr, "send_file: waving goodbye\n" );
		send_pkt ( socket_fd, FILE_DOWNLOAD, strlen(buf), buf );
	}
}
/*------------------------------------------*/

/*------------ Cool features ---------------*/
int change_pwd ( int socket_fd, User *user, User_list *usr_list, int len, char *text ) {
	free ( user->pwd );
	user->pwd=(char *) malloc ( len+2 );
	for ( int i=0; i<len; ++i ) user->pwd[i]=text[i]; user->pwd[len]='\0';
	dump ( usr_list );
	send_pkt ( socket_fd, CHANGE_PWD, strlen(password_changed), password_changed );
}
int clear_log ( int socket_fd, User *user ) {
	char buf[300]="rm asset/";
	strcat ( buf, user->usr );
	strcat ( buf, "/*");
	system ( buf );
	send_pkt ( socket_fd, CLEAR_LOG, strlen(log_cleared), log_cleared );
}
/*------------------------------------------*/

