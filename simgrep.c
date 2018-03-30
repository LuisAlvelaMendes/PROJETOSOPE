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

#define MAX_LINE_LENGTH 100

//INFO WILL BE THE ARRAY WE WILL PASS BETWEEN FUNCTIONS

//counts how many options there are
int countOptions(char* argv[], int argc){

int counter = 0;

for(int i = 1; i < argc; i++) {

 if(strcmp(argv[i],"-i") == 0 || strcmp(argv[i],"-l") == 0 || strcmp(argv[i],"-n") == 0 || strcmp(argv[i],"-c") == 0 || strcmp(argv[i],"-w") == 0 || strcmp(argv[i],"-r") == 0){
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
char* match_pattern_i(char* argv[], char* info[]){

return *info;

}

//-l
void match_pattern_l(char* argv[]){
 return;
}

//-n
void match_pattern_n(char* argv[]){
 return;
}

//-c
void match_pattern_c(char* argv[]){
 return;
}

//-w
void match_pattern_w(char* argv[]){
 return;
}

//-r
void match_pattern_r(char* argv[]){
 return;
}

//select which function to run
void parse_option(char* argv[], int argc){

char* info[MAX_LINE_LENGTH];

for(int i = 1; i < argc; i++) {

 if(strcmp(argv[i],"-i") == 0){
  match_pattern_i(argv, info);
 }

 else if(strcmp(argv[i],"-l") == 0){
  match_pattern_l(argv);
 }

 else if(strcmp(argv[i],"-n") == 0){
  match_pattern_r(argv);
 }

 else if(strcmp(argv[i],"-c") == 0){
  match_pattern_c(argv);
 }

 else if(strcmp(argv[i],"-w") == 0){
  match_pattern_w(argv);
 }

 else if(strcmp(argv[i],"-r") == 0){
  match_pattern_r(argv);
 }

 else{
 //no more options from here on out, print and quit.
 break;
 }
}

return;
}

int main(int argc, char* argv[]){

char filename[100];

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

  char* new[20];

  int argcounter = 2;

  new[0] = argv[0];
  new[1] = argv[1];

  printf("Filename to search in:\n");
  printf("To terminate file input write nothing and just press enter.\n");

  for(int i = 2; i < 20; i++){
  
   if (fgets (new[i], 50, STDIN_FILENO) == NULL)
     break;

   if (strcmp (new[i], "\n") == 0)
     break;

    argcounter++;
  }

  match_pattern_default(new, argcounter);
 }

}


if(argc == 3){
//se forem 3 pode ser do género grep -r (qqlcoisa) || grep (pattern) (file) || grep -(outra opção válida, das que estão no enunciado, qql para além de r) (pattern) mas sem file, que tem de perguntar.

if(strcmp(argv[1],"-r") == 0){
 match_pattern_r(argv);
}

else if(strcmp(argv[1],"-i") == 0 || strcmp(argv[1],"-l") == 0 || strcmp(argv[1],"-n") == 0 || strcmp(argv[1],"-c") == 0 || strcmp(argv[1],"-w") == 0){

 printf("Filename to search in: ");
 scanf("%s", filename);
 char* new[3];
  
 new[0] = argv[0];
 new[1] = argv[1];
 new[2] = filename;
 
 //FOR SEVERAL FILES??????
 //parse_option(new);
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
