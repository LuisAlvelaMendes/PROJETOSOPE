#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <unistd.h>
#include<string.h>
#include <fcntl.h>

//in case he doesn't select any special options
void match_pattern_default(char* argv[]){
 return;
}

//-i
void match_pattern_i(char* argv[]){
 return;
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
void parse_option(char* argv[]){
 
 if(strcmp(argv[1],"-i") == 0){
printf("but he is here once \n");
  match_pattern_i(argv);
 }

 if(strcmp(argv[1],"-l") == 0){
  match_pattern_l(argv);
 }

 if(strcmp(argv[1],"-n") == 0){
  match_pattern_r(argv);
 }

 if(strcmp(argv[1],"-c") == 0){
  match_pattern_c(argv);
 }

 if(strcmp(argv[1],"-w") == 0){
  match_pattern_w(argv);
 }

 if(strcmp(argv[1],"-r") == 0){
  match_pattern_r(argv);
 }

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
  printf("Filename to search in: ");
  scanf("%s", filename);
  
  char* new[3];
  
  new[0] = argv[0];
  new[1] = argv[1];
  new[2] = filename;

  match_pattern_default(new);
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

 parse_option(new);
}

else{
 match_pattern_default(argv);
}

}

if(argc > 3){
 parse_option(argv);
}

return 0;
}
