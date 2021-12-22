#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <sqlite3.h> 

#include <string.h>
#include <string>
#include <iostream>
using namespace std;

#include "db-queries.h"


const int PORT = 2024;
const char * ADRESS = "127.0.0.2";
const int LOGIN_INFO_SIZE = 50;
const int MSG_SIZE = 200;

int errno;

void exitWithErr (string errorMessage);
int isLoginInfoCorrect(string username, string password);

DbQueries database("users.db");

int main ()
{
	int listenSock; 			// listen socket descriptor
    struct sockaddr_in server;	// attaching server info to sock
    struct sockaddr_in client;	// saving client's connection data

    // creating a socket
    if ((listenSock = socket (AF_INET, SOCK_STREAM, 0)) == -1)
		exitWithErr ("[server] Couldn't create the socket. ");

    // clean the structs
    bzero (&server, sizeof (server));
    bzero (&client, sizeof (client));

	// filling the needed struct for the server
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr (ADRESS);
    server.sin_port = htons (PORT);

    // binding server info to sock
    if (bind (listenSock, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    	exitWithErr ("[server] Couldn't bind. ");

    // listen to the clients who want to connect
    if (listen (listenSock, 1) == -1)
		exitWithErr ("[server] Couldn't listen. ");

    // serving clients
    while (1)
    {
    	int clientSock1;
		int clientSock2;
    	unsigned int length = sizeof(client);
		char userLoginInfo[LOGIN_INFO_SIZE * 2] = "\0";
		string username1;
		string username2;
		string password;

    	// accepting a client
    	clientSock1 = accept (listenSock, (struct sockaddr *) &client, &length);

    	// didn't accept the client
    	if (clientSock1 < 0)
    	{
    		perror ("[server] Couldn't accept the client. ");
    		continue;
		}

		// getting the username and the password
		if (read(clientSock1, userLoginInfo, LOGIN_INFO_SIZE * 2) <= 0)
		{
			close(clientSock1);
			perror("[server] Couldn't read username from client. ");
			continue;
		}
		username1 = strtok(userLoginInfo, " ");
		password = strtok(NULL, " ");

		// logging the client
		int loginType;
		while ( !(loginType = isLoginInfoCorrect(username1, password)) )
		{
			if ( write(clientSock1, "username or password wrong", MSG_SIZE) <= 0 )
			{
				close(clientSock1);
				perror("[server] Couldn't write to client. ");
				continue;
			}

			// getting the username and the password
			strcpy(userLoginInfo, "\0");
			if (read(clientSock1, userLoginInfo, LOGIN_INFO_SIZE * 2) <= 0)
			{
				close(clientSock1);
				perror("[server] Couldn't read username from client. ");
				continue;
			}
			username1 = strtok(userLoginInfo, " ");
			password = strtok(NULL, " ");
		}
		
		// write to the client if the account was created or that the client was logged
		if (loginType == 1 &&  write(clientSock1, "login", MSG_SIZE) <= 0)
		{
			close(clientSock1);
			perror("[server] Couldn't write to client. ");
			continue;
		}
		else if (write(clientSock1, "register", MSG_SIZE) <= 0)
		{
			close(clientSock1);
			perror("[server] Couldn't write to client. ");
			continue;
		}

		return 0;
		// // accepting a client
    	// clientSock2 = accept (listenSock, (struct sockaddr *) &client, &length);

    	// // didn't accept the client
    	// if (clientSock2 < 0)
    	// {
    	// 	perror ("[server] Couldn't accept the client. ");
    	// 	continue;
    	// }

		

		// // creating a child for the pair of users
    	// int pid;

		// // an error occured
    	// if ((pid = fork()) == -1) 
		// {
		// 	perror ("[server] Couldn't create a child. ");
    	// 	close (clientSock1);
		// 	close (clientSock2);
    	// 	continue;
    	// } 

		// // parent
		// else if (pid > 0) 
		// {
    	// 	close (clientSock1);
		// 	close (clientSock2);
		// 	// wait for the exited children to finish "courtesy wait"
    	// 	while ( waitpid(-1, NULL, WNOHANG) );
    	// 	continue;
    	// } 

		// // child
		// else if (pid == 0) 
		// {
    	// 	close (listenSock);
		// 	write (clientSock1, msg, 100);
		// 	write (clientSock2, msg, 100);
		// 	close (clientSock1);
		// 	close (clientSock2);
    	// 	exit (0);
    	// }
    }			
}				

void exitWithErr (string errorMessage)
{
    perror (errorMessage.c_str());
    exit (errno);
}

int isLoginInfoCorrect (string username, string password)
{
	if (database.isNameAvailable(username))
	{
		database.addUser(username, password);
		return 2;
	}
	else if (database.isPasswordCorrect(username, password))
		return 1;
	
	return 0;
}