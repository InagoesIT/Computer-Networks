/* Program: 
Scrieti un program in care procesul tata testeaza daca procesul fiu are un PID par. In cazul in care rezultatul este pozitiv, tatal ii scrie fiului mesajul “fortune”, mesaj care va fi afisat de fiu. Daca PID-ul este impar, tatal va scrie mesajul “lost” si moare inaintea fiului.

Detalii: Mesajul va fi un sir de caractere care va fi scris de procesul tata intr-un fisier si apoi citit de fiu. Amandoi cunosc denumirea fisierului.
*/

#include <stdio.h> 
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

int errNr = 2;
int BUFF_SIZE = 20;

void handleErr (char* text)
{
    perror(text); 
    exit(errNr++);
}

int main()
{
    pid_t pid;
    int fd, childExit = 1;
    char* FILE_NAME = "result.txt";
    
    if ( (pid = fork()) ) // parent
    {                       
        if( (fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC)) == -1)
            handleErr("file not opened");
                
        if (pid % 2 == 0)
        {
            char* text = "fortune";         
                
            if ( write(fd, text, strlen(text)) == -1 )
                handleErr("fortune writing failed");
            
            wait(&childExit);     
        }
        else
        {
            char* text = "lost";
                
            if ( write(fd, text, strlen(text)) == -1)
                handleErr("lost writing failed");            
        }
        
        close(fd);
    }
    
    else if (pid == -1) //error
        handleErr("fork");
        
    else //child
    {
        char result[BUFF_SIZE];
        int readRes = 0;
        
        if( ( fd = open(FILE_NAME, O_RDONLY)) == -1)
            handleErr("file not opened");
                
        while ( (readRes = read(fd, result, BUFF_SIZE)) == 0 )
        {        
            if ( readRes == -1 )
                handleErr("reading failed");
        }        
        
        close(fd);
        
        result[readRes] = '\0'; // magic happened here for lost, so I needed to establish the ending myself
        printf("%s\n", result);        
        
        exit(childExit);
    }
}
