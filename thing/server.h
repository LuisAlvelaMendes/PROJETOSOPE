#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/file.h> 
#include <pthread.h>
#include <semaphore.h>
#include <errno.h> 
#include <time.h>
#define MAX_MSG_LEN 20  
#define MAX_ROOM_SEATS 9999
#define MAX_CLI_SEATS 99
#define WIDTH_PID 5
#define WIDTH_XXNN 5
#define WIDTH_SEAT 4
#define SIZE_OF_HEADER 13
#define SIZE_OF_SECTION 5
#define SIZE_OF_ERROR_TAG 3
#define SIZE_OF_END_NOTE_HEADER 6
#define DELAY() sleep(0.5)

// - Utilizing mutex as well as condition variables
pthread_mutex_t seats_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threads_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t end_ticketer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t end_ticketer_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t delay_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t threads_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t writing_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t seats_aux_lock = PTHREAD_MUTEX_INITIALIZER;

struct Seat
{
	int seatId;
	int clientId;
	char occupied; // 'y' means occupied, 'n' means free.
};

struct Request
{
	int idClient;
	int nrIntendedSeats;
	int idPreferedSeats[MAX_CLI_SEATS];
	char answered; //tells you whether it has been answered or not 'y' means answered and 'n' means unanswered.
};
