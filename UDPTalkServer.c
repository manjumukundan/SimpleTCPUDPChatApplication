#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include "Database.h"

int main (int argc, char* argv[])
{
	int sock; // socket descriptor
	struct sockaddr_in *udpServerAddr; // Local address
	struct sockaddr_in *udpClientAddr; // Client address
	unsigned short serverPort; // server port number
	int clientAddrLen;

	// check for correct no. of parameters
	if (argc != 2)
	{
		perror("\nINVALID NO. OF ARGUMENTS \n");
		exit(1);
	}

	serverPort = atoi(argv[1]);

	/* Create socket for sending and receiving datagram*/
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		perror("\nSOCKET CREATION FAILED !!!!!!!!!! \n");
		exit(1);
	}

	udpServerAddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	/* fill the server address structure with zeros*/
	memset(udpServerAddr, 0, sizeof(*udpServerAddr));
	/*Creat the server address structure*/
	udpServerAddr->sin_family = AF_INET;
	udpServerAddr->sin_addr.s_addr = htonl(INADDR_ANY); // any incoming interface
	udpServerAddr->sin_port = htons(serverPort); // Local Port

	/* bind to the local address */
	if (bind(sock, (struct sockaddr *)udpServerAddr, sizeof(*udpServerAddr) ) < 0)
	{
		perror("\nSOCKET BIND FAILURE !!!!!!!!!! \n");
		exit(1);
	}

	printf("\n************ Talk Server Running *************** \n");

	initUsersDatabase();

	udpClientAddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	memset(udpClientAddr, 0, sizeof(*udpClientAddr));

	ClientMessage * clientMsg = malloc(sizeof(ClientMessage));
	memset(clientMsg, 0, sizeof(*clientMsg));


	while (1)
	{
		printf("\nWaiting for Clients to connect ....................... \n");

		clientAddrLen = sizeof(struct sockaddr_in *);
		if (recvfrom(sock, clientMsg, sizeof(*clientMsg), 0, (struct sockaddr *)udpClientAddr,
			&clientAddrLen) < 0)
		{
			perror("RECEIVE MESSAGE FROM CLIENT FAILED !!!!!!!!! \n");
			exit(1);
		}

		ServerMessage * serverMsg = NULL;
		serverMsg = malloc(sizeof(ServerMessage));
		memset(serverMsg, 0, sizeof(*serverMsg));

		// Handle the client messages.
		switch (clientMsg->requestType)
		{

			case Login:
				serverMsg->userID = clientMsg->userID;
				serverMsg->requestID = clientMsg->requestID;
				serverMsg->responseType = InvalidLogin;
				serverMsg->port_number = ntohs(udpClientAddr->sin_port);
				checkUserValid(serverMsg, serverMsg->userID);
				if (serverMsg->responseType == Okay)
				{
					serverMsg->loginStatus = 1;
					updateUsersLoginStatus(serverMsg, serverMsg->userID, "Login_Status", serverMsg->loginStatus);
					updateUsersLoginStatus(serverMsg, serverMsg->userID, "Port_number", serverMsg->port_number);
				}
				break;

			case Online:
				serverMsg->userID = clientMsg->userID;
				serverMsg->requestID = clientMsg->requestID;
				serverMsg->responseType = InvalidOperation;
				getOnlineUsers(serverMsg);
				break;

			case Talk:
				serverMsg->userID = clientMsg->userID;
				serverMsg->requestID = clientMsg->requestID;
				serverMsg->port_number = ntohs(udpClientAddr->sin_port);
				serverMsg->session_with = clientMsg->talkTo;

				serverMsg->responseType = InvalidOperation;
				// if user to talk to is valid, set the sender and receiver user details in db.
				getTalkRequestInfo(serverMsg);

				if (serverMsg->responseType == Okay)
				{
					//sender
					updateUsersLoginStatus(serverMsg, serverMsg->userID, "Session_with", clientMsg->talkTo);

					//recevier
					updateUsersLoginStatus(serverMsg, clientMsg->talkTo, "Session_with", serverMsg->userID);
				}
				break;
//
//			case Wait:
//
//				break;

			case Quit:
				serverMsg->userID = clientMsg->userID;
				serverMsg->requestID = clientMsg->requestID;
				int talkingTo = clientMsg->talkTo;
				serverMsg->session_with = -1;
				updateUsersLoginStatus(serverMsg, serverMsg->userID, "Session_with", serverMsg->session_with);
				updateUsersLoginStatus(serverMsg, talkingTo, "Session_with", -1);
				break;

			case Logout:
				serverMsg->userID = clientMsg->userID;
				serverMsg->requestID = clientMsg->requestID;
				serverMsg->loginStatus = 0;
				serverMsg->port_number = -1;
				serverMsg->session_with = -1;
				updateUsersLoginStatus(serverMsg, serverMsg->userID, "Login_Status", serverMsg->loginStatus);
				updateUsersLoginStatus(serverMsg, serverMsg->userID, "Port_number", serverMsg->port_number - (UINT16_MAX + 1));
				updateUsersLoginStatus(serverMsg, serverMsg->userID, "Session_with", serverMsg->session_with);

				// check if userid logged out is used by any other sessions.

				break;

			default:
				break;

		}

		if (sendto(sock, serverMsg, sizeof(*serverMsg), 0, (struct sockaddr *)udpClientAddr,
						sizeof(*udpClientAddr)) < 0)
		{
			perror("SEND DATA FAILED !!!!!!!! \n");
		}
	}

}
