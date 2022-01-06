#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <string>
#include <iostream>

#include <SDL2/SDL.h>
#include <sqlite3.h>

using namespace std;

#include "db-queries.h"
#include "board.h"

struct clientSocks
{
	int sock1;
	int sock2;
};

const int PORT = 2024;
const char *ADRESS = "127.0.0.1";
const int LOGIN_INFO_SIZE = 50;
const int MSG_SIZE = 600;
int errno;

DbQueries database("users.db");

void exitWithErr(string errorMessage);
int isLoginInfoCorrect(string username, string password);
void writeOppDown(int clientSock1, int clientSock2);
bool writeMsgToClient(int clientSock1, int clientSock2, const char msg[MSG_SIZE]);
void writeMsgToClientWithExit(int clientSock1, int clientSock2, const char msg[MSG_SIZE]);
static void * giveLeaderboard(void * args);
bool showLeaderboard(int clientSock1, int clientSock2);

int main()
{
	int clientSock1;            // first client's socket descriptor
	int clientSock2;			// second client's socket descriptor
	int listenSock;				// listen socket descriptor
	struct sockaddr_in server;	// attaching server info to sock
	struct sockaddr_in client1; // saving client's connection data
	struct sockaddr_in client2; // saving client's connection data

	// creating a socket
	if ((listenSock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		exitWithErr("[server] Couldn't create the socket. ");

	// reuse address
	int on = 1;
	setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

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
		char username2Char[LOGIN_INFO_SIZE];
		strcpy(username1Char, username1.c_str());
		strcpy(username2Char, username2.c_str());

		if (!writeMsgToClient(clientSock1, clientSock2, username2Char))
			continue;
		if (!writeMsgToClient(clientSock2, clientSock1, username1Char))
			continue;

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
			// create threads for the leaderboard
			pthread_t threadId[2];    					// identifier of the created thread

			struct clientSocks * client1Socks = (struct clientSocks*)malloc(sizeof(struct clientSocks));	
			client1Socks->sock1 = clientSock1;
			client1Socks->sock2 = clientSock2;

			struct clientSocks * client2Socks = (struct clientSocks*)malloc(sizeof(struct clientSocks));
			client2Socks->sock1 = clientSock2;
			client2Socks->sock2 = clientSock1;

			pthread_create(&threadId[0], NULL, &giveLeaderboard, client1Socks);
			pthread_create(&threadId[1], NULL, &giveLeaderboard, client2Socks);

			// wait for threads to finish execution
			pthread_join(threadId[0], NULL);
			pthread_join(threadId[1], NULL);

			// GAME
			// choose gamers colors and order
			int playersSock[2];
			bool colors[2];

			srand((unsigned) time(0));
			int order = rand() % 2;
			int color = rand() % 2;

			playersSock[order] = clientSock1;
			playersSock[!order] = clientSock2;
			colors[order] = color;
			colors[!order] = !color;

			// send result to players
			memset(msg, 0, MSG_SIZE * sizeof(msg[0]));
			sprintf(msg, "%d%d", order, color);
			writeMsgToClient(clientSock1, clientSock2, msg);

			memset(msg, 0, MSG_SIZE * sizeof(msg[0]));
			sprintf(msg, "%d%d", !order, !color);
			writeMsgToClient(clientSock2, clientSock1, msg);

			// start game
			bool currPlayer = 0;
			char move[3];
			bool isMoveValid = false;

			Board board(username1, username2);
			bool isEnd = board.isGameEnded();

			while (!isEnd)
			{
				// the current player can't move
				if (!board.canMove(colors[currPlayer]) )
				{
					writeMsgToClientWithExit(playersSock[currPlayer], playersSock[!currPlayer], "skip");
					currPlayer = !currPlayer;
					continue;	
				}

				// try to get a possible move from the player
				while(!isMoveValid)
				{
					// request a move from the client
					writeMsgToClientWithExit(playersSock[currPlayer], playersSock[!currPlayer], "move");
					// get the move
					bzero(move, 3);
					if (read(playersSock[currPlayer], move, 3) <= 0)
					{
						writeOppDown(playersSock[!currPlayer], playersSock[currPlayer]);
						exitWithErr("Couldn't get move from client");
					}
					if (board.isMovePossible(move[1] - '1', move[0]- 'A' , colors[currPlayer]))
						break;
				}

				writeMsgToClientWithExit(playersSock[currPlayer], playersSock[!currPlayer], "success");
				board.makeMove(move[1] - '1', move[0]- 'A', colors[currPlayer]);

				if (!(isEnd = board.isGameEnded()))
					currPlayer = !currPlayer;	
			}

			// tell the clients that the game has ended
			writeMsgToClientWithExit(clientSock1, clientSock2, "end");
			writeMsgToClientWithExit(clientSock2, clientSock1, "end");

			int wonRes = board.whoWon();
			bool blackIndex = 0;

			if (colors[0] == 1)
				blackIndex = !blackIndex;

			// make username1 be the one with black, and 2 white
			if (!blackIndex)
				swap(username1, username2);

			if (wonRes == 0)
			{
				database.incrementScore(username1);
				writeMsgToClientWithExit(playersSock[blackIndex], playersSock[!blackIndex], "won");
				writeMsgToClientWithExit(playersSock[!blackIndex], playersSock[blackIndex], "lost");
			}
			else if (wonRes == 1)
			{
				database.incrementScore(username2);
				writeMsgToClientWithExit(playersSock[!blackIndex], playersSock[blackIndex], "won");
				writeMsgToClientWithExit(playersSock[blackIndex], playersSock[!blackIndex], "lost");
			}
			else
			{
				database.incrementScore(username1);
				database.incrementScore(username2);
				writeMsgToClientWithExit(clientSock1, clientSock2, "draw");
				writeMsgToClientWithExit(clientSock2, clientSock1, "draw");
			}

			// give the leaderboard if the user wants
			pthread_create(&threadId[0], NULL, &giveLeaderboard, client1Socks);
			pthread_create(&threadId[1], NULL, &giveLeaderboard, client2Socks);

			// wait for threads to finish execution
			pthread_join(threadId[0], NULL);
			pthread_join(threadId[1], NULL);

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

void * giveLeaderboard(void * args)
{
	struct clientSocks clients_socks;
	clients_socks = *((struct clientSocks * )args);	
	char msg[MSG_SIZE];

	// find out if the client wants to see the leaderboard
	memset(msg, 0, MSG_SIZE * sizeof(msg[0]));
	if (read(clients_socks.sock1, msg, MSG_SIZE) <= 0)
	{
		close(clients_socks.sock1);
		perror("[server] Couldn't read leaderboard preference from client 1. ");
		if (write(clients_socks.sock2, "opponent down", MSG_SIZE) <= 0)
		{
			close(clients_socks.sock2);
			perror("[server] Couldn't write to client 2.");
			exit(0);
		}		
	}

	// give the leaderboard if wanted
	if (!strcmp(msg, "Y") || !strcmp(msg, "y"))
	{
		if (!showLeaderboard(clients_socks.sock1, clients_socks.sock2))
			exit(0);
	}

	return NULL;
}

bool showLeaderboard(int clientSock1, int clientSock2)
{
	char msg[MSG_SIZE];
	bzero(msg, MSG_SIZE);
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

bool writeMsgToClient(int clientSock1, int clientSock2, const char msg[MSG_SIZE])
{
	if (write(clientSock1, msg, MSG_SIZE) <= 0)
	{
		close(clientSock1);
		perror("[server] Couldn't write to client.");
		if (write(clientSock2, "opponent down", MSG_SIZE) <= 0)
		{
			close(clientSock2);
			perror("[server] Couldn't write to client.");
			return false;
		}
		return false;
	}
	return true;
}

void writeMsgToClientWithExit(int clientSock1, int clientSock2, const char msg[MSG_SIZE])
{
	if (write(clientSock1, msg, MSG_SIZE) <= 0)
	{
		close(clientSock1);
		exitWithErr("[server] Couldn't write to client.");
		if (write(clientSock2, "opponent down", MSG_SIZE) <= 0)
		{
			close(clientSock2);
			exitWithErr("[server] Couldn't write to client.");
		}
		close(clientSock2);
	}
}

void writeOppDown(int clientSock1, int clientSock2)
{
	close(clientSock2);
	if (write(clientSock1, "opponent down", MSG_SIZE) <= 0)
	{
		close(clientSock1);
		exitWithErr("[server] Couldn't write to client.");
	}
	close(clientSock1);
}