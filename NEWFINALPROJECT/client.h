#include <stdio.h>  
#include <stdlib.h>  
#include <pthread.h> 
#include <string.h> 
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <sys/file.h>  
#include <pthread.h> 
#include <errno.h>  
#define MAX_MSG_LEN 20   
#define MAX_ROOM_SEATS 9999 
#define MAX_CLI_SEATS 99 
#define WIDTH_PID 5 
#define WIDTH_XXNN 5 
#define WIDTH_SEAT 4 
#define DELAY() 
 
//utilizar tanto mutex como variavéis de condição 
 
pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER; 
 
struct Request 
{ 
  int idClient; 
  int nrIntendedSeats; 
  int idPreferedSeats[MAX_CLI_SEATS]; 
  char answered; //tells you whether it has been answered or not 'y' means answered and 'n' means unanswered. 
};
