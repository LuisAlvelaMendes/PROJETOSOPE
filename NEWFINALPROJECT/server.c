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

struct Request global_current_Request; //the request that will be handled one at a time by the program, the main function receives a request from the FIFO, puts it here if the previous one was answered, and then the threads will take it on and try to reserve a seat.

struct Seat *seats;

int isSeatFree(struct Seat *seats, int seatNum, size_t seatsSize){	
	
	for(unsigned int i = 0; i < seatsSize; i++){
		if(seats[i].seatId == seatNum && seats[i].occupied == 'n'){
			return 0;
		}
	}

	return 1;
}

void bookSeat(struct Seat *seats, int seatNum, int clientId, size_t seatsSize){

	for(unsigned int i = 0; i < seatsSize; i++){
		if(seats[i].seatId == seatNum){
			seats[i].clientId = clientId;
			seats[i].occupied = 'y';
			return;
		}
	}

}

void freeSeat(struct Seat *seats, int seatNum, size_t seatsSize){
 //while answering the request, goes through the prefered seat IDs array every time and if it can't book any of them at one point, frees everything, the request is invalid.

	for(unsigned int i = 0; i < seatsSize; i++){
		if(seats[i].seatId == seatNum){
			seats[i].clientId = 0;
			seats[i].occupied = 'n';
			return;
		}
	}

}


void *reserveSeat(void *threadId)
{
	pthread_mutex_lock(&buffer_lock);
	
	
	pthread_mutex_unlock(&buffer_lock);
	pthread_exit(NULL);
}

int validate_request_parameters(struct Request r1, int num_room_seats){
	
	if(r1.nrIntendedSeats > MAX_CLI_SEATS || r1.nrIntendedSeats < 1){
		printf("Invalid number of intended seats.\n");
		return -1;
	}
	
	//checking the amount of ids is equal to the number of intended seats, and if each ID is valid.

	int countIDs = 0;
	
	for(unsigned int i = 0; i < r1.nrIntendedSeats; i++){	
		if(r1.idPreferedSeats[i] == 0){
			//preferedSeats will be all zeros from the point where you no longer have any more seat IDs.		
			break;
		} else {
			countIDs++;
		}

		if(r1.idPreferedSeats[i] > num_room_seats || r1.idPreferedSeats[i] < 0){
			//checking identifiers for each seat at the same time
			printf("Invalid seat identifier %d.\n", r1.idPreferedSeats[i]);
			return -3;
		}
	}

	if(countIDs < r1.nrIntendedSeats || countIDs > MAX_CLI_SEATS){
		printf("Invalid size for prefered seats.\n");		
		return -2;
	}

	if(r1.answered != 'y' && r1.answered != 'n'){
		printf("Invalid answer character '%c'\n", r1.answered);
		return -4;
	}
}

int main(int argc, char* argv[]){

	int fd, n;
	char str[MAX_MSG_LEN]; 
	
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

	// - Initializing Request Buffer:
	global_current_Request.idClient = 0;
	global_current_Request.nrIntendedSeats = 0;
	memset(global_current_Request.idPreferedSeats, 0, MAX_CLI_SEATS);
	global_current_Request.answered = 'n';

	// - Initializing Seats
	seats = (struct Seat*)malloc(sizeof(struct Seat)*num_room_seats);

	//3. Making Fifo
	if (mkfifo("/tmp/requests",0660)<0) 
		if (errno==EEXIST) printf("FIFO '/tmp/requests' already exists\n"); 
		else printf("Can't create FIFO\n"); 
	else printf("FIFO '/tmp/requests' sucessfully created\n"); 

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

	if ((fd=open("/tmp/requests",O_RDONLY)) !=-1) printf("FIFO '/tmp/requests' openned in READONLY mode\n"); 

	do { 
		//reading from FIFO    		
		n=read(fd,&global_current_Request,sizeof(struct Request)); 
		
		if (n>0) printf("%d client request has arrived\n", global_current_Request.idClient);
		
		//checking if what was read has valid parameters
		validate_request_parameters(global_current_Request, num_room_seats);

	} while (global_current_Request.idClient != -1); 
  	
	close(fd); 

	if (unlink("/tmp/requests")<0) printf("Error when destroying FIFO '/tmp/requests'\n"); 
	else printf("FIFO '/tmp/requests' has been destroyed\n"); 

	pthread_exit(NULL); 

	return 0;
}
