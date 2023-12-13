/* 
Sa se scrie un program care afiseaza din 3 in 3 secunde pid-ul procesului curent si a cata afisare este. La primirea semnalului SIGUSR1 se scrie intr-un fisier mesajul "Am primit semnal".
Semnalul SIGINT se va ignora in primele 60 de secunde de la inceperea rularii programului si apoi va avea actiunea default.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

int main()
{
    sigset_t mask_sigint_blocked, mask_sigint_unblocked;
    
    sigemptyset(&mask_sigint_blocked);
	sigaddset(&mask_sigint_blocked,SIGINT);
	
	sigemptyset(&mask_sigint_unblocked);
	sigdelset(&mask_sigint_unblocked,SIGINT);
	
	if(-1 == sigprocmask(SIG_BLOCK, &mask_sigint_blocked, NULL) )
	{
	    perror("sigprocmask err"); exit(1);
	}
	
	

}
