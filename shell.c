#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "shell.h"

//built in commands that can't be run in the child processs (array of strings)
char *bi_commands[] = {
  "cd",
  "help",
  "exit"
};

//built in commands functions (array of function pointers)
int (*bi_functions[]) (char **) = {
  &my_cd,
  &my_help,
  &my_exit
};

char * redirects[] = {
  ">",
  "<"
};

int oldFDs[] = {
  STDOUT_FILENO,
  STDIN_FILENO
};

//returns the number of builtin commands
int numBI (){
  return sizeof(bi_commands) / sizeof(char *);
}

int numRedirs(){
  return sizeof(redirects) / sizeof(char *);
}

//function for 'cd'
int my_cd(char **args){
  if (args[1] == NULL) {
    fprintf(stderr, "no argument found\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("chdir error");
    }
  }
  return 1;
}

//function for 'exit'
int my_exit(){
  return 0; //program terminates
}

//function for 'help'
int my_help(){
  printf("\n||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
  printf("\nWelcome to Angelica's Shell!\n\n");
  printf("These are the instructions for use:\n");
  printf("      Type a command followed by appropriate argument(s) and press enter.\n");
  printf("      Items on the command line can be seperated by any number of spaces.\n");
  printf("      Multiple commands on one line should be seperated by a ';'\n");
  printf("      Use the man command to get information on other commands.\n");
  printf("\n||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n\n");
  return 1;
}

//returns number of tokens in command line
int count (char *line, char *delim) {
  int count = 1;
  char *temp = line;
  while((temp = strchr(temp, delim[0]))) {
      count++;
      temp++;
  }
  return count;
}

//reads in user input and returns it as a string
char * get_line(){ //reading user inpute
  char *input = (char *)calloc(1024, sizeof(char));
  char *line = fgets(input, 1024, stdin);


  int length = strlen(input);
  if (length > 1){
    input[length-1]= 0;
  }

  if (line == NULL) {
    return 0;
  }

  return input;
}

//splits user input based on delimiter (usually " " or ";")
char ** split_line(char *line, char *delim){
  int num = count (line, delim);
  char **args = (char**)calloc(num + 1, sizeof(char *));
  int i = 0;
  while(line && i < 9){
    args[i]= strsep(&line, delim);
    i ++;
  }
  args[i] = NULL;
  return args;
}

//deletes extra spaces around command line arguments
char ** delete_spaces (char ** args) {
  int i = 0;
  int n = 0;
  char ** new_args = (char**)calloc(10, sizeof(char *));
  while (args[i] && i < 9) {
    if (strcmp(args[i],"") != 0){
      new_args[n] = args[i];
      n ++;
    }
      i ++;
    }
    new_args[n] = NULL;
  return new_args;
}

//exits on keyboard interupt signal
static void sighandler(int signo){
  if (signo == SIGINT){//keyboard interrupt
    exit(0);
  }
}

int run_redirect(char **command, int *newfd, int fd2){
  char * file = command[1];
  char ** action = delete_spaces(split_line(command[0], " "));
  int i, status;
  pid_t parent, child;
  parent = fork();
  if (parent < 0) {
      perror("fork error");
  } else if (!parent) {// child process
      *newfd = dup(fd2);
      int fd1 = open(file, O_RDWR | O_CREAT, 0644);
      dup2(fd1, fd2);
      close(fd1);
      if (execvp(action[0], action) == -1) {
        perror("execution error");
        exit(EXIT_FAILURE);
      }
  } else { //parent process
      child = wait(&status);
    }

  return 1;
}

int run_pipe (char* input, char* output){
  char path[1024];
  FILE * i = popen(input,"r");
  FILE * o = popen(output, "w");
  while (fgets(path, 1024, i)){
    fprintf(o, "%s", path);
  }
  pclose(i);
  pclose(o);
  return 1;
}

//if command is one of the built in commands, it will run the built in command
int run_builtin (char ** c) {
  int x;
  for (x = 0; x < numBI(); x++) {
    if (strcmp(c[0], bi_commands[x]) == 0) { //if command is the same as one of the built in commands
      return (*bi_functions[x])(c); //do the built in command function
    }
  }
  return -1;
}

int run_nonbuiltin (char ** c) {
  int i, status;
  pid_t parent, child;
  parent = fork();
  if (parent < 0) {
      perror("fork error");
  } else if (!parent) {// child process
      if (execvp(c[0], c) == -1) {
        perror("execution error");
        exit(EXIT_FAILURE);
      }
  } else { //parent process
      child = wait(&status);
    }
  return 1; //success
}

int run_special(char * line) {
  int i, newfd, fd;
  char ** c;
    if(strchr(line, '|')){
      c = split_line(line, "|");
      return run_pipe(c[0], c[1]);
    }
    for (i = 0; i < numRedirs(); i++){
        char * action = redirects[i];
        if (strchr(line, (char)action)) {
          c = split_line(line, action);
          //printf ("file: %s\n",c[0]);
          //printf ("action[0]: %s\n",c[0]);
          return run_redirect(c, &newfd, oldFDs[i]);

        }
    }

  return -1;
}

int execute(char ** args, int num){
  int i, bi, special, nonbi, returnval;
  char * c;
  char ** parsed;
  if (args[0] == NULL) {//no commands
    return 0;
  }
  //printf ("num: %d \n", num);
  for(i = 0; i < num; i++){
    c = args[i];
    //printf ("c: %s \n", c);
    special = run_special(c);
    if (special != -1) returnval = special;
    else {
        parsed = split_line(c, " ");
        parsed = delete_spaces(parsed);
        bi = run_builtin (parsed);
        if (bi != -1) returnval = bi;
        else {
          nonbi = run_nonbuiltin(parsed);
          if (nonbi != -1) returnval = nonbi;
        }
    }
  }
  return returnval;
}

int command_loop() {
  char * line;
  char **args;
  int status, num;

  //runs once before checking status and keeps running if status doesn't equal 0
  do {
    signal(SIGINT, sighandler);//whenever the SIGINT is sent, RUN this function
    char cwd[2000];
    getcwd(cwd, sizeof(cwd));
    fprintf(stdout, "angelica's shell:%s$ ", cwd);
    line = get_line();
    if (line == 0) {
      printf("\n");
      return 0;
    }
    if (strcmp (line, "\n") != 0) {
      num = count (line, ";");
      args = split_line(line, ";");
      status = execute(args, num);

      free(line);
      free(args);
  } else {
    status = 1;
  }
  } while (status);
  return status;
}

//runs command loop
int main(int argc, char **argv){
  return command_loop();
}
