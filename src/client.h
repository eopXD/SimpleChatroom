// by Yueh-Ting Chen (eopXD)
// 2018 Computer Network Final Project -- Chatroom

/*
This file is of functions needed by the client.
Client functions are commands sent to the server for response.
Sent packets are all in certain format, check them out for more.

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

#include "common.h"

char bigbuf[MAX_TXT_LENGTH];

/*-------------Static Dialogues-------------*/
static char register_good[15]="register good\n";
static char register_fail[15]="register fail\n";

static char login_good[15]="login good\n";
static char login_fail[15]="login fail\n";
/*------------------------------------------*/


/*------------Everyday quotes---------------*/
static char *quotes[20]={
						"I used to think I was indecisive, but now I'm not too sure.",
						"Doing nothing is hard, you never know when you're done.",
						"If two wrongs don't make a right, try three.",
						"I am not lazy, I am on energy saving mode.",
						"Life is short, smile while you still have teeth.",
						"A balanced diet means a cupcake in each hand.",
						"Maybe you should eat some makeup so you can be pretty on the inside too.",
						"I'm not shy, I'm holding back my awesomeness so I don't intimidate you.",
						"Sorry for the mean, awful, accurate things I said.",
						"I’m sorry, if you were right, I’d agree with you.",
						"Your life can't fall apart if you never had it together!",
						"People say nothing is impossible, but I do nothing every day.",
						"A bank is a place that will lend you money if you can prove that you don't need it.",
						"If you think nobody cares if you're alive, try missing a couple of payments.",
						"Always remember that you're unique. Just like everyone else.",
						"The answer you're looking for is inside of you, but it's wrong",
						"One advantage of talking to yourself is that you know at least somebody's listening.",
						"The elevator to success is out of order. You’ll have to use the stairs.",
						"An apple a day keeps anyone away if you throw it hard enough.",
						"The more you weight the harder you are to kidnap. Stay safe, eat cake."
};
/*------------------------------------------*/


/*--------------Login/Logout--------------*/
User* send_register ( int socket_fd ) {
	char usr[MAX_USR_LENGTH],pwd[MAX_PWD_LENGTH];
	fprintf ( stdout, "\n******************************* Start register **********************************************\n" );
	fprintf ( stdout, "register user name: " );
	fscanf ( stdin, "%s", usr );
	fprintf ( stdout, "register password: " );
	fscanf ( stdin, "%s", pwd );
	
	User *usr_ptr;
	usr_ptr= (User *) calloc ( 1, sizeof(User) );
	usr_ptr->usr=(char *) malloc ( strlen(usr)+2 );
	usr_ptr->pwd=(char *) malloc ( strlen(pwd)+2 );
	strcpy ( usr_ptr->usr, usr );
	strcpy ( usr_ptr->pwd, pwd );

	char buf[300]={};
	sprintf ( buf, "usr:%.3ld:%s:pwd:%.3ld:%s:", strlen(usr), usr, strlen(pwd), pwd );
	
	fprintf ( stderr, "send_register: %s\n", buf );
	fprintf ( stdout, "\n******************************* End register **************************************************\n" );
	// format: "usr:[USR_LENGTH]:[USR]:pwd:[PWD_LENGTH]:[PWD]:"
	send_pkt ( socket_fd, REGISTER, strlen(buf), buf );
	return usr_ptr;
}
User* send_login ( int socket_fd ) {
	char usr[MAX_USR_LENGTH],pwd[MAX_PWD_LENGTH];
	fprintf ( stdout, "\n******************************* Start login ***************************************************\n" );
	fprintf ( stdout, "login user name: " );
	fscanf ( stdin, "%s", usr );
	fprintf ( stdout, "login password: " );
	fscanf ( stdin, "%s", pwd );
	
	User *usr_ptr;
	usr_ptr= (User *) calloc ( 1, sizeof(User) );
	usr_ptr->usr=(char *) malloc ( strlen(usr)+2 );
	usr_ptr->pwd=(char *) malloc ( strlen(pwd)+2 );
	strcpy ( usr_ptr->usr, usr );
	strcpy ( usr_ptr->pwd, pwd );

	char buf[300]={};
	sprintf ( buf, "usr:%.3ld:%s:pwd:%.3ld:%s:", strlen(usr), usr, strlen(pwd), pwd ); // format string is so good to use!!!
	
	fprintf ( stderr, "send_login: %s\n", buf );
	fprintf ( stdout, "\n******************************* End login *****************************************************\n" );
	// format: "usr:[USR_LENGTH]:[USR]:pwd:[PWD_LENGTH]:[PWD]:"
	send_pkt ( socket_fd, LOGIN, strlen(buf), buf );
	return (usr_ptr);
}
void send_logout ( int socket_fd ) {
	char buf[5]="bye\n";
	send_pkt ( socket_fd, BYE_BYE, strlen(buf) , buf );
}
/*----------------------------------------*/
/*--------------Online query--------------*/
void send_who_online ( int socket_fd ) {
	char buf[20]="who is online???\n";
	send_pkt ( socket_fd, ONLINE_LIST, strlen(buf), buf );
}
/*----------------------------------------*/
/*-------------Cool features -------------*/
void get_weather () {
	system ( "clear" );
	system ( "curl wttr.in/Taipei" );

	fprintf ( stdout, "Press any key to continue;" );
	fflush ( stdout ) ;
	getchar(); getchar();
}
void get_funny_quote () {
	system ( "clear" );
	char buf[500]="cowsay \"";
	srand ( time(NULL) );
	strcat ( buf, quotes[rand()%20] );
	strcat ( buf, "\"" );
	system ( buf );
	fprintf ( stdout, "Press any key to continue;" );
	fflush ( stdout ) ;
	getchar(); getchar();
}
int change_pwd ( int socket_fd ) {
	char pwd[MAX_USR_LENGTH]={};
	fprintf ( stdout, "What is your new password? " );
	fscanf ( stdin, "%s", pwd );
	send_pkt ( socket_fd, CHANGE_PWD, strlen(pwd), pwd );
} 
int log_clear ( int socket_fd ) {
	char buf[300]="I want to clear log";
	send_pkt ( socket_fd, CLEAR_LOG, strlen(buf), buf);
}
/*----------------------------------------*/

/*----------Message operations------------*/
int send_msg ( int socket_fd ) {
	char usr[MAX_USR_LENGTH]={};
	fprintf ( stdout, "\nWho do you want to send to? " );
	fscanf ( stdin, "%s", usr );
	fprintf ( stdout, "Now enter the message you want to send, press \n(ctrl+d) to send it.\n\n" );
	memset ( bigbuf, 0, sizeof(bigbuf) );
	strcpy ( bigbuf, usr );
	strcat ( bigbuf, "\n" );

	char buf[300]={};
	while ( gets(buf)!=NULL ) {
		strcat ( buf, "\n" );
		strcat ( bigbuf, buf );
	}
	strcat ( bigbuf, "\n" );
	// format: [RECV_USR]\n[MESSAGE ..]
	send_pkt ( socket_fd, MSG_SEND, strlen(bigbuf), bigbuf );
}
int recv_msg ( int socket_fd ) {
	char usr[MAX_USR_LENGTH]={};
	fprintf ( stdout, "Who do you want to receive from? " );
	fscanf ( stdin, "%s", usr );
	usr[strlen(usr)]='\0';
	fprintf ( stderr, "recv_msg: send \"%s\"\n", usr );
	
	// format: [RECV_FROM_USR]
	send_pkt ( socket_fd, MSG_RECV, strlen(usr), usr );
}

int msg_log ( int socket_fd ) {
	char usr[MAX_USR_LENGTH]={},op[10]={};
	fprintf ( stdout, "With whom's conversation do you want to see? " );
	fscanf ( stdin, "%s", usr );
	fprintf ( stdout, "Message you send \"to\", or you receive \"from\"?  [to/from] " );
	fscanf ( stdin, "%s", op);
	char buf[300]={};
	strcpy ( buf, op );
	strcat ( buf, "_" );
	strcat ( buf, usr );
	strcat ( buf, "\0" );
//	fprintf ( stderr, "msg_log: %s\n", buf );

	// format: "from_[USR]" or "to_[USR]" 
	send_pkt ( socket_fd, MSG_LOG, strlen(buf), buf );
}
/*----------------------------------------*/

/*------------File transfer---------------*/
int send_file ( int socket_fd ) {
	fprintf ( stdout, "How many files do you want to upload?[1~20] " ); fflush ( stdout );
	int file_cnt; fscanf ( stdin, "%d", &file_cnt );

	fprintf ( stdout, "Insert the file names, seperate them with spaces...\n> " ); fflush ( stdout );
	char filenames[20][100]; 
	for ( int i=0; i<file_cnt; ++i ) {
		fscanf ( stdin, "%s", filenames[i] );
	}

	fprintf ( stdout, "\nChecking if all files sendable...\n" ); fflush ( stdout );
	for ( int i=0; i<file_cnt; ++i ) {
		fprintf ( stdout, "check %s: ", filenames[i] );
		int f=open ( filenames[i], O_RDONLY, 0777 );
		if ( f<0 ) {
			fprintf ( stdout, "fail\nhalt file transfer procedure\n" );
			return (-1);
		}
		else fprintf ( stdout, "ok\n" );
		close ( f );
	}
	fflush ( stdout );

// all files sendable, start sending files, halt when something fails
	fprintf ( stdout, "\n******************************* Start upload ************************************************\n" );
	//fprintf ( stdout, "(sleeping 1 sec/file for server to react)\n" );
	for ( int i=0; i<file_cnt; ++i ) {
		fprintf ( stdout , "send %s: ", filenames[i] );
		// send filename
		send_pkt ( socket_fd, FILE_NAME, strlen(filenames[i]), filenames[i] );
		fflush ( stdout );
		sleep (1);
		// send file
		FILE *fp=fopen ( filenames[i], "rb" );
		memset ( bigbuf, 0, sizeof(bigbuf) );
		int bytesread=fread ( bigbuf, sizeof(char), sizeof(bigbuf), fp );
		if ( bytesread<=0 ) {
			fprintf ( stdout, "fail to read file\n" );
			fclose ( fp );
			return (-1);
		}
		if ( i+1==file_cnt ) {
			//fprintf ( stderr, "send_file: sending last file\n" );
			send_pkt ( socket_fd, FILE_FILE_END, bytesread, bigbuf );
		}
		else send_pkt ( socket_fd, FILE_FILE, bytesread, bigbuf );
		fclose ( fp );
		fprintf ( stdout, "send success\n" );
	}
	fprintf ( stdout, "\n******************************* End upload **************************************************\n" );
	
}
int recv_file ( int socket_fd ) {
	fprintf ( stdout, "how many files do you want to download?[1~20] " ); fflush ( stdout );
	int file_cnt; fscanf ( stdin, "%d", &file_cnt );

	fprintf ( stdout, "insert the file names, seperate them with spaces...\n> " ); fflush ( stdout );
	char filenames[20][100]; 
	for ( int i=0; i<file_cnt; ++i ) {
		fscanf ( stdin, "%s", filenames[i] );
	}

// request download to server, halt when something fails
	fprintf ( stdout, "\n******************************* Start download **********************************************\n" );
	//fprintf ( stdout, "(sleeping 1 sec/file for server to react)\n" );
	for ( int i=0; i<file_cnt; ++i ) {
		fprintf ( stdout , "download %s: ", filenames[i] ); fflush ( stdout );
		// send filename
		if ( i+1==file_cnt ) send_pkt ( socket_fd, FILE_DOWNLOAD_END, strlen(filenames[i]), filenames[i] );
		else send_pkt ( socket_fd, FILE_DOWNLOAD, strlen(filenames[i]), filenames[i] );
		// receive file and write with filename
		sleep (1);
		Packet *pkt_ptr;
		if ( (pkt_ptr=recv_pkt ( socket_fd ))==NULL ) {
			fprintf ( stdout, "fail to receive packet\n" );
			return (-1);
		}
		// write file
		FILE *fp=fopen ( filenames[i], "wb" );
		int byteswrite=fwrite ( pkt_ptr->text, sizeof(char), pkt_ptr->len, fp );
		if ( byteswrite<=0 ) {
			fprintf ( stdout, "fail to write file\n" );
			fclose ( fp );
			return (-1);
		}
		free_pkt (pkt_ptr);
		fclose ( fp );
		fprintf ( stdout, "receive success\n" );
	}
	fprintf ( stdout, "\n******************************* End download ************************************************\n" );
}
/*----------------------------------------*/
