/* Scrieti un program care sa simuleze comanda: cat prog.c | grep "include" > prog.c , folosind fifo-uri si primitiva dup. */

#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_NAME "MyTest_FIFO"


int main ()
{          
  int fd; // descriptorul necesar pu fifo       
  /* cream fifo-ul */
  if (mknod(FIFO_NAME, S_IFIFO | 0666, 0) == -1)         
  {
      perror("fifo");
      exit (1);
  } 
  
  /* cream primul copil */  
  switch (fork ())              
  {
    case -1:
      perror("fork 1");
      exit (2);
      
    case 0:  /* copilul */
      fd = open(FIFO_NAME, O_WRONLY);
      
      /* duplicam scrierea in fifo la iesirea standard (1) */
      close (1);
      if (dup (fd) != 1)    
      {
          perror("dup 1");
          exit (3);
      }
             
      close (fd);
      
      execlp ("cat", "cat", "prog.c", NULL);
      fprintf (stderr, "exec 1\n");
      exit (4);
  }
  /* cream al doilea copil */  
  switch (fork ())
  {
    case -1:
      perror("fork 2");
      exit (5);
      
    case 0: /* copilul */
      fd = open(FIFO_NAME, O_RDONLY); 
      
      /* duplicam citirea din fifo care va fi asociata intrarii standard */      
      close (0);
      if (dup (fd) != 0)
      {
          perror("dup 2");
          exit (6);
      }
      
      int progfd = open("prog.c", O_WRONLY);
      
      close (1);      
      if (dup (progfd) != 1)    
      {
          perror("dup 3");
          exit (7);
      }
      
      /* descriptorii pot fi inchisi */
      close (fd);
      close(progfd);
      
      execlp ("grep", "grep", "\"include\"", NULL);
      perror("exec 2");
      exit (8);
    }
    
  /* parintele */  
  return 0;
} 
