#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "./wsh.h"

static struct JOB* jobList;
static int shellfd;

int getargc(char* argv[]) {
    // Count arguments
    int argc = 0;
    char** arg = argv;
    while (NULL != *arg) {
        argc++;
        arg++;
    }

    return argc;
}

void bg(char* argv[]) {
    int argc = getargc(argv);
    pid_t fgjob;

    if (argc == 1) {
        fgjob = tcgetpgrp(0);
    }
    //if

    exit(1);
}

void exitShell() {
    exit(0);
}

void cd(char* argv[]){
    // Count arguments
    int argc = getargc(argv);

    if (argc != 2) {
        printf("Invald arguments passed to cd\n");
        exit(1);
    }

    if (chdir(argv[1])) {
        printf("Unable to change directories\n");
        exit(1);
    }
}

int builtin(char* command, char* argv[]) {
    if (!strcmp(command, "exit")) {
        exitShell();
    }
    else if (!strcmp(command, "bg"))
    {
        //bg(argv);
    }
    else if (!strcmp(command, "fg"))
    {
        //fg(argv);
    }
    else if (!strcmp(command, "jobs"))
    {
        //jobs(argv);
    }
    else if (!strcmp(command, "cd"))
    {
        cd(argv);
    }
    else {
        return 0;
    }
    return 1;
}

int execute(char* command) {
    // Return 0 if NULL command was passed
    if (NULL == command) {
        return 0;
    }

    // Return 0 if empty command was passed
    int maxsize = strlen(command);
    int bg = ('&' == command[maxsize-1]);
    if (bg) {
        command[maxsize-1] = '\0';
        maxsize--;
    }
    if (0 == maxsize) {
        return 0;
    }

    // if (maxsize > MAX_ARG_CNT) {
    //     printf("\n");
    //     return 0;
    // }
    // Remove trailing newline
    // if ('\n' == command[maxsize-1]) {
    //     command[maxsize-1] = '\0';
    //     maxsize--;
    // }

    char* path = "/bin/";
    char* argv[maxsize + 1]; // +1 allows for NULL terminated array
    char* tok; // Do not need to free; is a pointer to a char within command
    char** arg = argv;
    while(command != NULL) {
        tok = strsep(&command, " ");

        if(0 == strlen(tok)) {
            continue;
        }

        // Allocate mem and copy token into argument array
        *arg = malloc(strlen(tok));
        if (NULL == *arg) {
            printf("Memory allocation failed. Exiting.");
            exit(1);
        }
        strcpy(*arg, tok); // tok is always null terminated so strcpy is safe here 
        
        arg += 1;
    }
    // NULL terminate our argv arr of ptr
    *arg = NULL;

    // Check if command is internally handled
    if (builtin(argv[0], argv)) {
        return 0;
    }

    // Construct the path to the command we want to execute
    char *cmdpath = malloc(strlen(path) + strlen(argv[0]) + 1);
    if (NULL == cmdpath) {
        printf("Memory allocation failed. Exiting.");
        exit(1);
    }
    *cmdpath = '\0';
    strcpy(cmdpath, path);
    strcat(cmdpath, *argv);

    // Verify file existance for cmdpath
    if(-1 == access(cmdpath, F_OK)) {
        //printf("File does not exist: %s\n", cmdpath);
        //return 0;
    }

    // Verify execute permisions for cmdpath
    if(-1 == access(cmdpath, X_OK) ) {
        //printf("Execute permission denied for %s\n", cmdpath);
        //exit(1);
    }

    // Execute command
    int isParent = fork();
    if (-1 == isParent) {
        printf("Failed to fork.\n");
        exit(1);
    }
    if (isParent) {
        printf("Parent\n");
        // We should not wait if we are running a process with &
        if(!bg){
            printf("CHILD is: %d\n", isParent);
            setpgid(isParent, isParent);
            int tcerno = tcsetpgrp(0, getpgid(isParent));
            printf("NEW FG: %d\n", tcgetpgrp(STDIN_FILENO));
            if (tcerno == -1) {
                if (errno == EINVAL) {
                    printf("An invalid value of pgid_id was specified.\n");
                }
                if (errno == EBADF) {
                    printf("The fildes argument is not a valid file descriptor.\n");
                }
                if(errno == ENOTTY) {
                    printf("The calling process does not have a controlling terminal, or the file represented by fildes is not the controlling terminal, or the controlling terminal is no longer associated with the session of the calling process.\n");
                }
            }
            waitpid(isParent, NULL, 0);
            tcsetpgrp(STDIN_FILENO, getpid());
        }
    }
    else {
        // or cmdpath
        printf("Child\n");
        if(!bg) {
            setpgid(getpid(), getpid());
            printf("FG is: %d\n", tcgetpgrp(0));
            printf("PRE FG child pid: %d child pgid: %d\n", getpid(), getpgid(getpid()));
            printf("child pid: %d child pgid: %d\n", getpid(), getpgid(getpid()));
        }
        else {
            printf("BACKGROUNDED\n");
        }

        if (-1 == execvp(argv[0], argv)) {
            printf("Execution failed for %s\n", cmdpath);
            exit(1);
        }
        exit(0);
    }

    // Free allocated mem
    free(cmdpath);
    arg = argv;
    while(NULL != *arg) {
        free(*arg);
        arg += 1;
    }

    // return success
    return 0;
}

int listen(SHELL_MODE mode) {
    int len = 0;
    size_t cap = 256;
    char* line = NULL;
    if (INTERACTIVE == mode) {
        printf("wsh> ");
    }

    // getline mallocs a new buffer for us and will realloc() as necessary to handle longer lines
    while (-1 != (len = getline(&line, &cap, stdin))) {
        printf("pid: %d, pgid: %d, psid %d\n", getpid(), getpgid(getpid()), getsid(getpid()));
        printf("fgpgid: %d\n", tcgetpgrp(0));
        // Remove trailing newline
        if ('\n' == line[strlen(line)-1]) {
            line[strlen(line)-1] = '\0';
        }
        
        //execute line as a command and arguments
        execute(line);
        if (INTERACTIVE == mode) {
            printf("wsh> ");
        }
    }

    free(line);
    exit(0);
}

void sigtou_handler(int signo) {
    // Set shell to fg if it is not already the foreground process
    if (tcgetpgrp(shellfd) != getpid()) {
        printf("running this code: %d\n", tcgetpgrp(shellfd));
        tcsetpgrp(shellfd, getpid());
    }
}
void myhndlr2(int signo) {
    printf("STOP\n\n\n");
}

int main(int argc, char** argv) {
    // Linked list of jobs started from the shell
    jobList = NULL;

    char *shellname = ttyname(STDIN_FILENO);
    if (!shellname) {
        printf("Error getting controlling tty name.\n");
        exit(1);
    }

    shellfd = open(shellname, O_RDWR); // fd associated to the controlling terminal
    if (shellfd)

    signal(SIGTSTP, myhndlr2);
    signal(SIGTTOU, SIG_IGN);

    printf("%s\n", ttyname(STDIN_FILENO));

    pid_t shell_pgid = getpid();
    setpgid(shell_pgid, shell_pgid);
    tcsetpgrp(shellfd, shell_pgid);
    signal(SIGTTOU, sigtou_handler);

    printf("TTY/SHELLFD: %d, fg: %d\n", shellfd, tcgetpgrp(shellfd));
    printf("pid: %d, pgid: %d, psid %d\n", getpid(), getpgid(getpid()), getsid(getpid()));
    // interactive shell mode
    if (1 == argc) {
        listen(INTERACTIVE);
    }
    // batch shell mode
    else if (2 == argc) {
        int fd = open(argv[1], O_RDONLY);
        if (fd < 0) {
            printf("Error opening file %s.\n", argv[1]);
            exit(1);
        }

        dup2(fd, 0);
        close(fd);
        listen(BATCH);
    }

    // Too many arguments passed exit 1
    printf("Invalid usage of %s with %d arguments.\n", argv[0], argc);
    exit(1);
}
