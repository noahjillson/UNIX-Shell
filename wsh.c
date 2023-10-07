#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include "./wsh.h"

void exitShell() {
    exit(0);
}

void cd(char* argv[]){
    int argc = 0;
    char** arg = argv;
    while (NULL != *arg) {
        argc++;
        arg++;
    }

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
    if (NULL == command) {
        return 0;
    }

    int maxsize = strlen(command);

    // Remove trailing newline
    if ('\n' == command[maxsize-1]) {
        command[maxsize-1] = '\0';
        maxsize--;
    }

    // Return 0 if no command was passed
    if (0 == maxsize) {
        return 0;
    }

    char* path = "/bin/";
    char* argv[maxsize + 1]; // +1 allows for NULL terminated array
    char* tok; // Do not need to free; is a pointer to a char within command
    char** arg = argv;
    while(command != NULL) {
        tok = strsep(&command, " ");

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

    // Verify execute permisions for cmdpath
    if(-1 == access(cmdpath, X_OK)) {
        printf("Execute permission denied for %s\n", cmdpath);
        exit(1);
    }

    // Execute command
    int isParent = fork();
    if (isParent) {
        waitpid(isParent, NULL, 0);
    }
    else {
        if (-1 == execvp(cmdpath, argv)) {
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
        execute(line);
        if (INTERACTIVE == mode) {
            printf("wsh> ");
        }
    }

    free(line);
    exit(0);
}

int main(int argc, char** argv) {
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
