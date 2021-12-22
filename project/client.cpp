#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <iostream>
using namespace std;

const int PORT = 2024; 
const char * ADRESS = "127.0.0.2";
const int LOGIN_INFO_SIZE = 50;
const int MSG_SIZE = 200;

int errno;
string username;
string password;

void exitWithErr(string errorMessage);
bool isMoveValid (string move);
int isLoginInfoInvalid(string info);
void getLoginInfo();

int main () 
{
    int sock; // socket descriptor
    struct sockaddr_in server; // data for connecting to server
    char msg[MSG_SIZE] = "\0";
    bool isLogged = false;
    char loginInfo[LOGIN_INFO_SIZE * 2] = "\0";

    // creating the socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        exitWithErr ("[client] Couldn't create the socket. ");

    // clean the struct
    bzero (&server, sizeof (server));

    // filling the needed struct for communicating with the server
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr (ADRESS);
    server.sin_port = htons (PORT); // any free port chosen

    // connecting to the server
    if (connect (sock, (struct sockaddr * ) & server, sizeof(struct sockaddr)) == -1) 
        exitWithErr ("[client] Couldn't connect to the server. ");

    // LOGIN
    
    cout << "[client] You need to log in or register in order to play a game." << endl;
    
    while (!isLogged)
    {
        // getting login info from the client
        getLoginInfo();

        // send login info to server
        string loginInfoAux = username + " " + password;
        strcpy(loginInfo, loginInfoAux.c_str());

        if ( write(sock, loginInfo, strlen(loginInfo)) <= 0 )
            exitWithErr("[client] Couldn't write login info to server.");

        // check if login was successful
        if ( read(sock, msg, MSG_SIZE) <= 0 )
            exitWithErr("[client] Couldn't read the login result from server.");
        if (strcmp(msg, "username or password wrong"))
            isLogged = true;
        else
            cout << "[server] Username or password incorrect! Try again." << endl;
    }

    if (!strcmp(msg, "login"))
        cout << "[server] Login successful." << endl;
    else
        cout << "[server] Registering successful." << endl;

    // closing the conection
    close (sock);

    return 0;
}


void exitWithErr (string errorMessage)
{
    perror (errorMessage.c_str());
    exit (errno);
}

bool isMoveValid (string move)
{
    if (move.length() > 2)
        return false;
    if (move.at(0) < 'A' || move.at(0) > 'H')
        return false;
    if (move.at(1) < '1' || move.at(1) > '8')
        return false;

    return true;
}

int isLoginInfoInvalid(string info)
{
    if (info.length() > LOGIN_INFO_SIZE && info.find(' ') != string::npos)
        return 1;
    else if (info.length() > LOGIN_INFO_SIZE)
        return 2;
    else if (info.find(' ') != string::npos)
        return 3;

    return 0;
}

void getLoginInfo()
{
    int errorNr;
    int i = 0;
    string info;
    string infoType = "username";

    for ( ; i < 2; i++)
    {
        cout << "[client] Type your " << infoType << 
            " containing no spaces with maximum " << LOGIN_INFO_SIZE <<" characters..." << endl;
        getline (cin, info);

        while ( (errorNr = isLoginInfoInvalid(info)) )
        {
            if (errorNr == 1)
                cout << "[client] Your " << infoType << " can't have more than " << LOGIN_INFO_SIZE <<" characters or spaces!" << endl;
            else if (errorNr == 2)
                cout << "[client] Your " << infoType << " can't have more than " << LOGIN_INFO_SIZE <<" characters!" << endl;
            else 
                cout << "[client] Your " << infoType << " can't contain spaces!" << endl;

            cout << "[client] Type another  " << infoType << "..." << endl;
            getline (cin, info);
        }

        if (i == 0)
            username = info;
        infoType = "password";
    }
    password = info;
}