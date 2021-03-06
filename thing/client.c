#include "client.h"

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

int main(int argc, char* argv[]){

	int fd, n;
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
	int pref_seat_list [MAX_CLI_SEATS] = {0};
	int zeros[MAX_CLI_SEATS] = {0};

	// - Parsing preferences

	char* token = malloc(10*sizeof(char));
	token = strtok(argv[3]," ");
	int countingIDs = 0;
	
        while(token != NULL) {
		pref_seat_list[countingIDs] = atoi(token);
                token = strtok(NULL," ");
		countingIDs++;
	}

	// - Making sure from the point where we no longer have IDs it'll be all zeros, that portion of the array is unused.
	for(unsigned int i = countingIDs; i < MAX_CLI_SEATS; i++){
		pref_seat_list[i] = 0;
	}

	char fifoname[20];

	// - Getting the FIFO's name 
	
	int pid = getpid();

	int digits = countDigits(pid);

	if(digits < WIDTH_PID){
		if(digits == 4){
			pid = pid*10;
		}

		if(digits == 3){
			pid = pid*100;
		}

		if(digits == 2){
			pid = pid*1000;
		}

		if(digits == 1){
			pid = pid*10000;
		}
	}
	
	sprintf(fifoname, "/tmp/ans%d", pid);

	// - Initializing Request

	request_1.idClient = pid;
	
	request_1.nrIntendedSeats = num_wanted_seats;
	
	// - Initializing the request array at all zeros just in case
	for(unsigned int i = 0; i < MAX_CLI_SEATS; i++){
		request_1.idPreferedSeats[i] = zeros[i];
	}
	
	// - Passing over the intended elements
	for(unsigned int i = 0; i < MAX_CLI_SEATS; i++){
		request_1.idPreferedSeats[i] = pref_seat_list[i];
	}

	request_1.answered = 'n';

	//3. Making Fifo for ANSWER

	if (mkfifo(fifoname,0660)<0) 
		if (errno==EEXIST) printf("FIFO already exists\n"); 
		else printf("Can't create FIFO\n"); 
	else printf("FIFO %s sucessfully created\n", fifoname); 

	//4. Send a Request

	if ((fd=open("/tmp/requests",O_WRONLY)) !=-1) printf("FIFO '/tmp/requests' openned in WRITEONLY mode\n"); 

	// - Write to FIFO

    	n=write(fd,&request_1,sizeof(struct Request)); 

	if (n>0) printf("%d request has been sent.\n", request_1.idClient); 

	sleep(15);

	struct Request nextValid;

	nextValid.idClient = 40000;
	nextValid.nrIntendedSeats = 4;
	nextValid.idPreferedSeats[0] = 100;
	nextValid.idPreferedSeats[1] = 20;
	nextValid.idPreferedSeats[2] = 10;
	nextValid.idPreferedSeats[3] = 8;
	nextValid.answered = 'n';

	n=write(fd,&nextValid,sizeof(struct Request)); 

	if (n>0) printf("%d request has been sent.\n", nextValid.idClient);

	//-- WAIT FOR RESPONSE HERE --
  	
	close(fd); 

	return 0;
}
