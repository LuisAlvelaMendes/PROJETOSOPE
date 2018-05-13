#include "server.h"

struct Request global_current_Request; //the request that will be handled one at a time by the program, the main function receives a request from the FIFO, puts it here if the previous one was answered, and then the threads will take it on and try to reserve a seat.

struct Seat *seats;

struct Answer answer;

int num_room_seats = 0;

int close_thread = 0; //tells threads when to close

int MAX_OVERFLOW_CHARACTERS = 0; //auxiliary variable for writing the file when - MAX is triggered

int isSeatFree(struct Seat *seats, int seatNum, size_t seatsSize){	

	for(unsigned int i = 0; i < seatsSize; i++){
		if(seats[i].seatId == seatNum && seats[i].occupied == 'n'){
			return 1;
		}
	}

	pthread_mutex_lock(&delay_lock);
		DELAY();
	pthread_mutex_unlock(&delay_lock);

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

	pthread_mutex_lock(&delay_lock);
		DELAY();
	pthread_mutex_unlock(&delay_lock);
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

	pthread_mutex_lock(&delay_lock);
		DELAY();
	pthread_mutex_unlock(&delay_lock);
}

void clear_Request_Buffer(){
	global_current_Request.idClient = 0;
	global_current_Request.nrIntendedSeats = 0;
	memset(global_current_Request.idPreferedSeats, 0, MAX_CLI_SEATS);
	global_current_Request.answered = 'n';
}

void initialize_Answer(){
	answer.error_flag = 0;
	memset(answer.reservedSeats, 0, MAX_CLI_SEATS + 1);
}

int countDigits(int num){
	long long n;
	int count = 0;

	n = (long long) num;	
	
	while(n != 0){
		n /= 10;
		++count;
    	}
	
	return count;
}

int validate_request_parameters(struct Request r1, int num_room_seats){
	
	if(r1.nrIntendedSeats > MAX_CLI_SEATS || r1.nrIntendedSeats < 1){
		printf("Invalid number of intended seats.\n");
		MAX_OVERFLOW_CHARACTERS = countDigits(r1.nrIntendedSeats) - 2;
		return -1;
	}
	
	// - Checking the amount of ids is equal to the number of intended seats, and if each ID is valid.

	int countIDs = 0;
	
	for(unsigned int i = 0; i < r1.nrIntendedSeats; i++){	
		if(r1.idPreferedSeats[i] == 0){
			// - PreferedSeats will be all zeros from the point where you no longer have any more seat IDs.		
			break;
		} else {
			countIDs++;
		}

		if(r1.idPreferedSeats[i] > num_room_seats || r1.idPreferedSeats[i] < 0){
			// - Checking identifiers for each seat at the same time
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
	
	// - Valid request (according to these parameters)
	return 0;
}

void write_TO_OPEN(int threadId){
	int fd;
	
	fd = open("slog.txt",O_WRONLY|O_APPEND,0600);

	if (fd == -1) { 
  		perror("slog.txt"); 
		return; 
 	}

	char message[MAX_MSG_LEN];	
	
	if(threadId < 10){
		sprintf(message, "\n0%d-OPEN", threadId);
		write(fd, message, 8);
	}

	else {
		sprintf(message, "\n%d-OPEN", threadId);
		write(fd, message, 8);
	} 

	close(fd);
}

void write_TO_CLOSED(int threadId){
	int fd;
	
	fd = open("slog.txt",O_WRONLY|O_APPEND,0600);

	if (fd == -1) { 
  		perror("slog.txt"); 
		return; 
 	}

	char message[MAX_MSG_LEN];	
	
	if(threadId < 10){
		sprintf(message, "\n0%d-CLOSED", threadId);
		write(fd, message, 10);
	}

	else {
		sprintf(message, "\n%d-CLOSED", threadId);
		write(fd, message, 10);
	} 

	close(fd);
}

void write_to_sbook(int seatID){

	int file;
	
	file = open("sbook.txt",O_WRONLY|O_APPEND,0600);

	if (file < 0) { 
  		perror("sbook.txt"); 
		return; 
 	}

	char message[MAX_MSG_LEN];
	sprintf(message, "%04d\n", seatID);
	write(file, message, 5);

	close(file);
}


char* parse_preference_into_string(int preference){
	char* preferenceString = malloc(20*sizeof(char));	
	
	// - Each block will be called a "SECTION"

	if(preference <= 10){
		sprintf(preferenceString, "000%d ", preference);
	}

	if(preference >= 10 && preference < 100){
		sprintf(preferenceString, "00%d ", preference);
	}

	if(preference >= 100 && preference < 1000){
		sprintf(preferenceString, "0%d ", preference);
	}

	return preferenceString;
}

char* get_ending_note(int identifier){

	char* tag = malloc(10*sizeof(char)); 

	if(identifier < 0){
		if(identifier == -1) strcat(tag, "MAX");
		if(identifier == -2) strcat(tag, "NST");
		if(identifier == -3) strcat(tag, "IID");
		if(identifier == -4) strcat(tag, "ERR");
		if(identifier == -5) strcat(tag, "NAV");
		if(identifier == -6) strcat(tag, "FUL");
	}

	else {
		strcat(tag, parse_preference_into_string(identifier));	
	}

	return tag;
}

void write_TO_CLIID_NT(struct Request r1, int threadId, int validatedIds[], int numValidatedSeats, int error_flag){
	int fd;
	
	fd = open("slog.txt",O_WRONLY|O_APPEND,0600);

	if (fd == -1) { 
  		perror("slog.txt"); 
		return; 
 	}

	char message[MAX_CLI_SEATS*8];	
		
	// - Formating the "HEADER" (first part of each line)

	if(threadId < 10 && r1.nrIntendedSeats < 10){
		sprintf(message, "\n0%d-%d-0%d: ", threadId, r1.idClient, r1.nrIntendedSeats);
	}

		
	if(threadId >= 10 && r1.nrIntendedSeats < 10){
		sprintf(message, "\n%d-%d-0%d: ", threadId, r1.idClient, r1.nrIntendedSeats);
	}

			
	if(threadId < 10 && r1.nrIntendedSeats >= 10){
		sprintf(message, "\n0%d-%d-%d: ", threadId, r1.idClient, r1.nrIntendedSeats);
	}

		
	if(threadId >= 10 && r1.nrIntendedSeats >= 10){
		sprintf(message, "\n%d-%d-%d: ", threadId, r1.idClient, r1.nrIntendedSeats);
	}

	// - Adding the bulk of the information "header: <section section section ...>"

	int count_preference = 0;
	
	for(unsigned int i = 0; i < MAX_CLI_SEATS; i++){
	
		if(r1.idPreferedSeats[i] != 0) count_preference++;
		else break;

		strcat(message, parse_preference_into_string(r1.idPreferedSeats[i]));
	}

	// - Add the final note at the end of the line

	strcat(message, "    - "); // END_NOTE_HEADER (first part of the ending note)

	if(error_flag == 0){

		for(unsigned int i = 0; i < numValidatedSeats; i++){
			strcat(message, get_ending_note(validatedIds[i]));
		}
	
		int numCharsToWrite = (SIZE_OF_HEADER + (SIZE_OF_SECTION*count_preference)) + (SIZE_OF_END_NOTE_HEADER + (numValidatedSeats*SIZE_OF_SECTION));

		write(fd, message, numCharsToWrite);
	}

	else{
		if(error_flag != -1){
			strcat(message, get_ending_note(error_flag));
	
			int numCharsToWrite = SIZE_OF_HEADER + (SIZE_OF_SECTION*count_preference) + SIZE_OF_END_NOTE_HEADER + SIZE_OF_ERROR_TAG + 1;

			write(fd, message, numCharsToWrite);
		}

		else {
			strcat(message, get_ending_note(error_flag));
	
			int numCharsToWrite = SIZE_OF_HEADER + (SIZE_OF_SECTION*count_preference) + SIZE_OF_END_NOTE_HEADER + SIZE_OF_ERROR_TAG + MAX_OVERFLOW_CHARACTERS + 1;

			write(fd, message, numCharsToWrite);
		}	
	}

	close(fd);
}

void *reserveSeat(void *threadId)
{		
	//int error_flag = 0; // - IN CASE THE REQUEST CANNOT BE ANSWERED, THIS WILL BE SET TO SOMETHING NEGATIVE.
	int fd, n;
	initialize_Answer();

	char message[41];

	int intThreadId = *((int *) threadId);

	// - Syncing with the start of main

	pthread_mutex_lock(&threads_lock);

		sprintf(message, "\n in thread lock");
		write(STDOUT_FILENO, message, 16);

		write_TO_OPEN(intThreadId);	

		while(global_current_Request.idClient == 0 && close_thread == 0){
			sprintf(message, "\n waiting");
			write(STDOUT_FILENO, message, 9);
			pthread_cond_wait(&threads_cond, &threads_lock);
		}

		sprintf(message, "\n out thread lock");
		write(STDOUT_FILENO, message, 17);
	
	pthread_mutex_unlock(&threads_lock);
	
	// - In case it's time to close the thread, do so now

	if(close_thread == 1){
		write_TO_CLOSED(intThreadId);
		return NULL;
	}

	// - Taking the actual request from the buffer (leaving it empty) and validating

	struct Request r1 = global_current_Request;

	char fifoname[20];

	// - Getting the FIFO's name	
	sprintf(fifoname, "/tmp/ans%d", r1.idClient);

	if ((fd=open(fifoname,O_WRONLY)) !=-1) printf("FIFO %s openned in WRITEONLY mode\n", fifoname);

	clear_Request_Buffer();

	answer.error_flag = validate_request_parameters(r1, num_room_seats);
		
	int numValidatedSeats = 0;
	int validatedIds[r1.nrIntendedSeats];
	int clientID = r1.idClient;

	// - Check if request is valid before needing any operations

	if(answer.error_flag < 0){
		pthread_mutex_lock(&writing_lock);
			write_TO_CLIID_NT(r1, intThreadId, validatedIds, numValidatedSeats, answer.error_flag);
		pthread_mutex_unlock(&writing_lock);

		//write answer to fifo
		n=write(fd,&answer.error_flag,sizeof(int)); 
		if (n>0) printf("%d answer was sent to the client.\n", answer.error_flag); 
	}	
	
	sprintf(message, "\n executing reserve");
	write(STDOUT_FILENO, message, 19);	
	
	// - Actually starting reserving seats		
			
	if(answer.error_flag == 0){
		for(unsigned int a = 0; a < r1.nrIntendedSeats; a++){
			
			pthread_mutex_lock(&seats_lock);
							
			for(unsigned int i = 0; i < MAX_CLI_SEATS; i++){

				int seatNum = r1.idPreferedSeats[i];
						
				if(seatNum == 0){
					//- Reached the unused portion of the array.
					break;
				}
				
				sprintf(message, "\n seatNum:%d", seatNum);
				if(seatNum >= 10)			
				write(STDOUT_FILENO, message, 13);
				else
				write(STDOUT_FILENO, message, 12);	
				
				
				if(isSeatFree(seats, seatNum, num_room_seats)){
					sprintf(message, "\n booking seat");
					write(STDOUT_FILENO, message, 14);
					bookSeat(seats, seatNum, clientID, num_room_seats);

					pthread_mutex_lock(&seats_aux_lock);
						validatedIds[numValidatedSeats] = seatNum;
						numValidatedSeats++;
					pthread_mutex_unlock(&seats_aux_lock);

					break;	
				}
			}
			
			pthread_mutex_unlock(&seats_lock);
		}
			
		sprintf(message, "\n valid:%d intended:%d\n", numValidatedSeats, r1.nrIntendedSeats);
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
				
			answer.error_flag = -5; // - At least one of the requests was not valid.

			//write answer to fifo
			n=write(fd,&answer.error_flag,sizeof(int)); 
			if (n>0) printf("%d answer was sent to the client.\n", answer.error_flag);
			
			pthread_mutex_lock(&writing_lock);
				write_TO_CLIID_NT(r1, intThreadId, validatedIds, numValidatedSeats, answer.error_flag);
			pthread_mutex_unlock(&writing_lock);
		}
		else{ //writing the reserved seats to sbook and updating answer
			answer.reservedSeats[0] = numValidatedSeats;
			for(unsigned int a = 0; a < numValidatedSeats; a++){
				write_to_sbook(validatedIds[a]);
				answer.reservedSeats[a+1] = validatedIds[a];
			}
		}

		if(answer.error_flag == 0){
			pthread_mutex_lock(&writing_lock);
				write_TO_CLIID_NT(r1, intThreadId, validatedIds, numValidatedSeats, answer.error_flag);
			pthread_mutex_unlock(&writing_lock);

			//write answer to fifo
			n=write(fd,answer.reservedSeats,(MAX_CLI_SEATS + 1)*sizeof(int)); 
			if (n>0){
				 printf("Answer was sent to the client: "); 
				 for(unsigned int a = 0; a < numValidatedSeats + 1; a++){
					printf("%d ", answer.reservedSeats[a]);
				 }
				 printf("\n");
			}
		}	
				
		for(unsigned int a = 0; a < numValidatedSeats; a++){
			sprintf(message, "\n validated seat %d", validatedIds[a]);
			if(validatedIds[a] >= 10) write(STDOUT_FILENO, message, 20);
			else write(STDOUT_FILENO, message, 18);
		}
	}	
	
	close(fd);

	r1.answered = 'y';
	
	//<function for sending the request here>

	pthread_mutex_lock(&end_ticketer_lock);
		
		while(!close_thread){
			pthread_cond_wait(&end_ticketer_cond, &end_ticketer_lock);
		}

		write_TO_CLOSED(intThreadId);

	pthread_mutex_unlock(&end_ticketer_lock);

	return NULL;
}

int main(int argc, char* argv[]){


	int fd, n, file, sbook;
	
	//1. Checking Input
	
	if(argc < 4){
		printf("Not enough arguments.\n");
		printf("usage: server <num_room_seats> <num_ticket_offices> <open_time>\n"); 
		exit(1);
	}

	// - Making the logfile or cleaning it.

	file = open("slog.txt",O_CREAT|O_TRUNC,0600);
	
	if (file == -1) { 
  		perror("slog.txt"); 
		return 0; 
 	}
	
	close(file);

	// - Creating the server bookings file
	
	sbook = open("sbook.txt",O_CREAT|O_TRUNC,0600);
	
	if (sbook < 0) { 
  		perror("sbook.txt");
		return 0; 
 	}
	
	close(sbook);

	//2. Getting args

	num_room_seats = atoi(argv[1]);
    	int num_ticket_offices = atoi(argv[2]);
	int open_time = atoi(argv[3]);

	// - Time constraints due to open_time

	time_t endwait = 0;   	
	time_t start = time(NULL);
    	time_t seconds = open_time; // end loop after this time has elapsed

	endwait = start + seconds;

	printf("Tickets opening at: %s", ctime(&start));

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
	
	//5. Fifo operations
	
	if ((fd=open("/tmp/requests",O_RDONLY)) !=-1) printf("FIFO '/tmp/requests' openned in READONLY mode\n"); 

	do { 
		// - Reading from FIFO

		struct Request tempRequest;		

		n=read(fd,&tempRequest,sizeof(struct Request)); 
		
		if (n>0) printf("\n %d client request has arrived\n", tempRequest.idClient);
		else {
			// - Even if no requests are taken, time will pass ..
			sleep(1);
        		start = time(NULL);
			continue;
		}

		// - Placing the request in the buffer

		global_current_Request = tempRequest;

		// - Syncing with threads, letting them execute

		pthread_mutex_lock(&threads_lock);		
			pthread_cond_signal(&threads_cond);
		pthread_mutex_unlock(&threads_lock);
	
		// - Wait until the request has been removed by one of the threads.

		while(global_current_Request.nrIntendedSeats != 0){
			sleep(1);
		}

		sleep(1); // - Sleep placed due to passage of time
        	start = time(NULL);

	} while (start < endwait);

	printf("\n Tickets closed at %s", ctime(&endwait));

	//6. Terminating

	// - Telling the threads it's time to close

	pthread_mutex_lock(&end_ticketer_lock);
		close_thread = 1;
		pthread_cond_broadcast(&end_ticketer_cond);
	pthread_mutex_unlock(&end_ticketer_lock);

	// - Closing all threads and cleaning them up

	for(int k = 0; k < num_ticket_offices; k++){
		pthread_mutex_lock(&threads_lock);		
			pthread_cond_broadcast(&threads_cond);
		pthread_mutex_unlock(&threads_lock);
	
		pthread_join(tid[k], NULL);
	}		

	close(fd);

	if (unlink("/tmp/requests")<0) printf("Error when destroying FIFO '/tmp/requests'\n"); 
	else printf("FIFO '/tmp/requests' has been destroyed\n"); 

	return 0;
}
