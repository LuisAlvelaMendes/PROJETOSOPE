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

int main(int argc, char* argv[]){

	int fd, n;
	char str[MAX_MSG_LEN]; 
	struct Request request_1;
	
	//1. Checking Input	
	if(argc < 4){
		printf("Not enough arguments.\n");
		printf("usage: client <time_out> <num_wanted_seats> <pref_seat_list>\n"); 
		exit(1);
	}

	//2. Getting args
	int time_out = atoi(argv[1]);
    	int num_wanted_seats = atoi(argv[2]);
	int pref_seat_list [MAX_CLI_SEATS];

	// - Parsing preferences
	char* token = malloc(10*sizeof(char));
	token = strtok(argv[3]," ");
	int i = 0;
	
        while(token != NULL) {
		pref_seat_list[i] = atoi(token);
                token = strtok(NULL," ");
		i++;
	}	

	char fifoname[20];

	// - Getting the FIFO's name	
	sprintf(fifoname, "/tmp/ans%d", getpid());

	// - Initializing Request
	request_1.idClient = getpid();
	request_1.nrIntendedSeats = num_wanted_seats;
	memcpy(request_1.idPreferedSeats, pref_seat_list, MAX_CLI_SEATS);
	request_1.answered = 'n';

	//3. Making Fifo
	if (mkfifo(fifoname,0660)<0) 
		if (errno==EEXIST) printf("FIFO already exists\n"); 
		else printf("Can't create FIFO\n"); 
	else printf("FIFO %s sucessfully created\n", fifoname); 

	//4. Send a Request
	if ((fd=open("/tmp/requests",O_WRONLY)) !=-1) printf("FIFO '/tmp/requests' openned in WRITEONLY mode\n"); 

	/*do { 
		//SEND REQUEST CODE
    		n=write(fd,str,MAX_MSG_LEN); 
		if (n>0) printf("%s has arrived\n",str);
	} while (strcmp(str,"SHUTDOWN")!=0);*/ 
  	
	close(fd); 

	return 0;
}
