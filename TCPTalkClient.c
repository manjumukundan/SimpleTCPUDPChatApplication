#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<stdbool.h>
#include<pthread.h>

#define MAXPENDING 5    /* Maximum outstanding connection requests */

typedef struct
{
	unsigned int requestID; /* unique client id */
	unsigned int userID; /* unique user identifier*/
	int usersLogged[10]; /* user ids of users logged in */
	enum ResponseType{UsersOnline, Notification, InvalidLogin, Okay, InSession}responseType;
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

struct sockaddr_in *udpServerAddr; // server address details - ip and port number
char *serverIP; // server IP address
unsigned short portNumber;
pthread_mutex_t lock;

int sendMessageToServer(int sock, ClientMessage * msg);
int receiveDataFromServer(int sock, ServerMessage* serverMsg);
void * receiveMessageCallback(void * socket);
int handleTCPTalkOperation();
int handleTCPWaitOperation();
int sendMessage(int newSock);
int * receiveMessage(int newSock);
int * receiveNotification(int newSock);
void * receiveNotificationCallback(void * socket);

int main(int argc, char *argv[])
{
	int sock; // socket descriptor


	unsigned short serverPort; // server port number
	int userId = 0;

	if (argc < 2 || argc > 3)
	{
		perror("INVALID NO. OF ARGUMENTS");
		exit(1);
	}

	serverIP = argv[1];
	serverPort = atoi(argv[2]);
	srand(time(NULL)); // set up the seed for the random generator

	/* allocate pointer for udp server addr structure*/
	udpServerAddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	/*  fill the structure with zeros */
	memset(udpServerAddr, 0, sizeof(*udpServerAddr));
	/* construct the server address structure. */


	udpServerAddr->sin_family = AF_INET;
	udpServerAddr->sin_addr.s_addr = inet_addr(serverIP);
	udpServerAddr->sin_port = htons(serverPort);

	/* Create a UDP socket to connect*/
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP )) < 0)
	{
		perror("\nSOCKET CREATION FAILED !!!!!!!!!!!!!\n");
		exit(1);
	}


	printf("\n************ Talk Client Running *************** \n");

	/* Allocate for the client message to be sent*/
	ClientMessage *clientMsg = (ClientMessage*)malloc(sizeof(ClientMessage));
	memset(clientMsg, 0, sizeof(*clientMsg));

 	// Create the structure for receiving the server response
	ServerMessage* serverMsg = (ServerMessage *)malloc(sizeof(ServerMessage));
	memset(serverMsg, 0, sizeof(*serverMsg));

	do
	{
		printf("\n Enter a Valid User id to login to Talk Server\n");
		printf("\n User ID: ");
		scanf("%d", &userId);
		clientMsg->userID = userId;
		clientMsg->requestID = rand() % 100 + 1;
		clientMsg->requestType = Login;

		/* Send message to login to server*/
		sendMessageToServer(sock, clientMsg);

		/*wait for  receive response from server*/
		receiveDataFromServer(sock, serverMsg);

		if (serverMsg->responseType != Okay)
		{
			printf ("\n Invalid Login...........\n");
		}

	}
	while (serverMsg->responseType != Okay);

	portNumber = serverMsg->port_number;

	printf("\n Logged in Successfully \n");

	// find users online to talk to....................
	clientMsg->requestID = rand() % 100 + 1;
	clientMsg->requestType = Online;
	/* Send message to login to server*/
	sendMessageToServer(sock, clientMsg);

	/*wait for  receive response from server*/
	receiveDataFromServer(sock, serverMsg);

	if (serverMsg->responseType == UsersOnline)
	{
		int i,j = 0;
		int len = sizeof(serverMsg->usersLogged)/sizeof(serverMsg->usersLogged[0]);
		for (i = 0; i < len; i ++)
		{
			if (serverMsg->usersLogged[i] > 0)
			{
				printf("\n %d. User ID: %d", j + 1, serverMsg->usersLogged[i]);
				j++;
			}
		}
		printf("\n");
	}
	else
	{
		printf ("\n No users Online. Please wait...........\n");
	}

	int ret;

	while(1)
	{
		int option;
		printf("\n Please select following options to continue:  \n");
		printf(" 1) Talk\n");
		printf(" 2) Wait\n");
		printf(" 3) Logout\n");

		do
		{

			printf("\n Enter a valid a option: ");
			scanf("%d", &option);
		}
		while (option < 1 && option > 3);

		switch (option)
		{

			case 1: // Talk
				clientMsg->userID = userId;
				clientMsg->requestID = rand() % 100 + 1;
				clientMsg->requestType = Talk;
				printf("\n Enter the userId of user to talk to: ");
				int talkto;
				scanf("%d", &talkto);
				clientMsg->talkTo = talkto;
				/* Send message to login to server*/
				sendMessageToServer(sock, clientMsg);

				/*wait for  receive response from server*/
				receiveDataFromServer(sock, serverMsg);
				if (serverMsg->responseType == Okay)
				{
					printf("\n Talk request granted.\n");
					printf("\n Address to talk to : %d\n", serverMsg->talkto_port_number);
				}
				else
				{
					printf("\n Requested user is not Logged in.\n");
					printf("\n OR\n");
					printf("\n Requested user is already in another talk.\n");
				}

				// setup tcp connection with a client to talk to
				ret = handleTCPTalkOperation(serverMsg);
				if (ret)
				{
					clientMsg->userID = userId;
					clientMsg->requestID = rand() % 100 + 1;
					clientMsg->requestType = Quit;
					/*send query message to server*/
					sendMessageToServer(sock, clientMsg);
					/*receive response from server*/
					receiveDataFromServer(sock, serverMsg);
					if (serverMsg->responseType == Okay)
					{
						printf("\n Talk Quit......\n");
					}
				}
				break;

			case 2: // Wait
				ret = handleTCPWaitOperation();
				if (ret)
				{
					clientMsg->userID = userId;
					clientMsg->requestID = rand() % 100 + 1;
					clientMsg->requestType = Quit;
					/*send query message to server*/
					sendMessageToServer(sock, clientMsg);
					/*receive response from server*/
					receiveDataFromServer(sock, serverMsg);
					if (serverMsg->responseType == Okay)
					{
						printf("\n Talk Quit......\n");
					}
				}
				break;

			case 3: //Logout
				clientMsg->requestID = rand() % 100 + 1;
				clientMsg->requestType = Logout;
				/*send query message to server*/
				sendMessageToServer(sock, clientMsg);
				/*receive response from server*/
				receiveDataFromServer(sock, serverMsg);

				if (serverMsg->requestID == clientMsg->requestID
						&& serverMsg->responseType == Okay)
				{
					printf("\n LOGGING OUT. THANK YOU\n");
					exit(1);
				}
				else
				{
					printf("\n INVALID RESPONSE FROM SERVER !!!\n");
				}
				break;

			default:
				break;
		}
	}

}

int sendMessageToServer(int sock, ClientMessage * msg)
{
	int sendLen = -1;

	// send message to server
	sendLen = sendto(sock, msg, sizeof(*msg), 0, (struct sockaddr *)udpServerAddr,
		sizeof(*udpServerAddr));

	if (sendLen < 0)
	{
		perror("\n SENDING DATA FAILED !!!!!!!!!!!!!\n");
		exit(1);
	}

	return sendLen;
}

int receiveDataFromServer(int sock, ServerMessage* serverMsg)
{
	int responseSize = -1;
	int serverAddrLen = -1;
	struct sockaddr_in *responseAddr; // response source address

    /*allocate for the response address structure */
	responseAddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	memset(responseAddr, 0, sizeof(*responseAddr));

	/* Loop recvfrom till a valid data received */
	while (1)
	{
		serverAddrLen = sizeof(struct sockadr_in *);
		responseSize = recvfrom(sock, serverMsg, sizeof(*serverMsg), 0, (struct sockaddr *)responseAddr,
					&serverAddrLen);
		if (responseSize > 0)
		{
			break;
		}
		else
		{
			perror("\n RECEIVING RESPONSE MESSAGE FAILED !!!!!!!!!\n");
			exit(1);
		}
	}

	// check whether the response is from valid source
	if (responseAddr->sin_addr.s_addr != udpServerAddr->sin_addr.s_addr)
	{
		perror("\n RESPONSE MESSAGE IS FROM AN UNKNOWN SOURCE !!!!!!!!!\n");
		exit(1);
	}

	return responseSize;
}


int handleTCPTalkOperation(ServerMessage * serverMsg)
{
	int sock, ret = 0;;
	struct sockaddr_in *udpClient2Addr; // client2 address details - ip and port number

	/* Create a UDP socket to connect*/
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("\n SOCKET CREATION FAILED !!!!!!!!!!!!!\n");
		exit(1);
	}

	/* allocate pointer for udp server addr structure*/
	udpClient2Addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	/*  fill the structure with zeros */
	memset(udpClient2Addr, 0, sizeof(*udpClient2Addr));
	/* construct the server address structure. */
	udpClient2Addr->sin_family = AF_INET;
	udpClient2Addr->sin_addr.s_addr = inet_addr(serverIP);
	udpClient2Addr->sin_port = htons(serverMsg->talkto_port_number);

	/* Establish the connection to the echo server */
	if (connect(sock, (struct sockaddr *)udpClient2Addr, sizeof(*udpClient2Addr)) < 0)
	{
		perror("\n Connection error !!!!!!!!!!!!!\n");
		exit(1);

	}
	printf("\n Connection Success....\n");

	while (1)
	{
		if (sendMessage(sock) == 1)
		{
			perror("\n Quitting talk........Bye\n");
			ret = 1;
			break;
		}

		if (receiveMessage(sock) == 1)
		{
			perror("\n Quitting talk........Bye\n");
			ret = 1;
			break;
		}
	}

	close(sock);

	return ret;
}


int handleTCPWaitOperation()
{
	int sock, newSock, ret = 0;
	struct sockaddr_in * tcpAddr; // Local address
	struct sockaddr_in * userAddr; /* Users address */

	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("\n SOCKET CREATION FAILED !!!!!!!!!!!!!\n");
		exit(1);
	}

	tcpAddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	/* fill the server address structure with zeros*/
	memset(tcpAddr, 0, sizeof(*tcpAddr));
	/*Creat the server address structure*/
	tcpAddr->sin_family = AF_INET;
	tcpAddr->sin_addr.s_addr = htonl(INADDR_ANY); // any incoming interface
	tcpAddr->sin_port = htons(portNumber); // Local Port

	if (bind(sock, (struct sockaddr *)tcpAddr, sizeof(*tcpAddr)) < 0)
	{
		perror("\n Bind FAILED !!!!!!!!!!!!!\n");
	    exit(1);
	}
	printf("\n Bind Success....\n");
	if (listen(sock, MAXPENDING) < 0)
	{
		perror("\n Listen FAILED !!!!!!!!!!!!!\n");
	    exit(1);
	}
	printf("\n Listening....\n");
	/* Set the size of the in-out parameter */
	int userLen = sizeof(userAddr);

	/* Wait for a client to connect */

	if ((newSock = accept(sock, (struct sockaddr *) &userAddr,
									   &userLen)) < 0)
	{
		perror("\n Accept FAILED !!!!!!!!!!!!!\n");
		exit(1);
	}

	while (1)
	{
		if (receiveMessage(newSock) == 1)
		{
			perror("\n Quitting talk........Bye\n");
			ret = 1;
			break;
		}

		if (sendMessage(newSock) == 1)
		{
			perror("\n Quitting talk........Bye\n");
			ret = 1;
			break;
		}
	}


	close(newSock);
	close(sock);

	return ret;
}

int sendMessage(int sock)
{
	int ret = 0;
	int bufferSize = 2000;
	int sendMsgSize;
	char buffer[bufferSize];

	printf ("\n Send: ");
	scanf("%s", buffer);
	if (send(sock, buffer, bufferSize, 0) < 0)
	{
		printf("\n Send error......\n");
		exit(1);
	}

	if (strcmp(buffer, "quit") == 0)
	{
		ret = 1;
	}

	return ret;
}

int * receiveMessage(int newSock)
{

	pthread_t rThread;
	int * status = 0;
	//creating a new thread for receiving messages from the user
	int res = pthread_create(&rThread, NULL, receiveMessageCallback, (void *) newSock);
	if (res)
	{
		printf("\n ERROR: Return Code from pthread_create() is %d \n", res);
		exit(1);
	}

	printf("\n 2.Received: ret %d\n", status);
	pthread_join(rThread, status);
	printf("\n 3.Received: ret %d\n", status);
	return status;
}



void * receiveMessageCallback(void * socket)
{
	int sockfd;
	int * ret = malloc(sizeof(int));
	int bufferSize = 2000;
	int recvMsgSize;
	char buffer[bufferSize];
	sockfd = (int) socket;
	memset(buffer, 0, bufferSize);

	if ((recvMsgSize = recv(sockfd, buffer, bufferSize, 0)) < 0)
	{
		printf("\n Receive failed.........\n");
		exit(1);
	}

	if (recvMsgSize > 0)
	{
		printf("\n Received: %s\n", buffer);
	}

	if (strcmp(buffer, "quit") == 0)
	{
		*ret = 1;
	}

	printf("\n 1.Received: ret %d\n", *ret);
	pthread_exit(ret);
}

int * receiveNotification(int newSock)
{

	pthread_t rThread;
	int * status = 0;
	//creating a new thread for receiving messages from the user
	int res = pthread_create(&rThread, NULL, receiveNotificationCallback, (void *) newSock);
	if (res)
	{
		printf("\n ERROR: Return Code from pthread_create() is %d \n", res);
		exit(1);
	}

	return status;
}

void * receiveNotificationCallback(void * socket)
{
	int sockfd;
	int * ret = malloc(sizeof(int));
	int bufferSize = 2000;
	int recvMsgSize;
	char buffer[bufferSize];
	sockfd = (int) socket;
	memset(buffer, 0, bufferSize);

	if ((recvMsgSize = recv(sockfd, buffer, bufferSize, 0)) < 0)
	{
		printf("\n Notification failed.........\n");
		exit(1);
	}

	if (recvMsgSize > 0)
	{
		printf("\n Notification ........... \n");
	}


	pthread_exit(ret);
}