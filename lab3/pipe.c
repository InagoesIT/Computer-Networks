/*
Scrieti un program in care se foloseste un singur pipe pentru comunicare si in care:
    1. tatal scrie intr-un pipe un numar;
    2. fiul verifica daca numarul este par si transmite prin pipe tatalui raspunsul (yes, no);
    3. tatal va afisa raspunsul primit.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main()
{
    int pfd[2], nr;
    char* response;
    int rv = 6;
    
    if ( pipe(pfd) == -1 )
    {
        perror("pipe"); exit(1);
    }

    switch ( fork() )
    {
        case -1:
            perror("fork"); exit(2);
            
        case 0: // copilul
            if ( (read(pfd[0], &nr, sizeof(int))) == -1 )
            {
                perror("read child"); exit(3);
            }
            
            close(pfd[0]);
            
            if ( nr % 2 == 0 )
            {            
                if ( (write(pfd[1], "yes", 3)) == -1 )
                {
                    perror("write child"); exit(4);
                }
            }                
            else
            {            
                if ( (write(pfd[1], "no", 2)) == -1 )
                {
                    perror("write child"); exit(5);
                }
            }
            
            close(pfd[1]);
            
            exit(rv);  
                        
        default: // parintele
            printf("Dati numarul dorit: ");
            scanf("%i", &nr);
            printf("Va multumim, numarul %i a fost receptionat!\n", nr);
                       
            if ( (write(pfd[1], &nr, sizeof(int))) == -1 )
            {
                perror("write parent"); exit(7);
            }         
                                            
            int rspSize;
            
            if ( (rspSize = read(pfd[0], response, 3)) == -1 )
            {
                perror("read parent"); exit(8);
            }            
            
            //wait(&rv);
            
            response[rspSize] = '\0';
            
            printf("Raspunsul este: %s", response); 
            
            close(pfd[0]);
            close(pfd[1]);                      
    }
    
    return 0;

}
