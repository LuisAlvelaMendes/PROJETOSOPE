#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <unistd.h>
#include<string.h>
#include <fcntl.h>

//in case he doesn't select any special options
void math_pattern_default(char* argv[]){
 return;
}

//-i
void math_pattern_i(char* argv[]){
 return;
}

//-l
void math_pattern_l(char* argv[]){
 return;
}

//-n
void math_pattern_n(char* argv[]){
 return;
}

//-c
void math_pattern_c(char* argv[]){
 return;
}

//-w
void math_pattern_w(char* argv[]){
 return;
}

//-r
void math_pattern_r(char* argv[]){
 return;
}

//select which function to run
void parse_option(char* argv){
 
 if(strcmp(argv,"-i") == 0){
  math_pattern_i(&argv);
 }

 if(strcmp(argv,"-l") == 0){
  math_pattern_l(&argv);
 }

 if(strcmp(argv,"-n") == 0){
  math_pattern_r(&argv);
 }

 if(strcmp(argv,"-c") == 0){
  math_pattern_c(&argv);
 }

 if(strcmp(argv,"-w") == 0){
  math_pattern_w(&argv);
 }

 if(strcmp(argv,"-r") == 0){
  math_pattern_r(&argv);
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
  return 2;
 }

}


if(argc == 3){
//se forem 3 pode ser do género grep -r (qqlcoisa) || grep (pattern) (file) || grep -(outra opção válida, das que estão no enunciado, qql para além de r) (pattern) mas sem file, que tem de perguntar.

if(strcmp(argv[1],"-r") == 0){
 math_pattern_r(argv);
}

else if(strcmp(argv[1],"-i") == 0 || strcmp(argv[1],"-l") == 0 || strcmp(argv[1],"-n") == 0 || strcmp(argv[1],"-c") == 0 || strcmp(argv[1],"-w") == 0){

 printf("Filename to search in: ");
 scanf("%s", filename);
 return 2;
}

else{
 math_pattern_default(argv);
}

}

if(argc > 3){
 parse_option(argv[1]);
}

return 0;
}
