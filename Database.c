#include<stdio.h>
#include<stdlib.h>
#include"Database.h"

int index = 0;

int initUsersDatabase()
{
	int res = 0;
	res = createUsersTable();
	return res;
}

sqlite3 * openDatabase()
{
	sqlite3 *db;

	int res = sqlite3_open("users.db", &db);

	if (res!= SQLITE_OK)
	{

		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);

		return NULL;
	}
	return db;
}

int createUsersTable()
{
	sqlite3 * db = openDatabase();
	if (NULL != db)
	{
		char *sql_stmt = "SELECT * FROM Users";
		char *err_msg = 0;

		int res = sqlite3_exec(db, sql_stmt, 0, 0, &err_msg);

		if (res != SQLITE_OK )
		{
			printf("\n Users table not created!!!!!!!!!!! \n");
			printf("\n Creating table Users.........\n");
			sql_stmt = "CREATE TABLE IF NOT EXISTS Users(Id INTEGER PRIMARY KEY AUTOINCREMENT, User_ID INTEGER, Login_Status INTEGER, Port_number INTEGER, "
					" Session_With INTEGER);"
				"INSERT INTO Users (User_ID, Login_Status, Port_number, Session_with) VALUES(10, 0, -1, -1);"
				"INSERT INTO Users (User_ID, Login_Status, Port_number, Session_with) VALUES(20, 0, -1, -1);"
				"INSERT INTO Users (User_ID, Login_Status, Port_number, Session_with) VALUES(30, 0, -1, -1);"
				"INSERT INTO Users (User_ID, Login_Status, Port_number, Session_with) VALUES(40, 0, -1, -1);"
				"INSERT INTO Users (User_ID, Login_Status, Port_number, Session_with) VALUES(50, 0, -1, -1);"
				"INSERT INTO Users (User_ID, Login_Status, Port_number, Session_with) VALUES(60, 0, -1, -1);"
				"INSERT INTO Users (User_ID, Login_Status, Port_number, Session_with) VALUES(70, 0, -1, -1);"
				"INSERT INTO Users (User_ID, Login_Status, Port_number, Session_with) VALUES(80, 0, -1, -1);"
				"INSERT INTO Users (User_ID, Login_Status, Port_number, Session_with) VALUES(90, 0, -1, -1);"
				"INSERT INTO Users (User_ID, Login_Status, Port_number, Session_with) VALUES(100, 0, -1, -1);";
			res = sqlite3_exec(db, sql_stmt, 0, 0, &err_msg);
			if (res != SQLITE_OK)
			{
				fprintf(stderr, "Failed to create table Users\n");
				fprintf(stderr, "SQL error: %s\n", err_msg);
			}
			else
			{
				printf("Users table successfully created !!!!!!!!!!!");
			}

			sqlite3_free(err_msg);
			sqlite3_close(db);

			return 1;
		}
	}

	sqlite3_close(db);
	return 0;

}

void checkUserValid(ServerMessage* msg, int user)
{
	sqlite3 * db = openDatabase();
	if (NULL != db)
	{
		char* final = NULL;
		char *sql_stmt = "SELECT * FROM Users where User_ID = ";
		char userid[10] = "\0";

		sprintf(userid, "%d", user);
		char * colon = ";";

		final = (char*)malloc(strlen(sql_stmt) + strlen(userid) + strlen(colon) + 1 + 1 + 1 );
		memset(final, 0, sizeof(final));

		strcat(final, sql_stmt);
		strcat(final, userid);
		strcat(final, colon);

		char *err_msg = 0;

		int res = sqlite3_exec(db, final, checkUserValidCallback, msg, &err_msg);

		if (res != SQLITE_OK )
		{

			msg->responseType = InvalidOperation;
			fprintf(stderr, "Failed to select data\n");
			fprintf(stderr, "SQL error: %s\n", err_msg);

			sqlite3_free(err_msg);
		}
	}
	sqlite3_close(db);
}

int checkUserValidCallback(void *data, int argc, char **argv, char **colName)
{
	ServerMessage* msg = (ServerMessage*)data;
	if (atoi(argv[2]) == 0)
	{
		msg->responseType = Okay;
	}
    return 0;
}

int getOnlineUsers(ServerMessage* smsg)
{
	sqlite3 *db = openDatabase();
	if (NULL != db)
	{
		char *errMsg = 0;
		int res;
		char* final = NULL;
		char * sql_stmt = "SELECT * FROM Users where Login_Status = 1";

		/* Execute SQL statement */
		res = sqlite3_exec(db, sql_stmt, onlineUserInfoCallback, smsg, &errMsg);
		if( res != SQLITE_OK )
		{
			smsg->responseType = InvalidOperation;
			sqlite3_close(db);
			return 1;
		}

	}
	sqlite3_close(db);

	return 0;
}

int onlineUserInfoCallback(void *data, int argc, char **argv, char **colName)
{
	ServerMessage* msg = (ServerMessage*)data;
	if (atoi(argv[1]) != msg->userID)
	{
		msg->responseType = UsersOnline;
		msg->usersLogged[index] = atoi(argv[1]);
		index ++;
	}
    return 0;
}

int updateUsersLoginStatus(ServerMessage * smsg, int userID, char * colName, int value)
{
	sqlite3 *db = openDatabase();
	if (NULL != db)
	{
		char *errMsg = 0;
		int res;
		char* final = NULL;
		char * sql_stmt = "UPDATE Users set ";
		char stat[2]  = "\0";

		sprintf(stat, "%d", value);

		char * where = " where User_ID = ";
		char userid[10] = "\0";
		sprintf(userid, "%d", userID);
		char * colon = ";";
		char * eql = " = ";
		final = (char*)malloc(strlen(sql_stmt) + 1 + strlen(colName) + 1 + strlen(eql) + 1
				+ strlen(stat) + 1
				+ strlen(where) + 1 + strlen(userid) + 1
				+ strlen(colon) + 1);
		memset(final, 0, sizeof(final));

		strcat(final, sql_stmt);
		strcat(final, colName);
		strcat(final, eql);
		strcat(final, stat);
		strcat(final, where);
		strcat(final, userid);
		strcat(final, colon);

		/* Execute SQL statement */
		res = sqlite3_exec(db, final, 0, smsg, &errMsg);
		if( res != SQLITE_OK )
		{
			smsg->responseType = InvalidOperation;
			sqlite3_close(db);
			return 1;
		}
		else
		{
			smsg->responseType = Okay;
		}

	}
	sqlite3_close(db);

	return 0;
}

void getTalkRequestInfo(ServerMessage* msg)
{
	sqlite3 * db = openDatabase();
	if (NULL != db)
	{
		char* final = NULL;
		char *sql_stmt = "SELECT * FROM Users where Login_Status == 1 and Session_with == -1 and User_ID = ";
		char userid[10] = "\0";

		sprintf(userid, "%d", msg->session_with);
		char * colon = ";";

		final = (char*)malloc(strlen(sql_stmt) + strlen(userid) + strlen(colon) + 1 + 1 + 1 );
		memset(final, 0, sizeof(final));

		strcat(final, sql_stmt);
		strcat(final, userid);
		strcat(final, colon);

		char *err_msg = 0;
		int res = sqlite3_exec(db, final, getTalkRequestInfoCallback, msg, &err_msg);

		if (res != SQLITE_OK )
		{

			msg->responseType = InvalidOperation;
			fprintf(stderr, "Failed to select data\n");
			fprintf(stderr, "SQL error: %s\n", err_msg);

			sqlite3_free(err_msg);
		}
	}
	sqlite3_close(db);
}

int getTalkRequestInfoCallback(void *data, int argc, char **argv, char **colName)
{
	ServerMessage* msg = (ServerMessage*)data;
	msg->responseType = Okay;
	msg->talkto_port_number = (unsigned short)atoi(argv[3]);
    return 0;
}
