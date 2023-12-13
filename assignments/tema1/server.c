#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <utmp.h>

#define FIFO_NAME "client_server_fifo"
#define MAX_NAME_SIZE 30
#define MAX_INPUT_SIZE MAX_NAME_SIZE + 9
#define MAX_RESULT_SIZE 500
#define USERS_FILE "users.txt"
#define USERS_LINE_SIZE 20


int exitNr = 1;
void displayError(char* err_text)
{
    perror(err_text); exit(exitNr++);
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

bool isConfigUser(char* user)
{
    int line_size; 
    FILE *users_file;
    size_t line_buf_size = 0;   
    char* user_line = NULL;
    bool isFound = false;
    
    if ((users_file = fopen(USERS_FILE, "r")) == NULL)
        displayError("[client] users file open");
                   
    line_size = getline(&user_line, &line_buf_size, users_file);
    user_line[ strlen(user_line) - 1 ] = '\0';
    
    if (strcmp(user_line, user) == 0)
        isFound = true;
    else
    {
        while (line_size >= 0)
        {
            line_size = getline(&user_line, &line_buf_size, users_file);
            user_line[ strlen(user_line) - 1 ] = '\0';
            if (strcmp(user_line, user) == 0)
            {
                isFound = true;
                break;
            }
        }
    }
    
    fclose(users_file);
    return isFound;
}
    
bool pidHasLetters(char* pid)
{
    int i = 0;
    int size = strlen(pid);
    bool hasLetters = false;
    
    while( pid[i] >= '0' && pid[i] <= '9' && i < size )
        i++;
        
    if (i != size)
        hasLetters = true;
    
    return hasLetters;    
}
    
    
int main()
{
    char input[MAX_INPUT_SIZE], *result; 
    char logged_user[MAX_NAME_SIZE] = "\0";
    int nr;
    int fd, pfd[2];   
    
    //create fifo
    if ((mkfifo (FIFO_NAME, 0666) == -1) && (errno != EEXIST)) // ignore if fifo already exists
        displayError("[server] fifo");
     
    // open fifo    
    if ((fd = open (FIFO_NAME, O_RDWR)) == -1)
        displayError("[server] open fifo");
        
    // create pipe for login
    if (pipe (pfd) == -1) 
    {
        write(fd, "5error", MAX_RESULT_SIZE);
        sleep(3);
        write(fd, "[server] pipe", MAX_RESULT_SIZE);
        displayError("[server] pipe");
    
    }       
        
    // login child
    switch (fork())
    {
        // error
        case -1:
        {
            write(fd, "5error", MAX_RESULT_SIZE);
            sleep(3);
            write(fd, "20[server] login child", MAX_RESULT_SIZE);
            displayError("[server] login child");
        
        }
        
        // child
        case 0: 
            while (true)
            {               
                if (read(pfd[0], input, sizeof(input)) == -1)
                {
                    write(pfd[1], "5error", MAX_RESULT_SIZE);
                    sleep(3);
                    write(pfd[1], "20[server] input login", MAX_RESULT_SIZE);
                    displayError("[login child] input login");                
                }
                    
                result = (char *) malloc(MAX_RESULT_SIZE);
                
                if (strcmp(input, "quit") == 0)
                    break;
                    
                char* user = (char *) malloc(MAX_NAME_SIZE);
                strncpy(user, input + 8, strlen(input) - 8);
                
                sprintf(result, "%ld%s", strlen(user), user);
                
                if (isConfigUser(user))
                    sprintf(result, "%ld%s", strlen(user), user);
                else
                    strcpy(result, "4fail");
                
                if (write(pfd[1], result, MAX_RESULT_SIZE) == -1)
                {
                    write(pfd[1], "5error", MAX_RESULT_SIZE);  
                    sleep(3);                  
                    write(pfd[1], "21[server] result login", MAX_RESULT_SIZE);
                    displayError("[login child] result login");                
                }
                 
                sleep(4);
            }  
             
            close(pfd[0]);
            close(pfd[1]);
            exit(exitNr++);
    }    
    
    
    // create socket for get-logged-users
    int sock_log[2];
    
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sock_log) == -1)
    {
        write(fd, "5error", MAX_RESULT_SIZE); 
        sleep(3);            
        write(fd, "28[server] socket logged users", MAX_RESULT_SIZE);
        displayError("[server] socket logged users");                
    }
    
    // get-logged-users child + close the pipe
    switch (fork())
    {
        // error
        case -1:
        {
            write(fd, "5error", MAX_RESULT_SIZE);
            sleep(3);
            write(fd, "26[server] fork logged users", MAX_RESULT_SIZE);
            displayError("[server] fork logged users");        
        }
        
        // child
        case 0:   
            // close the pipe with login   
            close(pfd[0]);
            close(pfd[1]);    
              
            // close parent's socket end
            close(sock_log[1]);
            
            while (true)
            {
                if (read(sock_log[0], input, sizeof(input)) == -1)
                {
                    write(sock_log[0], "5error", MAX_RESULT_SIZE);
                    sleep(3);
                    write(sock_log[0], "39[logged users child] input logged users", MAX_RESULT_SIZE);                    
                    displayError("[logged users child] input logged users");        
                }
                
                if (strcmp(input, "quit") == 0)
                    break;    
                
                // move the file pointer to the beggining
                setutent();    
                // the structure which will hold information for a user
                struct utmp *log_struct;
                
                // preparing the strings for use
                char* line = (char *) malloc(MAX_RESULT_SIZE);    
                result = (char *) malloc(MAX_RESULT_SIZE);
                char* result_aux = (char *) malloc(MAX_RESULT_SIZE);
                 
                while ( (log_struct = getutent()) )
                {
                    sprintf(line, "> username: %s, hostname: %s, time in seconds: %d, time in microseconds: %d\n", 
                            log_struct->ut_user, log_struct->ut_host, log_struct->ut_tv.tv_sec, 
                            log_struct->ut_tv.tv_usec );
                    strcat(result_aux, line);
                }
                
                // close the utmp file
                endutent();                
                
                // format result_aux and the result
                result_aux[ strlen(result_aux) - 1] = '\0';                
                sprintf(result, "%ld%s", strlen(result_aux), result_aux); 
                
                //check if extracting info was succesful
                if (strcmp(result, "\0") == 0)
                    strcpy(result, "4fail");                    
                    
                if (write(sock_log[0], result, MAX_RESULT_SIZE) == -1)
                {
                    write(sock_log[0], "5error", MAX_RESULT_SIZE); 
                    sleep(3);                   
                    write(sock_log[0], "40[logged users child] result logged users", MAX_RESULT_SIZE); 
                    displayError("[logged users child] result logged users");        
                }
                   
                sleep(20);                 
            }  
             
            close(sock_log[0]);
            exit(exitNr++);
    }
    
    
    
    // create socket for get-proc-info
    int sock_proc[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sock_proc) == -1)
    {
        write(fd, "5error", MAX_RESULT_SIZE);   
        sleep(3);     
        write(fd, "40[server] socket proc", MAX_RESULT_SIZE); 
        displayError("[server] socket proc");        
    }
    
    // get-proc-info child + close the pipe
    switch (fork())
    {
        // error
        case -1:
        {
            write(fd, "5error", MAX_RESULT_SIZE);
            sleep(3);
            write(fd, "19[server] fork proc", MAX_RESULT_SIZE); 
            displayError("[server] fork proc");        
        }
        
        // child
        case 0:   
            // close the pipe with login   
            close(pfd[0]);
            close(pfd[1]);    
            // close the socket with get-logged-users
            close(sock_log[0]);
            close(sock_log[1]);
            
            // close parent's socket end
            close(sock_proc[1]);
            
            char* file_name, *pidCh, *result_aux;
            file_name = (char *) malloc(50);
            int pid;
            
            while (true)
            {
                if (read(sock_proc[0], input, sizeof(input)) == -1)
                {
                    write(sock_proc[0], "5error", MAX_RESULT_SIZE);                    
                    sleep(3);
                    write(sock_proc[0], "31[proc child] input logged users", MAX_RESULT_SIZE); 
                    displayError("[proc child] input logged users");        
                }
                    
                result = (char *) malloc(MAX_RESULT_SIZE);
                result_aux = (char *) malloc(MAX_RESULT_SIZE);
                
                if (strcmp(input, "quit") == 0)
                    break;
                    
                pidCh = (char *) malloc(15);
                strncpy(pidCh, input + 16, strlen(input) - 16);
                
                // the pid is not a number
                if (pidHasLetters(pidCh))
                {                
                    if (write(sock_proc[0], "21! pid isn't correct !", MAX_RESULT_SIZE) == -1)
                    {
                        write(sock_proc[0], "5error", MAX_RESULT_SIZE);                        
                        sleep(3);
                        write(sock_proc[0], "37[proc child] result pid err from proc", MAX_RESULT_SIZE); 
                        displayError("[proc child] result pid err from proc");        
                    }
                }
                
                //the pid is correct      
                else
                {
                    pid = atoi(pidCh);
                    sprintf(file_name, "/proc/%d/status", pid);
                                
                    int line_size; 
                    FILE *proc_file;
                    size_t proc_line_size = 0;   
                    char* proc_line = NULL;
                    
                    if ((proc_file = fopen(file_name, "r")) == NULL)
                    {
                        write(sock_proc[0], "5error", MAX_RESULT_SIZE);                        
                        sleep(3);
                        write(sock_proc[0], "29[proc child] open status file", MAX_RESULT_SIZE); 
                        displayError("[proc child] open status file");        
                    }
                                   
                    // reading from /proc/<pid>/status                                      
                    while ((line_size = getline(&proc_line, &proc_line_size, proc_file)) >= 0)
                    {
                        proc_line[ strlen(proc_line) ] = '\0';
                        
                        if (  strncmp(proc_line, "Name", 4) == 0 || strncmp(proc_line, "State", 5) == 0 
                             || strncmp(proc_line, "PPid", 4) == 0 || strncmp(proc_line, "Uid", 3) == 0
                             || strncmp(proc_line, "VmSize", 6) == 0
                           )
                           strcat(result_aux, proc_line);
                    }
                    
                    fclose(proc_file);
                    
                    // format result_aux and the result
                    result_aux[ strlen(result_aux) - 1] = '\0';                
                    sprintf(result, "%ld%s", strlen(result_aux), result_aux);               
                                 
                    if (write(sock_proc[0], result, MAX_RESULT_SIZE) == -1)
                    {
                        write(sock_proc[0], "5error", MAX_RESULT_SIZE);                        
                        sleep(3);
                        write(sock_proc[0], "32[proc child] result logged users", MAX_RESULT_SIZE); 
                        displayError("[proc child] result logged users");        
                    }
                        
                    sleep(20); 
                }               
                
            }                
                            
            close(sock_proc[0]);
            exit(exitNr++);
    }
    
    // close children sockets' ends
    close(sock_log[0]);
    close(sock_proc[0]);
        
    // main loop
    while (true)
    {    
        if ((nr = read(fd, input, MAX_INPUT_SIZE)) == -1)
        {
            write(fd, "5error", MAX_RESULT_SIZE);            
            sleep(3);
            write(fd, "18[server] read fifo", MAX_RESULT_SIZE); 
            displayError("[server] read fifo");        
        }
        
        result = (char *) malloc(MAX_RESULT_SIZE);
        
        //if no one is logged and command is logout, get-logged-users or get-proc-info : <pid>
        if ( strcmp(logged_user, "\0") == 0 
             && ( strcmp(input, "logout") == 0 || strcmp(input, "get-logged-users") == 0 
                  || (strncmp(input, "get-proc-info", 13) == 0 && input[13] == ' ' && input[14] == ':'
                         && input[15] == ' ') 
                ) 
           )
            strcpy(result, "15! Login first !");
            
        else if (strncmp(input, "login", 5) == 0)
        {      
            //if we have a logged user
            if (strcmp(logged_user, "\0"))      
                strcpy(result, "16! Logout first !");
            else
            {                
                write(pfd[1], input, MAX_INPUT_SIZE); 
                        
                sleep(4);
                
                // if this is de-commented you will see how the big errors handling is done
                // aka how the client reads the errors from the server
                
                /*write(fd, "5error", MAX_RESULT_SIZE);                    
                sleep(3);
                write(fd, "22[server] eroare bagata", MAX_RESULT_SIZE); 
                displayError("[server] eroare bagata"); */
                
                if (read(pfd[0], result, MAX_RESULT_SIZE) == -1)
                {
                    write(fd, "5error", MAX_RESULT_SIZE);                    
                    sleep(5);
                    write(fd, "18[server] read pipe", MAX_RESULT_SIZE); 
                    displayError("[server] read pipe");        
                }
              
                // check if text was read correctly 
                int i;
                if ((i = isReadOk(result)) == 0)
                {
                    write(fd, "5error", MAX_RESULT_SIZE);                       
                    sleep(3);
                    write(fd, "52[server] ! Unsuccesful read from login child! Bye. !", MAX_RESULT_SIZE); 
                    displayError("[server] ! Unsuccesful read from login child! Bye. !");        
                }
                
                // delete the prefix
                char* formattedRes;
                formattedRes = (char *) malloc(MAX_RESULT_SIZE - 1);
                
                strncpy(formattedRes, result + i, strlen(result) - i);
                
                // check if the operation was successful
                if (strcmp(formattedRes, "fail") == 0)
                    sprintf(result, "19! Failed to login !");
                // if there has been an error in the child
                else if (strcmp(formattedRes, "error") == 0)
                {
                    write(fd, "5error", MAX_RESULT_SIZE); 
                    read(pfd[0], result, MAX_RESULT_SIZE); 
                    sleep(2);                            
                    write(fd, result, MAX_RESULT_SIZE); 
                    displayError("[server] ! There has been an error! Bye. !");
                }
                // create the result and set logged-user's value
                else
                {
                    strcpy(logged_user, formattedRes);
                    logged_user[ strlen(logged_user) ]  = '\0';
                    int nr = strlen(logged_user) + 36;
                    sprintf(result, "%dLogin successful. %s is now logged in.", nr, logged_user);
                }
            }                     
        } 
        
        else if (strcmp(input, "get-logged-users") == 0)
        {            
            write(sock_log[1], input, MAX_INPUT_SIZE); 
                              
            sleep(3);
            if (read(sock_log[1], result, MAX_RESULT_SIZE) == -1)
            {
                write(fd, "5error", MAX_RESULT_SIZE);                
                sleep(3);
                write(fd, "25[server] read socket logg", MAX_RESULT_SIZE); 
                displayError("[server] read socket logg");        
            }
              
            // check if text was read correctly  
            int i;
            if ((i = isReadOk(result)) == 0)
            {
                write(fd, "5error", MAX_RESULT_SIZE);                
                sleep(3);
                write(fd, "63[server] read socket logg", MAX_RESULT_SIZE); 
                displayError("[server] ! Unsuccesful read from get-logged-users child! Bye. !");        
            }
                            
            // check if the operation was successful
            if (strcmp(result, "4fail") == 0)
                sprintf(result, "30! Failed to get-logged-users !");
           // if there has been an error in the child
           else if (strcmp(result, "5error") == 0)
           {
                write(fd, "5error", MAX_RESULT_SIZE); 
                read(pfd[0], result, MAX_RESULT_SIZE); 
                sleep(2);                   
                write(fd, result, MAX_RESULT_SIZE); 
                displayError("[server] ! There has been an error Bye. !");
           }
        }
        
        else if (strncmp(input, "get-proc-info", 13) == 0)
        {            
            write(sock_proc[1], input, MAX_INPUT_SIZE);
                              
            sleep(10);
            if (read(sock_proc[1], result, MAX_RESULT_SIZE) == -1)
            {
                write(fd, "5error", MAX_RESULT_SIZE);
                sleep(3);
                write(fd, "23[server] read sock proc", MAX_RESULT_SIZE); 
                displayError("[server] read sock proc");      
            }
            
            // check if text was read correctly  
            int i;
            if ((i = isReadOk(result)) == 0)
            {
                write(fd, "5error", MAX_RESULT_SIZE);                
                sleep(3);
                write(fd, "60[server] ! Unsuccesful read from get-proc-info child! Bye. !", MAX_RESULT_SIZE); 
                displayError("[server] ! Unsuccesful read from get-proc-info child! Bye. !");      
            }
                
            
            // check if the operation was successful
            if (strcmp(result, "4fail") == 0)
                sprintf(result, "27! Failed to get-proc-info !");
            // if there has been an error in the child
            else if (strcmp(result, "5error") == 0)
            {
                write(fd, "5error", MAX_RESULT_SIZE); 
                read(pfd[0], result, MAX_RESULT_SIZE); 
                sleep(2);                   
                write(fd, result, MAX_RESULT_SIZE); 
                displayError("[server] ! There has been an error! Bye. !");   
            }
        }
        
        else if (strcmp(input, "logout") == 0)
        {                 
            strcpy(logged_user, "\0");
            strcpy(result, "18Logout successful."); 
        }
            
        else if (strcmp(input, "quit") == 0)
        {
            // tell children to quit
            if (write(pfd[1], input, MAX_INPUT_SIZE) == -1)
            {
                write(fd, "5error", MAX_RESULT_SIZE);                        
                sleep(3);
                write(fd, "18[server] quit pipe", MAX_RESULT_SIZE);
                displayError("[server] quit pipe");      
            }
            if (write(sock_log[1], input, MAX_INPUT_SIZE) == -1)
            {
                write(fd, "5error", MAX_RESULT_SIZE);
                sleep(3);
                write(fd, "22[server] quit sock log", MAX_RESULT_SIZE);
                displayError("[server] quit sock log");    
            }
            if (write(sock_proc[1], input, MAX_INPUT_SIZE) == -1)
            {
                write(fd, "5error", MAX_RESULT_SIZE);
                sleep(3);
                write(fd, "22[server] quit proc log", MAX_RESULT_SIZE);
                displayError("[server] quit proc log");  
            }
                
            // tell client to quit
            if ((nr = write(fd, "3bye", MAX_RESULT_SIZE)) == -1)
            {
                write(fd, "5error", MAX_RESULT_SIZE);
                sleep(3);
                write(fd, "24[server] write to client", MAX_RESULT_SIZE);
                displayError("[server] write to client"); 
            }
            
            break;
        }
               
            
        if ((nr = write(fd, result, MAX_RESULT_SIZE)) == -1)
        {
            write(fd, "5error", MAX_RESULT_SIZE);
            sleep(3);
            write(fd, "24[server] write to client", MAX_RESULT_SIZE);
            displayError("[server] write to client"); 
        }
                
        printf("The result sent to fifo: %s\n", result);        
        
        sleep(4);
    }
    
    // we received quit
    // close all the communication channels
    close(pfd[0]);
    close(pfd[1]);
    close(sock_log[1]);
    close(sock_proc[1]);
    close(fd);
    
    printf("-- Bye! --\n");
    
    return 0;
}
