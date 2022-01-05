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
const char *ADRESS = "127.0.0.1";
const int LOGIN_INFO_SIZE = 50;
const int MSG_SIZE = 600;
int errno;
DbQueries database("users.db");

void exitWithErr(string errorMessage);
int isLoginInfoCorrect(string username, string password);
bool giveLeaderboard(int clientSock1, int clientSock2);
bool showLeaderboard(int clientSock1, int clientSock2);

int main()
{
	int listenSock;				// listen socket descriptor
	struct sockaddr_in server;	// attaching server info to sock
	struct sockaddr_in client1; // saving client's connection data
	struct sockaddr_in client2; // saving client's connection data

	// creating a socket
	if ((listenSock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		exitWithErr("[server] Couldn't create the socket. ");

	// clean the structs
	bzero(&server, sizeof(server));
	bzero(&client1, sizeof(client1));
	bzero(&client2, sizeof(client2));

	// filling the needed struct for the server
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ADRESS);
	server.sin_port = htons(PORT);

	// binding server info to sock
	if (bind(listenSock, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
		exitWithErr("[server] Couldn't bind. ");

	// listen to the clients who want to connect
	if (listen(listenSock, 1) == -1)
		exitWithErr("[server] Couldn't listen. ");

	// serving clients
	int clientSock1;
	int clientSock2;
	unsigned int lengthSockInfo = sizeof(client1);
	int loginType;
	char userLoginInfo[LOGIN_INFO_SIZE * 2];
	char msg[MSG_SIZE];
	string username1;
	string username2;
	string password;

	while (1)
	{
		memset(userLoginInfo, 0, LOGIN_INFO_SIZE * sizeof(userLoginInfo[0]));
		// accepting a client
		clientSock1 = accept(listenSock, (struct sockaddr *)&client1, &lengthSockInfo);

		// didn't accept the client 1
		if (clientSock1 < 0)
		{
			perror("[server] Couldn't accept the client 1. ");
			continue;
		}

		// getting the username and the password
		if (read(clientSock1, userLoginInfo, LOGIN_INFO_SIZE * 2) <= 0)
		{
			close(clientSock1);
			perror("[server] Couldn't read login info from client 1. ");
			continue;
		}
		username1 = strtok(userLoginInfo, " ");
		password = strtok(NULL, " ");

		// logging the client 1
		while (!(loginType = isLoginInfoCorrect(username1, password)))
		{
			if (write(clientSock1, "username or password wrong", MSG_SIZE) <= 0)
			{
				close(clientSock1);
				perror("[server] Couldn't write to client 1. ");
				continue;
			}

			// getting the username and the password
			memset(userLoginInfo, 0, LOGIN_INFO_SIZE * sizeof(userLoginInfo[0]));
			if (read(clientSock1, userLoginInfo, LOGIN_INFO_SIZE * 2) <= 0)
			{
				close(clientSock1);
				perror("[server] Couldn't read login info from client 1. ");
				continue;
			}
			username1 = strtok(userLoginInfo, " ");
			password = strtok(NULL, " ");
		}

		// write to the client 1 if the account was created or that the client was logged
		if (loginType == 1 && (write(clientSock1, "login", MSG_SIZE) <= 0))
		{
			close(clientSock1);
			perror("[server] Couldn't write to client 1. ");
			continue;
		}
		else if (loginType == 2 && write(clientSock1, "register", MSG_SIZE) <= 0)
		{
			close(clientSock1);
			perror("[server] Couldn't write to client 1. ");
			continue;
		}

		// accepting client 2
		clientSock2 = accept(listenSock, (struct sockaddr *)&client2, &lengthSockInfo);

		// didn't accept client 2
		if (clientSock2 < 0)
		{
			perror("[server] Couldn't accept the client 2. ");
			continue;
		}

		// getting the username and the password
		memset(userLoginInfo, 0, LOGIN_INFO_SIZE * sizeof(userLoginInfo[0]));
		if (read(clientSock2, userLoginInfo, LOGIN_INFO_SIZE * 2) <= 0)
		{
			close(clientSock2);
			perror("[server] Couldn't read login info from client 2. ");
			continue;
		}
		username2 = strtok(userLoginInfo, " ");
		password = strtok(NULL, " ");

		// logging the client 2
		while (!(loginType = isLoginInfoCorrect(username2, password)))
		{
			if (write(clientSock2, "username or password wrong", MSG_SIZE) <= 0)
			{
				close(clientSock2);
				perror("[server] Couldn't write to client 2. ");
				continue;
			}

			// getting the username and the password
			memset(userLoginInfo, 0, LOGIN_INFO_SIZE * sizeof(userLoginInfo[0]));
			if (read(clientSock2, userLoginInfo, LOGIN_INFO_SIZE * 2) <= 0)
			{
				close(clientSock2);
				perror("[server] Couldn't read login info from client 2. ");
				continue;
			}
			username2 = strtok(userLoginInfo, " ");
			password = strtok(NULL, " ");
		}

		// write to the client 2 if the account was created or that the client was logged
		if (loginType == 1 && (write(clientSock2, "login", MSG_SIZE) <= 0))
		{
			close(clientSock2);
			perror("[server] Couldn't write to client 2. ");
			continue;
		}
		else if (loginType == 2 && write(clientSock2, "register", MSG_SIZE) <= 0)
		{
			close(clientSock2);
			perror("[server] Couldn't write to client 2. ");
			continue;
		}

		// tell the clients their opponent's name
		char username1Char[LOGIN_INFO_SIZE];
		strcpy(username1Char, username1.c_str());
		char username2Char[LOGIN_INFO_SIZE];
		strcpy(username2Char, username2.c_str());

		// tell the other client if the server can't contact their opponent
		if (write(clientSock1, username2Char, LOGIN_INFO_SIZE) <= 0)
		{
			close(clientSock1);
			perror("[server] Couldn't write to client 1.");
			if (write(clientSock2, "opponent down", MSG_SIZE) <= 0)
			{
				close(clientSock2);
				perror("[server] Couldn't write to client 2.");
				continue;
			}
			continue;
		}
		if (write(clientSock2, username1Char, LOGIN_INFO_SIZE) <= 0)
		{
			close(clientSock2);
			perror("[server] Couldn't write to client 2.");
			if (write(clientSock1, "opponent down", MSG_SIZE) <= 0)
			{
				close(clientSock1);
				perror("[server] Couldn't write to client 1.");
				continue;
			}
			continue;
		}

		// creating a child for the pair of users
		int pid;

		// an error occured
		if ((pid = fork()) == -1)
		{
			perror("[server] Couldn't create a child. ");
			close(clientSock1);
			close(clientSock2);
			continue;
		}

		// parent
		else if (pid > 0)
		{
			close(clientSock1);
			close(clientSock2);
			// wait for the exited children to finish - "courtesy wait"
			while (waitpid(-1, NULL, WNOHANG));
			continue;
		}

		// child
		else if (pid == 0)
		{
			close(listenSock);

			// LEADERBOARD
			// check if an error occured
			if (!giveLeaderboard(clientSock1, clientSock2))
				continue;
			

			// GAME
			// choose gamers colors
			// srand(time(0));
			// int number = rand() % 2;
			// int playersSock[2];
			// if (number)
			// 	playersSock
			// bool isGameEnded = 0;

			// while (!isGameEnded)
			// {
				
			// }

			close(clientSock1);
			close(clientSock2);
			exit(0);
		}
	}
}

void exitWithErr(string errorMessage)
{
	perror(errorMessage.c_str());
	exit(errno);
}

int isLoginInfoCorrect(string username, string password)
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

bool giveLeaderboard(int clientSock1, int clientSock2)
{
	char msg[MSG_SIZE];

	// find out if the first client wants to see the leaderboard
	memset(msg, 0, MSG_SIZE * sizeof(msg[0]));
	if (read(clientSock1, msg, MSG_SIZE) <= 0)
	{
		close(clientSock1);
		perror("[server] Couldn't read leaderboard preference from client 1. ");
		if (write(clientSock2, "opponent down", MSG_SIZE) <= 0)
		{
			close(clientSock2);
			perror("[server] Couldn't write to client 2.");
			return false;
		}
		return false;
	}

	// give the leaderboard if wanted
	if (!strcmp(msg, "Y") || !strcmp(msg, "y"))
	{
		if (!showLeaderboard(clientSock1, clientSock2))
			return false;
	}

	// find out if the second client wants to see the leaderboard
	memset(msg, 0, MSG_SIZE * sizeof(msg[0]));
	if (read(clientSock2, msg, MSG_SIZE) <= 0)
	{
		close(clientSock2);
		perror("[server] Couldn't read leaderboard preference from client 1. ");
		if (write(clientSock1, "opponent down", MSG_SIZE) <= 0)
		{
			close(clientSock1);
			perror("[server] Couldn't write to client 2.");
			return false;
		}
		return false;
	}
	
	// give the leaderboard if wanted
	if (!strcmp(msg, "Y") || !strcmp(msg, "y"))
	{
		if (!showLeaderboard(clientSock2, clientSock1))
			return false;
	}	

	return true;
}

bool showLeaderboard(int clientSock1, int clientSock2)
{
	char msg[MSG_SIZE];
	memset(msg, 0, MSG_SIZE * sizeof(msg[0]));
	if (read(clientSock1, msg, MSG_SIZE) <= 0)
	{
		close(clientSock1);
		perror("[server] Couldn't read leaderboard preference from client. ");
		if (write(clientSock2, "opponent down", MSG_SIZE) <= 0)
		{
			close(clientSock2);
			perror("[server] Couldn't write to client 2.");
			return false;
		}
		return false;
	}

	database.getNLeaders(clientSock1, atoi(msg));

	return true;
}