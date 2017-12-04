# project-1-systems

Angelica Zverovich's Shell 

Features my Shell Implements: 
    Read a line at a time, parse the line to separate the command from its arguments.
    Fork and exec the command. 
    The parent process waits until the exec'd program exits and then it should read the next command.
    cd, exit and help implemented seperately 
    Can read and separate multiple commands on one line with ; 
    Implement pipes 
    Takes care of extra white space (user is not restricted to a single space between items) 
    Exits on control-D (end of file) 
    Exits on control-C (keyboard interupt) 
    If you enter a newline, it will just enter a new blank prompt 
    
Features I wanted to implement: 
    Auto complete with tab 
    Using up arrow to get last entry

Bugs: 
    The biggest bug is that redirection does not work like it's supposed to 
    

//ALL FUNCTIONS THAT RETURN AN INT DO SO TO SIGNIFY WHETHER OR NOT THE PROCESS WAS SUCCESSFUL

//built in commands that can't be run in the child processs (array of strings)
char *bi_commands[];

//built in commands functions (array of function pointers)
int (*bi_functions[]) (char **);

//array of redirect characters (">" and "<")
char * redirects[];

//array of old file descriptors corresponding to redirect characters (STDOUT_FILENO, STDIN_FILENO )
int oldFDs[];

//returns the number of redirect characters
int numRedirs();

//returns the number of built in functions
int num_BI ();

//changes directory
//arugments: array of strings [command, argument1, argument2, ...]
int my_cd(char **args);

//exits out of shell
int my_exit();

//displays help information
int my_help(char **args);


//count the number of tokens in command line based on delimiter
//arguments: string (command line input), string(delimiter)
int count (char *line, char * delim);

//reads in input from command line
//returns: string(command line input)
char * get_line();

//split string into array based on delimiter
//arguments: string(command line input), string(delimiter)
//returns: array of strings seperated based on delimiter
char ** split_line(char *line, char * delim);

//deletes extra spaces in argument array
//arguments: array of strings(output of split_line)
//returns: array of strings with extra spaces taken out
char ** delete_spaces (char ** args);

//handles keyboard interupt signal
//arguments: int(number of the signal)
static void sighandler(int signo);

//executes redirect command
//arguments: string array (output of split_line), int pointer to new file descriptor, int (old file descriptor  (STDIN or STDOUT))
int run_redirect(char **command, int *newfd, int fd2);


//executes pipe command
//arguments: string (input for pipe), string (output for pipe)
int run_pipe(char* input, char* output);


//checks to see if command is built in (help, exit, cd) and runs it if it is
//arguments: array of strings (output from split_line)
int run_builtin (char ** c);

//if command is not built in, fork a process and execute the command
//arguments: array of strings (output from split_line)
int run_nonbuiltin (char ** c);

//checks to see if command line input uses a "special" command such as: >, <, or | and runs it if it does
//arguments: string (command line input)
int run_special(char * line);

//executes arguments
//arguments: array of strings (output of split_line), number of tokens (ouput from count)
int execute(char ** args, int num);

//loops through commands; returns 0 when done or -1 if there was a problem
int command_loop();

//initializes command_loop()
int main(int argc, char **argv);

