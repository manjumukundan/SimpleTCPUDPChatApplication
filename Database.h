#include<stdio.h>
#include<stdlib.h>
#include<sqlite3.h>
#include<stdbool.h>

typedef struct
{
	unsigned int requestID; /* unique client id */
	unsigned int userID; /* unique user identifier*/
	int usersLogged[10]; /* user ids of users logged in */
	enum ResponseType{UsersOnline, Notification, InvalidLogin, Okay, InvalidOperation, InSession}responseType;
	int loginStatus;
	unsigned short port_number;
	unsigned short talkto_port_number;
	int session_with;
} ServerMessage;

typedef struct
{
	unsigned int requestID; /* unique client identifier*/
	unsigned int userID;/*unique user id*/
	enum RequestType{Login, Online, Talk, Wait, Quit, Logout} requestType;
	int talkTo;
} ClientMessage;

int initUsersDatabase();

sqlite3 * openDatabase();

int createUsersTable();

int getOnlineUsers(ServerMessage* smsg);

int onlineUserInfoCallback(void *data, int argc, char **argv, char **colName);

void checkUserValid(ServerMessage* msg, int user);

int updateUsersLoginStatus(ServerMessage * smsg, int userID, char * colName, int value);

int checkUserValidCallback(void *data, int argc, char **argv, char **colName);

void getTalkRequestInfo(ServerMessage* msg);

int getTalkRequestInfoCallback(void *data, int argc, char **argv, char **colName);
