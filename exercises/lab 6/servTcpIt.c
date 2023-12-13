/* servTCPIt.c - Exemplu de server TCP iterativ
   Asteapta un nume de la clienti; intoarce clientului sirul
   "Hello nume".
   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* portul folosit */
#define PORT 2024

/* codul de eroare returnat de anumite apeluri */
/* !!! de analizat */extern int errno;

int isNumber(char* input)
{
	int size = strlen(input) - 1;
	int i = 0;
	
	while (input[i] >= '0' && input[i] <= '9' && i < size)
		i++;
	
	if (i == size)
		return 1;
	else
		return 0;
}

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  char msg[100];				//mesajul primit de la client 
  char msgrasp[200]=" ";        //mesaj de raspuns pentru client
  int sd;						//descriptorul de socket 
  int clientNr = -1; 			//numarul de ordine a clientului
  int number = 0;				//numarul primit de la client

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }

  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 5) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }

  /* servim in mod iterativ clientii... */
  while (1)
    {
      int client;
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      client = accept (sd, (struct sockaddr *) &from, &length);
	
      /* eroare la acceptarea conexiunii de la un client */
      if (client < 0)
		{
		  perror ("[server]Eroare la accept().\n");
		  continue;
		}
	  
	  clientNr++; // ne-am conectat la client
	  
      /* s-a realizat conexiunea, se astepta mesajul */
      bzero (msg, 100);
      printf ("[server]Asteptam mesajul...\n");
      fflush (stdout);
      
      /* citirea mesajului */
      if (read (client, msg, 100) <= 0)
	{
	  perror ("[server]Eroare la read() de la client.\n");
	  close (client);	/* inchidem conexiunea cu clientul */
	  continue;		/* continuam sa ascultam */
	}
	
      printf ("[server]Mesajul a fost receptionat...%s\n", msg);
      
      /*pregatim mesajul de raspuns */
      if ( isNumber(msg) )
      {
      	number = atoi(msg);
		bzero(msgrasp,200);
		sprintf(msgrasp,"input: %d, client number: %d ", number, clientNr);
      }
      else
      {
      	bzero(msgrasp,200);
		sprintf(msgrasp,"input: NOT A NUMBER, client number: %d ", clientNr);
      }
      
      printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);
      
      
      /* returnam mesajul clientului */
      if (write (client, msgrasp, 200) <= 0)
	{
	  perror ("[server]Eroare la write() catre client.\n");
	  continue;		/* continuam sa ascultam */
	}
      else
	printf ("[server]Mesajul a fost trasmis cu succes.\n");
      /* am terminat cu acest client, inchidem conexiunea */
      close (client);
    }				/* while */
}				/* main */