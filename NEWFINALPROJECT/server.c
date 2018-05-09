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
#define MAX_MSG_LEN 20  
#define MAX_ROOM_SEATS 9999
#define MAX_CLI_SEATS 99
#define SHARED 0
#define WIDTH_PID 5
#define WIDTH_XXNN 5
#define WIDTH_SEAT 4
#define DELAY()

//utilizar tanto mutex como variavéis de condição

pthread_cond_t  seats_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t  request_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t seats_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t request_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threads_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t threads_cond = PTHREAD_COND_INITIALIZER;
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

sem_t empty, full; //semaphores

struct Request global_current_Request; //the request that will be handled one at a time by the program, the main function receives a request from the FIFO, puts it here if the previous one was answered, and then the threads will take it on and try to reserve a seat.

struct Seat *seats;

int num_room_seats = 0;

int isSeatFree(struct Seat *seats, int seatNum, size_t seatsSize){	

	for(unsigned int i = 0; i < seatsSize; i++){
		if(seats[i].seatId == seatNum && seats[i].occupied == 'n'){
			return 1;
		}
	}

	return 0;
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

void clear_Request_Buffer(){
	global_current_Request.idClient = 0;
	global_current_Request.nrIntendedSeats = 0;
	memset(global_current_Request.idPreferedSeats, 0, MAX_CLI_SEATS);
	global_current_Request.answered = 'n';
}

void *reserveSeat(void *threadId)
{	
	char message[41];	

	//syncing with the start of main
	pthread_mutex_lock(&threads_lock);
		sprintf(message, "\n in thread lock");
		write(STDOUT_FILENO, message, 16);	

		while(global_current_Request.idClient == 0){
			sprintf(message, "\n waiting");
			write(STDOUT_FILENO, message, 9);		
			pthread_cond_wait(&threads_cond, &threads_lock);
		}

		sprintf(message, "\n out thread lock");
		write(STDOUT_FILENO, message, 17);
	
	pthread_mutex_unlock(&threads_lock);

	//taking the actual request from the buffer (leaving it empty)
	//pthread_mutex_lock(&request_lock);
	
		struct Request r1 = global_current_Request;
	
		clear_Request_Buffer();
	
	//pthread_cond_signal(&request_cond);
	//pthread_mutex_unlock(&request_lock);
	
	sprintf(message, "\n executing reserve");
	write(STDOUT_FILENO, message, 19);			
	
	int numValidatedSeats = 0;
	int validatedIds[r1.nrIntendedSeats];
	int clientID = r1.idClient;
			
	for(unsigned int a = 0; a < r1.nrIntendedSeats; a++){
		
		pthread_mutex_lock(&seats_lock);
						
		for(unsigned int i = 0; i < MAX_CLI_SEATS; i++){

			int seatNum = r1.idPreferedSeats[i];
					
			if(seatNum == 0){
				//reached the unused portion of the array.
				break;
			}
			
			sprintf(message, "\n seatNum:%d", seatNum);
			if(seatNum >= 10)			
			write(STDOUT_FILENO, message, 13);
			else
			write(STDOUT_FILENO, message, 12);	
			
			//pthread_mutex_lock(&seats_aux_lock); //closing due to isSeatFree and bookSeat having operations with seats array.
					if(isSeatFree(seats, seatNum, num_room_seats)){
						sprintf(message, "\n booking seat");
						write(STDOUT_FILENO, message, 14);
						bookSeat(seats, seatNum, clientID, num_room_seats);

						pthread_mutex_lock(&seats_aux_lock);
							validatedIds[numValidatedSeats] = seatNum;
							numValidatedSeats++;
						pthread_mutex_unlock(&seats_aux_lock);

			//pthread_mutex_unlock(&seats_aux_lock);
						break;	
					}
		}
		
		pthread_mutex_unlock(&seats_lock);
	}
		
	sprintf(message, "\n valid:%d intended:%d", numValidatedSeats, r1.nrIntendedSeats);
	write(STDOUT_FILENO, message, 20);		

	// - In case you couldn't validate every seat the costumer wanted, then free all of them.
	if(numValidatedSeats < r1.nrIntendedSeats){
		for(unsigned int a = 0; a < numValidatedSeats; a++){
			pthread_mutex_lock(&seats_aux_lock);
				sprintf(message, "\n free seat");
				write(STDOUT_FILENO, message, 11);
				freeSeat(seats, validatedIds[a], num_room_seats);
			pthread_mutex_unlock(&seats_aux_lock);
		}
	}	
			
	for(unsigned int a = 0; a < numValidatedSeats; a++){
		sprintf(message, "\n validated seat %d", validatedIds[a]);
		if(validatedIds[a] >= 10)
		write(STDOUT_FILENO, message, 20);
		else
		write(STDOUT_FILENO, message, 18);
	}
	
	r1.answered = 'y';
	
	//<function for sending the request here>	

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

	int num_occupied = 0;

	for(int i=0; i < num_room_seats;i++){
		if(seats[i].occupied == 'y'){
			num_occupied++;
		}
	}

	if(num_occupied == num_room_seats){
		printf("Room full.\n");
		return -6;
	}
	
	//valid request
	return 0;
}

int main(int argc, char* argv[]){

	int fd, n;
	
	//1. Checking Input	
	if(argc < 4){
		printf("Not enough arguments.\n");
		printf("usage: server <num_room_seats> <num_ticket_offices> <open_time>\n"); 
		exit(1);
	}

	//2. Getting args
	num_room_seats = atoi(argv[1]);
    	int num_ticket_offices = atoi(argv[2]);
	int open_time = atoi(argv[3]);

	// - Initializing Request Buffer:
	clear_Request_Buffer();

	// - Initializing Seats array and Seat objects
	seats = (struct Seat*)malloc(sizeof(struct Seat)*num_room_seats);
	
	for(unsigned int i = 0; i < num_room_seats; i++){
		struct Seat currentSeat;

		currentSeat.seatId = (i+1);
		currentSeat.clientId = 0;
		currentSeat.occupied = 'n';
		
		seats[i] = currentSeat;
	}

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
		write(STDOUT_FILENO, "\n Creating thread", 20); 
		thrArg[t-1] = t; 
		rc = pthread_create(&tid[t-1], NULL, reserveSeat, &thrArg[t-1]); 

		if (rc) { 
			printf("ERROR; return code from pthread_create() is %d\n", rc); 
			exit(1); 
    		} 
  	} 

	if ((fd=open("/tmp/requests",O_RDONLY)) !=-1) printf("FIFO '/tmp/requests' openned in READONLY mode\n"); 

	do { 
		// - Reading from FIFO, first into a "dummy variable" to check if the input is even valid or not
		struct Request tempRequest;		

		n=read(fd,&tempRequest,sizeof(struct Request)); 
		
		if (n>0) printf("%d client request has arrived\n", tempRequest.idClient);
		
		// - Validating
		if(validate_request_parameters(tempRequest, num_room_seats) != 0){
			break;
		}

		// - Placing the request in the buffer, locking it between critical zone.		 
		//pthread_mutex_lock(&request_lock);
  		
			global_current_Request.idClient = tempRequest.idClient;
			global_current_Request.nrIntendedSeats = tempRequest.nrIntendedSeats;

			for(unsigned int i = 0; i < MAX_CLI_SEATS; i++){
				global_current_Request.idPreferedSeats[i] = tempRequest.idPreferedSeats[i];
			}

			global_current_Request.answered = tempRequest.answered;

			//syncing with threads, letting them execute
			pthread_mutex_lock(&threads_lock);		
				pthread_cond_signal(&threads_cond);
			pthread_mutex_unlock(&threads_lock);
	
			// - Wait until the request has been removed by one of the threads.
			while(global_current_Request.nrIntendedSeats != 0){		
				//pthread_cond_wait(&request_cond, &request_lock);
				sleep(1);
			}
	
			//placing the request buffer back to all zeroes once he leaves the wait (request taken by threads)
			clear_Request_Buffer();
	
		//pthread_mutex_unlock(&request_lock);

	} while (global_current_Request.idClient != -1); 
  	
	for(int k=0; k <= num_ticket_offices; k++) {
		pthread_join(tid[k], NULL); //wait for threads to be done
	}	

	close(fd);

	if (unlink("/tmp/requests")<0) printf("Error when destroying FIFO '/tmp/requests'\n"); 
	else printf("FIFO '/tmp/requests' has been destroyed\n"); 

	pthread_exit(NULL); 

	return 0;
}
