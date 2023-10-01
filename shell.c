#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
/*
This program is composed of two functions: main() and getcmd(). The
getcmd() function reads in the user’s next command, and then parses it into separate tokens that are
used to fill the argument vector for the command to be executed. If the command is to be run in the
background (concurrently, no waiting for child process), it will end with ‘&’, and getcmd() will update the background
parameter
so the main() function can act accordingly. The program terminates when the user enters <Control><D>
because getcmd() invokes exit().
The main() calls getcmd(), which waits for the user to enter a command. The contents of the
command entered by the user are loaded into the args array. For example, if the user enters ls –l at
the command prompt, args[0] is set equal to the string “ls” and args[1] is set to the string to “–
l”. (By “string,” we mean a null-terminated C-style string variable.)
This programming assignment is organized into three parts: (1) creating the child process and
executing the command in the child, (2) redirections, and (3) piping.
*/

// getcmd() reads in the next command line, separating it into distinct tokens
int getcmd(char *prompt, char *args[], int *background) {
  int length, i = 0;
  char *token, *loc;
  char *line = NULL;
  size_t linecap = 0;
  printf("%s", prompt);
  length = getline(&line, &linecap, stdin);
  if (length <= 0) {
    exit(-1);
  }
  // Check if background is specified..
  if ((loc = index(line, '&')) != NULL) {
    *background = 1;
    *loc = ' ';
  } else
    *background = 0;

  while ((token = strsep(&line, " \t\n")) != NULL) {
    for (int j = 0; j < strlen(token); j++)
      if (token[j] <= 32)
        token[j] = '\0';
    if (strlen(token) > 0)
      args[i++] = token;
  }
  return i;
}

// Below are some built-in commands.. implement by self..

/*echo command: This command takes a string with spaces in between as an argument. It prints theargument to the screen.*/
void echo(char *args[], int cnt) {
  for (int i = 1; i < cnt; i++)
    printf("%s ", args[i]);
  printf("\n");
}

/*cd command: This command takes a single argument that is a string. It changes into that directory. You
are basically calling the chdir() system call with the string that is passed in as the argument. You can
optionally print the current directory (present working directory) if there are no arguments. You don’t
need to support additional features (e.g., those found in production shells like bash).*/
void cd(char *args[], int cnt) {
  if (cnt == 1) {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("Current working dir: %s\n", cwd);
  } else {
    if (chdir(args[1]) != 0)
      printf("Error..\n");
  }
}

/*pwd command: This command takes no argument. It prints the present working directory.*/
void pwd() {
  char cwd[1024];
  getcwd(cwd, sizeof(cwd));
  printf("Current working dir: %s\n", cwd);
}

/*exit command: This command takes no argument. It terminates the shell. It will terminate any jobs
that are running in the background before terminating the shell.*/

/*fg command: This command takes an optional integer parameter as an argument. The background can
contain many jobs you could have started with & at the end. The fg command should be called with a
number (e.g., fg 3) and it will pick the job numbered 3 (assuming it exists) and put it in the foreground.
The command jobs should list all the jobs that are running in the background at any given time. These
are jobs that are put in the background by running the command with & as the last character in the
command line. Each line in the list shown by the jobs command should have a number identifier that
can be used by the fg command to bring the job to the foreground (as explained above). */

/*jobs command: This command takes no argument. It lists all the jobs that are running in the background */

int main(void) {
  char *args[20];
  int bg = 0;
  while (1) {
    bg = 0;
    int cnt = getcmd("\nsh>>  ", args, &bg);
    printf("The command entered has %d arguments\n", cnt);
    for (int i = 0; i < cnt; i++)
      printf("  args[%d] = %s\n", i, args[i]);
    if (bg)
      printf("Background enabled..\n");
    else
      printf("Background not enabled..\n");

    // check if need pipe
    int pipe = 0;
    for (int i = 0; i < cnt; i++) {
      if (strcmp(args[i], "|") == 0) {
        pipe = 1;
        args[i] = NULL;
      }
    }
    int cid = fork();
    if (cid == 0) {
      // check if need to redirect..
      int redirect = 0;
      char *file;
      for (int i = 0; i < cnt; i++) {
        if (strcmp(args[i], ">") == 0) {
          redirect = 1;
          file = args[i + 1];
          args[i] = NULL;
        }
      }
      if (redirect) {
        close(1);
        open(file, O_CREAT | O_WRONLY | O_TRUNC, 0777);
      }
      execvp(args[0], args);
      printf("Command not found..\n");
      exit(0);
    } else if (cid > 0) {
      if (bg == 0) {
        wait(NULL);
        printf("Child completed..\n");
      }
    } else {
      printf("Error..\n");
      exit(0);
    }
  }
  return 0;
}