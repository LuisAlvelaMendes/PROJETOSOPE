#define _GNU_SOURCE
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <unistd.h>
#include<string.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h> 
#include <ctype.h>

#define MAX_LINE_LENGTH 100

//INFO WILL BE THE ARRAY WE WILL PASS BETWEEN FUNCTIONS


 int global_isThereL = 0;
 int global_isThereC = 0;
 int global_isThereN = 0;
 int global_isThereR = 0;
 int global_isThereI = 0;
 int global_isThereW = 0;

//counts how many options there are
int countOptions(char* argv[], int argc){

 int counter = 0;

 for(int i = 1; i < argc; i++) {
  
  if(strcmp(argv[i],"-r") == 0){
    global_isThereR = 1;
    counter++;
  }

  if(strcmp(argv[i],"-l") == 0){
     global_isThereL = 1;
     counter++;
  }

  if(strcmp(argv[i],"-n") == 0){
    global_isThereN = 1;
    counter++; 
  }

  if(strcmp(argv[i],"-c") == 0){
    global_isThereC = 1;
    counter++;
  }

  if(strcmp(argv[i],"-i") == 0){ 
     global_isThereI = 1;
     counter++;
  }

  if(strcmp(argv[i],"-w") == 0){
     global_isThereW = 1;
     counter++; 
  } 
 
 }

 return counter;
}

//counts how many files there are
int countFiles(char* argv[], int argc){

 int counter = 0;

 //the index for the first file will begin at the number of options +2. (+1 for the pattern, +1 for the first file)

 for(int i = countOptions(argv, argc)+2; i < argc; i++) {
    counter++;
 }

 return counter;
}

//in case he doesn't select any special options
void match_pattern_default(char* argv[], int argc){

 int fd;

 int r;

 int i = 0;

 char temp;

 char* pattern = argv[countOptions(argv, argc)+1]; 
 
 char line[MAX_LINE_LENGTH];

 DIR *dir;

 //this tells you the index at which files start
 int fileStartIndex = countOptions(argv, argc)+2;

for(int a = fileStartIndex; a < argc; a++){

 //primeiro, verificar se file é mesmo um ficheiro e não um diretório, fazendo opendir.

 char* file = argv[a];

 dir = opendir(file);

 if(dir != NULL) { 
  printf("%s is a directory.\n", file);
  return;
 } 

 //limpar a memória da linha
 memset(line,0,sizeof(line));

 //abrir o ficheiro para leitura e colocar carater a carater no buffer
 if((fd=open(file,O_RDONLY)) != -1)
 {
     while((r=read(fd,&temp,sizeof(char))) != 0)
     {

            if(temp != '\n')
            {
                line[i++] = temp;
            }

            else
            {   
		//se na linha que estar a ser analizada se econtra uma ocorrência do pattern, imprimir essa linha.
                if(strstr(line,pattern) != NULL){
                    if(argc > fileStartIndex+1){
			printf("%s:%s\n",file, line);
		    }

                    else printf("%s\n", line);
		}
                
		//para cada nova linha refrescar a memoria e continuar.
		memset(line,0,sizeof(line));
                
                i = 0;
            }

     }

   //null terminate da string
   line[i] = '\n';
 }

 else {
  perror("Can't open file");
  return;
 }

}
 
 return;
}

//-i
char* match_pattern_i(char* argv[], int argc, char* info[]){

 int fd;

 int r;

 int i = 0;

 char temp;

 char* pattern = argv[countOptions(argv, argc)+1]; 
 
 char line[MAX_LINE_LENGTH];

 DIR *dir;

 //this tells you the index at which files start
 int fileStartIndex = countOptions(argv, argc)+2;

for(int a = fileStartIndex; a < argc; a++){

 //primeiro, verificar se file é mesmo um ficheiro e não um diretório, fazendo opendir.

 char* file = argv[a];

 dir = opendir(file);

 if(dir != NULL) { 
  printf("%s is a directory.\n", file);
  return NULL;
 } 

 //limpar a memória da linha
 memset(line,0,sizeof(line));

 //abrir o ficheiro para leitura e colocar carater a carater no buffer
 if((fd=open(file,O_RDONLY)) != -1)
 {
     while((r=read(fd,&temp,sizeof(char))) != 0)
     {

            if(temp != '\n')
            {
                line[i++] = temp;
            }

            else
            {   

		if(global_isThereW){
		   
			if(strcasestr(line,pattern) != NULL){
				char *result = strcasestr(line,pattern);
				int position = result - line;
				int patternSize = strlen(pattern);
				//printf("chegou aqui\n");

	 			if(!isalpha(line[position-1]) && !isdigit(line[position-1]) && line[position-1] != '_'){
	  				if(!isalpha(line[position+patternSize]) && !isdigit(line[position+patternSize]) && line[position+patternSize] != '_'){

					if(argc > fileStartIndex+1){
		     		  		printf("%s:%s\n",file, line);
		   			}		
	           	    	
					else printf("%s\n",line);
				
			 	}	
			   }
		     }
		}
		
		//se na linha que estar a ser analizada se econtra uma ocorrência do pattern, imprimir essa linha.
                else if(strcasestr(line,pattern) != NULL){
                    if(argc > fileStartIndex+1){
			printf("%s:%s\n",file, line);
		    }

                    else printf("%s\n", line);
		}
                
		//para cada nova linha refrescar a memoria e continuar.
		memset(line,0,sizeof(line));
                
                i = 0;
            }

     }

   //null terminate da string
   line[i] = '\n';
 }

 else {
  perror("Can't open file");
  return NULL;
 }

}
 
 return *info;
}

//-l
char* match_pattern_l(char* argv[], int argc, char* info[]){

 int fd;

 int r;

 int i = 0;

 char temp;

 char* pattern = argv[countOptions(argv, argc)+1]; 
 
 char line[MAX_LINE_LENGTH];

 DIR *dir;

 //this tells you the index at which files start
 int fileStartIndex = countOptions(argv, argc)+2;

for(int a = fileStartIndex; a < argc; a++){

 //flag to check if you print this or not.
 int printThisFile = 0;

 //primeiro, verificar se file é mesmo um ficheiro e não um diretório, fazendo opendir.

 char* file = argv[a];

 dir = opendir(file);

 if(dir != NULL) { 
  printf("%s is a directory.\n", file);
  return NULL;
 } 

 //limpar a memória da linha
 memset(line,0,sizeof(line));

 //abrir o ficheiro para leitura e colocar carater a carater no buffer
 if((fd=open(file,O_RDONLY)) != -1)
 {
     while((r=read(fd,&temp,sizeof(char))) != 0)
     {

            if(temp != '\n')
            {
                line[i++] = temp;
            }

            else
            {   
		if(global_isThereI){
		  
		  if(global_isThereW){
			if(strcasestr(line,pattern) != NULL){
				char *result = strcasestr(line,pattern);
				int position = result - line;
				int patternSize = strlen(pattern);

				if(!isalpha(line[position-1]) && !isdigit(line[position-1]) && line[position-1] != '_'){
	  				if(!isalpha(line[position+patternSize]) && !isdigit(line[position+patternSize]) && line[position+patternSize] != '_'){
					printThisFile = 1;
				
			 		}
				}

                  	}
		  }
		  
		  if(strcasestr(line,pattern) != NULL){
		    printThisFile = 1;
		  }

		}

		if(global_isThereW){
		   if(strstr(line,pattern) != NULL){
			char *result = strstr(line,pattern);
			int position = result - line;
			int patternSize = strlen(pattern);

			if(!isalpha(line[position-1]) && !isdigit(line[position-1]) && line[position-1] != '_'){
		   	 if(!isalpha(line[position+patternSize]) && !isdigit(line[position+patternSize]) && line[position+patternSize] != '_'){	
				
				if(argc > fileStartIndex+1){
		     		  printf("%s:%s\n",file, line);
		   		}		
	           	    	
				else printf("%s\n",line);
				
			 }
			}
		   }

		}		
		
		//se na linha que estar a ser analizada se econtra uma ocorrência do pattern, imprimir essa linha.
                if(strstr(line,pattern) != NULL){
                    printThisFile = 1;
		}
                
		//para cada nova linha refrescar a memoria e continuar.
		memset(line,0,sizeof(line));
                
                i = 0;
            }
     }

   //null terminate da string
   line[i] = '\n';
 }

 else {
  perror("Can't open file");
  return NULL;
 }

 if(printThisFile){
   printf("%s\n", file);
 }

}
 
 return *info;
}

//-n
char* match_pattern_n(char* argv[], int argc, char* info[]){

 int fd, r;
 int i = 0;
 char temp;
 char* pattern = argv[countOptions(argv, argc)+1]; 
 char line[MAX_LINE_LENGTH];

 DIR *dir;

//this tells you the index at which files start
 int fileStartIndex = countOptions(argv, argc)+2;

for(int a = fileStartIndex; a < argc; a++){

 char* file = argv[a];

 int n = 0;

 dir = opendir(file);

 if(dir != NULL) { 
  printf("%s is a directory.\n", file);
  return NULL;
 } 

 //limpar a memória da linha
 memset(line,0,sizeof(line));

 if((fd=open(file,O_RDONLY)) != -1)
 {
     while((r=read(fd,&temp,sizeof(char))) != 0)
     {
        if(temp != '\n')
        {
          line[i++] = temp;
        }

        else{   
	     //fazer a contagem das linhas
	     n++;

	     if(global_isThereI){
		if(global_isThereW){
		  if(strcasestr(line,pattern) != NULL){
			char *result = strcasestr(line,pattern);
			int position = result - line;
			int patternSize = strlen(pattern);

	 		if(!isalpha(line[position-1]) && !isdigit(line[position-1]) && line[position-1] != '_'){
	  			if(!isalpha(line[position+patternSize]) && !isdigit(line[position+patternSize]) && line[position+patternSize] != '_'){
	
					if(argc > fileStartIndex+1){
		     		  		printf("%s:%d:%s\n",file, n, line);
		   			}		
	           	    	
					else printf("%d:%s\n",n, line);
				
			   }
			}
		     }
		}
		else{
		 if(strcasestr(line,pattern) != NULL){
		   if(argc > fileStartIndex+1){
		     printf("%s:%d:%s\n",file, n, line);
		   }		
	           else printf("%d: %s\n",n,line);
		 }
	        }
	     }
	     else if(global_isThereW){
		   if(strstr(line,pattern) != NULL){
			char *result = strstr(line,pattern);
			int position = result - line;
			int patternSize = strlen(pattern);

			if(!isalpha(line[position-1]) && !isdigit(line[position-1]) && line[position-1] != '_'){
		   	 if(!isalpha(line[position+patternSize]) && !isdigit(line[position+patternSize]) && line[position+patternSize] != '_'){	
				
				if(argc > fileStartIndex+1){
		     		  printf("%s:%d:%s\n",file, n, line);
		   		}		
	           	    	
				else printf("%d:%s\n", n, line);
				
			 }
			}
		   }
	     }
	     else{
		if(strstr(line,pattern) != NULL){
                 if(argc > fileStartIndex+1){
		    printf("%s:%d:%s\n",file, n, line);
		 }
				
                  else printf("%d: %s\n",n,line);
	        }
	     }
                
		//para cada nova linha refrescar a memoria e continuar.
		memset(line,0,sizeof(line));
                
                i = 0;
            }

     }

   //null terminate da string
   line[i] = '\n';
 }

 else {
  perror("Can't open file");
  return NULL;
 }

}

 return *info;
}

//-c
void match_pattern_c(char* argv[], int argc, char* info[]){

 int fd, r;
 int i = 0;
 char temp;
 char* pattern = argv[countOptions(argv, argc)+1]; 
 char line[MAX_LINE_LENGTH];

 DIR *dir;

 //this tells you the index at which files start
 int fileStartIndex = countOptions(argv, argc)+2;

 for(int a = fileStartIndex; a < argc; a++){

 	char* file = argv[a];

        int n = 0;

 	dir = opendir(file);

 	if(dir != NULL) { 
  		printf("%s is a directory.\n", file);
  		return;
 	} 

 	//limpar a memória da linha
 	memset(line,0,sizeof(line));

 	if((fd=open(file,O_RDONLY)) != -1)
 	{
    	 while((r=read(fd,&temp,sizeof(char))) != 0)
    	 {
            if(temp != '\n')
            {
                line[i++] = temp;
            }

            else
            {   
		if(global_isThereI){
		  if(global_isThereW){
		   if(strcasestr(line,pattern) != NULL){
			char *result = strcasestr(line,pattern);
			int position = result - line;
			int patternSize = strlen(pattern);

			if(!isalpha(line[position-1]) && !isdigit(line[position-1]) && line[position-1] != '_'){
		   	 if(!isalpha(line[position+patternSize]) && !isdigit(line[position+patternSize]) && line[position+patternSize] != '_'){	
			    //fazer a contagem das linhas que contem pattern
			    n++;
				
			 }
			}
		   }
		  }
		  else{
		   if(strcasestr(line,pattern) != NULL){
		     //fazer a contagem das linhas que contem pattern
		     n++;
		   }
	          }
	     	}
	     	else if(global_isThereW){
		   if(strstr(line,pattern) != NULL){
			char *result = strstr(line,pattern);
			int position = result - line;
			int patternSize = strlen(pattern);

			if(!isalpha(line[position-1]) && !isdigit(line[position-1]) && line[position-1] != '_'){
		   	 if(!isalpha(line[position+patternSize]) && !isdigit(line[position+patternSize]) && line[position+patternSize] != '_'){	
			    //fazer a contagem das linhas que contem pattern
		     	    n++;
				
			 }
			}
		   }
	     	}
	     	else{
		  if(strstr(line,pattern) != NULL){
		    //fazer a contagem das linhas que contem pattern
		    n++;
	          }
	     	}
                
		//para cada nova linha refrescar a memoria e continuar.
		memset(line,0,sizeof(line));
                
                i = 0;
            }

     }
		printf("%d\n",n);

   //null terminate da string
   line[i] = '\n';
 }

 else {
  perror("Can't open file");
  return;
 }
 }

 return;
}

//-w
char* match_pattern_w(char* argv[], int argc, char* info[]){

 int fd, r;
 int i = 0;
 char temp;
 char* pattern = argv[countOptions(argv, argc)+1]; 
 char line[MAX_LINE_LENGTH];

 DIR *dir;

 //this tells you the index at which files start
 int fileStartIndex = countOptions(argv, argc)+2;

 for(int a = fileStartIndex; a < argc; a++){

 	char* file = argv[a];

 	dir = opendir(file);

 	if(dir != NULL) { 
  		printf("%s is a directory.\n", file);
  		return NULL;
 	} 

 	//limpar a memória da linha
 	memset(line,0,sizeof(line));

 	if((fd=open(file,O_RDONLY)) != -1)
 	{
    	 while((r=read(fd,&temp,sizeof(char))) != 0)
    	 {
            if(temp != '\n')
            {
                line[i++] = temp;
            }

            else
            {   
		if(global_isThereI){

		   if(strcasestr(line,pattern) != NULL){
			char *result = strcasestr(line,pattern);
			int position = result - line;
			int patternSize = strlen(pattern);

			if(!isalpha(line[position-1]) && !isdigit(line[position-1]) && line[position-1] != '_'){
		   	 if(!isalpha(line[position+patternSize]) && !isdigit(line[position+patternSize]) && line[position+patternSize] != '_'){	
				
				if(argc > fileStartIndex+1){
		     		  printf("%s:%s\n",file, line);
		   		}		
	           	    	
				else printf("%s\n",line);
				
			 }
			}
		   }

	         }
	
	     	else{
		  if(strstr(line,pattern) != NULL){
			char *result = strstr(line,pattern);
			int position = result - line;
			int patternSize = strlen(pattern);

			//printf("%c", line[position-1]);
			//printf("%c", line[position+patternSize]);

			if(!isalpha(line[position-1]) && !isdigit(line[position-1]) && line[position-1] != '_'){
		   	 if(!isalpha(line[position+patternSize]) && !isdigit(line[position+patternSize]) && line[position+patternSize] != '_'){	
				
				if(argc > fileStartIndex+1){
		     		  printf("%s:%s\n",file, line);
		   		}		
	           	    	
				else printf("%s\n",line);
				
			 }
			}
	          }
	     	}
                
		//para cada nova linha refrescar a memoria e continuar.
		memset(line,0,sizeof(line));
                
                i = 0;
            }
	}

   //null terminate da string
   line[i] = '\n';
 }

 else {
  perror("Can't open file");
  return NULL;
 }
 }

 return *info;
}

//-r
void match_pattern_r(char* argv[], int argc){
 return;
}

//select which function to run
void parse_option(char* argv[], int argc){

 char* info[MAX_LINE_LENGTH];

  if(global_isThereR){
  match_pattern_r(argv, argc);
  }

  else if(global_isThereL){
  match_pattern_l(argv, argc, info);
  }

  else if(global_isThereC){
  match_pattern_c(argv, argc, info);
  }

  else if(global_isThereN){
  match_pattern_n(argv, argc, info);
  }

  else {

   for(int i = 1; i <= countOptions(argv, argc); i++) {

    if(strcmp(argv[i],"-i") == 0){
     match_pattern_i(argv, argc, info);
     break;
    }

    else if(strcmp(argv[i],"-w") == 0){
     match_pattern_w(argv, argc, info);
     break;
    }

    else{
    //no more options from here on out, print and quit.
    break;
    }
  
  }
 
 }

 return;
}


int main(int argc, char* argv[]){

if(argc == 1){

printf("You didn't insert the minimum necessary elements, usage is: %s [options] pattern [file/dir]\n", argv[0]);

return 1;
}

if(argc == 2){
 //se forem duas coisas grep -a (por exemplo, não dá, mas grep - já procura um - num dado ficheiro especificado).
 
 if((argv[1])[0] == '-' && strlen(argv[1]) > 1){
  printf("usage is: %s [options] pattern [file/dir]\n", argv[0]);
  return 1;
 } 

 else {

  char new[100][100];

  int r;   
  
  int argcounter = 2;

  char temp;

  strcpy(new[0], argv[0]);
  strcpy(new[1], argv[1]);

 for(int j = argcounter; j < 100; j++) {
    
  int i = 0;  

  while((r=read(STDIN_FILENO,&temp,sizeof(char))) != 0){
        
        if(temp != '\n'){
          new[j][i++] = temp;
        }

        else break;
  }

  if(r == 0){
   break;
  }

  else argcounter++;
 }
 
 char* send[argcounter];

 for(int a = 0; a < argcounter; a++){
  send[a] = new[a];
 }
 
 match_pattern_default(send, argcounter);

 }

}


if(argc == 3){
//se forem 3 pode ser do género grep -r (qqlcoisa) || grep (pattern) (file) || grep -(outra opção válida, das que estão no enunciado, qql para além de r) (pattern) mas sem file, que tem de perguntar.

if(strcmp(argv[1],"-r") == 0){
 match_pattern_r(argv, argc);
}

else if(strcmp(argv[1],"-i") == 0 || strcmp(argv[1],"-l") == 0 || strcmp(argv[1],"-n") == 0 || strcmp(argv[1],"-c") == 0 || strcmp(argv[1],"-w") == 0){

  char new[100][100];

  int r;   
  
  int argcounter = 3;

  char temp;

  strcpy(new[0], argv[0]);
  strcpy(new[1], argv[1]);
  strcpy(new[2], argv[2]);

 for(int j = argcounter; j < 100; j++) {
    
  int i = 0;  

  while((r=read(STDIN_FILENO,&temp,sizeof(char))) != 0){
        
        if(temp != '\n'){
          new[j][i++] = temp;
        }

        else break;
  }

  if(r == 0){
   break;
  }

  else argcounter++;
 }
 
 char* send[argcounter];

 for(int a = 0; a < argcounter; a++){
  send[a] = new[a];
 }

 parse_option(send, argcounter);

}

else{
 match_pattern_default(argv, argc);
}

}

if(argc > 3){
 
 if(countOptions(argv, argc) == 0){
   match_pattern_default(argv, argc);
 }
 
 parse_option(argv, argc);
}

return 0;
}
