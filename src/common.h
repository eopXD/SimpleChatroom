// by Yueh-Ting Chen (eopXD)
// 2018 Computer Network Final Project -- Chatroom

/*
This file is of protocol between client and server.
Definition of ...
	1. Restrictions on connections
	2. Message types
	3. Structures (packet and user)
	4. External functions
*/

/*-------------Restrictions-----------------*/
#define MAX_USER 			1320
#define MAX_CLIENT			128
#define MAX_USR_LENGTH 		30
#define MAX_PWD_LENGTH 		256
#define MAX_TXT_LENGTH 		1024*1024 //1MB		
/*------------------------------------------*/

/*--------------Message types---------------*/
// EOF
#define BYE_BYE				0

// Registration
#define REGISTER			10
#define REGISTER_GOOD		11
#define REGISTER_FAIL		12

// Login
#define LOGIN				20
#define LOGIN_GOOD			21
#define LOGIN_FAIL			22

// Online/Offline
#define ONLINE_LIST			30
#define FRIEND_LIST			31
#define FRIEND_ADD			32
#define BLACK_LIST			33
#define BLACK_ADD			34

// Message I/O
#define MSG_SEND			40
#define MSG_RECV			41
#define MSG_LOG				42

// File I/O
#define FILE_NAME			50
#define FILE_FILE			51
#define FILE_FILE_END		52
#define FILE_DOWNLOAD		53
#define FILE_DOWNLOAD_END	54

// Features
#define CHANGE_PWD			60
#define CLEAR_LOG			61
/*------------------------------------------*/

/*-----------------Stucture-----------------*/
typedef struct _packet {
	// packet type
	char type;
	// packet length
	long len;
	// packet text
	char *text;
} Packet;
typedef struct _user {
	char *usr;
	char *pwd;	
} User;
/*------------------------------------------*/

/*-----------External functions-------------*/
// Server linkage
extern int gogo_server ( int port );
extern int hook_server ( char *host, int port );

// Packet I/O
extern Packet*	recv_pkt ( int sd );
extern int 		send_pkt ( int sd, char type, long len, char *buf );
extern void 	free_pkt ( Packet *pkt );
/*------------------------------------------*/

/*--------------User operations-------------*/
extern User* 	user_parse ( int len, char *buf );
extern void 	free_usr ( User *usr );
/*------------------------------------------*/
