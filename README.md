# SimpleTCPUDPChatApplication
Talk Application System using TCP and UDP Socket Programming in C
==============================================================
This project is a talk application were the talk server takes care of maintaining the database of users, granting access for 
talk request, sending notification, etc.
The clients talk each other using TCP connection.
Communication between server and clients is using UDP sockets.

Amalgamation file for sqlite3 is used in the project.

Compilation and Execution
==========================

UDPServer.c
===========
Compilation : gcc -std=c99 -o udpTalkServer UDPTalkServer.c Database.c sqlite3.c -lpthread -ldl
Execution UDPServer.c : ./tcpTalkClient 127.0.0.1 23000


UDPClient.c
===========

Compilation: gcc -std=c99 -o tcpTalkClient TCPTalkClient.c -lpthread -ldl
Execution: ./tcpTalkClient 127.0.0.1 23000
