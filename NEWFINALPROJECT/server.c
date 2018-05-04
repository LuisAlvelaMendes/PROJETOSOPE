#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/file.h> 
#define MAX_ROOM_SEATS 9999
#define MAX_CLI_SEATS 99
#define WIDTH_PID 5
#define WIDTH_XXNN 5
#define WIDTH_SEAT 4


struct Request
{
	int idClient;
	int nrIntendedSeats;
	int idPreferedSeats[MAX_CLI_SEATS];
	int answered; //tells you whether it has been answered or not
};

struct Request global_current_Request; //the request that will be handled one at a time by the program, the main function receives a request from the FIFO, puts it here if the previous one was answered, and then the threads will take it on and try to reserve a seat.

void *reserveSeat(void *threadId)
{
	pthread_exit(NULL);
}

int main(int argc, char* argv[]){
	
	//1. Checking Input	
	if(argc < 4){
		printf("Not enough arguments.\n");
		printf("usage: server <num_room_seats> <num_ticket_offices> <open_time>\n"); 
		exit(1);
	}

	//2. Getting args
	int num_room_seats = atoi(argv[1]);
    	int num_ticket_offices = atoi(argv[2]);
	int open_time = atoi(argv[3]);

	//3. Making Fifo
	mkfifo("/tmp/requests", 0666);

	//4. Creating threads
	pthread_t tid[num_ticket_offices]; 
	int rc, t; 
	int thrArg[num_ticket_offices]; 

	for (t=1; t<= num_ticket_offices; t++){ 
		printf("Creating thread %d\n", t); 
		thrArg[t-1] = t; 
		rc = pthread_create(&tid[t-1], NULL, reserveSeat, &thrArg[t-1]); 

		if (rc) { 
			printf("ERROR; return code from pthread_create() is %d\n", rc); 
			exit(1); 
    		} 
  	} 
  
	pthread_exit(NULL); 

	return 0;
}
