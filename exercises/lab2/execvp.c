/*
Scrieti un program care executa comanda ls -a -l. Trebuiesc respectate:
    1. procesul fiu este cel care va apela primitiva execvp
    2. procesul tata asteapta ca fiul sa se termine si va afisa un mesaj
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
	int cod_term;
	
    char* args[4];
    args[0] = "ls"; // args[0] = "/usr/bin/ls" (pu execv)
    args[1] = "-a";
    args[2] = "-l";
    args[3] = NULL;
	
	switch( fork() )
	{
		case -1:	perror("Eroare la primul fork");  exit(1);

		case  0:	execvp(args[0], args); // execv(args[0], args);
				    perror("Eroare la exec");  exit(2);
		
		default:	wait(&cod_term);
				    if( WIFEXITED(cod_term) )
					    printf("Comanda ls a rulat, terminandu-se cu codul de terminare: %d.\n", WEXITSTATUS(cod_term) );
				    else
					    printf("Comanda ls a fost terminata fortat de catre semnalul: %d.\n", WTERMSIG(cod_term) );
	}
}
