#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>  

#define FIFO_NAME "client_server_fifo"
#define MAX_NAME_SIZE 30
#define MAX_INPUT_SIZE MAX_NAME_SIZE + 9
#define MAX_RESULT_SIZE 500


int exitNr = 1;
void displayError(char* err_text)
{
    perror(err_text); exit(exitNr++);
}

bool validateInput(char input[MAX_INPUT_SIZE])
{
    return ( strcmp(input, "get-logged-users") == 0 || strcmp(input, "quit") == 0 || strcmp(input, "logout") == 0 
            || (strncmp(input, "login", 5) == 0 && input[5] == ' ' && input[6] == ':' && input[7] == ' ') 
            || (strncmp(input, "get-proc-info", 13) == 0 && input[13] == ' ' && input[14] == ':' && input[15] == ' ')
           );
}

int isReadOk (char* result)
{
    int i = 1, size;
    
    size = (result[0] - '0');
    
    while (result[i] >= '0' && result[i] <= '9')
    {
        size *= 10;
        size += result[i++] - '0';
    }
    
    if (strlen(result) - i == size)
        return i;
    else 
        return 0;
}


int main()
{
    char input[MAX_INPUT_SIZE], result[MAX_INPUT_SIZE];   
    int fd;
        
    if ((fd = open (FIFO_NAME, O_RDWR)) == -1)
        displayError("open fifo");       
    
    // processsing and sending input loop
    while (true)
    {
        printf("--> Write the command you want to be executed -> ");
        fgets(input, sizeof(input), stdin);
        input[ strlen(input) - 1 ] = '\0';
       
        if (validateInput(input) == 0)
            printf("[client] ! Invalid command. Try typing another command. !\n");
        else
        {            
            if (write(fd, input, MAX_INPUT_SIZE) == -1)
                displayError("write fifo");
            
            printf("[client] Command was sent to server, please wait.\n");
            
            sleep(3);            
                       
            //reading the result with the size as it's prefix
            if (read(fd, result, MAX_RESULT_SIZE) == -1)
                displayError("read fifo");
              
            // check if text was read correctly  
            int i;
            if ((i = isReadOk(result)) == 0)
                displayError("[client] ! Unsuccesful read from server! Bye. !");
            
            // delete the prefix
            char* formattedRes;
            formattedRes = (char *) malloc(MAX_RESULT_SIZE - 1);
            
            strncpy(formattedRes, result + i, strlen(result) - i);
            strcpy (result, formattedRes);        
            
            // check if I should quit
            if (strcmp(result, "bye") == 0)
            {            
                printf("-- Bye, see you on the other side! --\n");
                break;
            }                
               
            if (strcmp(result, "error") == 0)
            {
                printf("[client] ! There has been an error from server !\n");
                
                //reading the error with the size as it's prefix
                if (read(fd, result, MAX_RESULT_SIZE) == -1)
                    displayError("read fifo");
                  
                // check if text was read correctly  
                int i;
                if ((i = isReadOk(result)) == 0)
                    displayError("[client] ! Unsuccesful read from server! Bye. !");
                
                // delete the prefix
                char* formattedRes;
                formattedRes = (char *) malloc(MAX_RESULT_SIZE - 1);
                
                strncpy(formattedRes, result + i, strlen(result) - i);
                
                printf("%s\n", formattedRes);
                printf("[server] -- Bye! I will miss you! --\n");
                printf("[client] Bye! I will miss you too!\n");                
                
                close(fd);
                   
                exit(0);
            }    
            // display the result
            printf("[server] %s\n", result);
        }           
    }
    
    close(fd);
       
    exit(0);
    return 0;
}
