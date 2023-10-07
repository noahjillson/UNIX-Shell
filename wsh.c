#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "./wsh.h"

int execute(char* command) {
    if (NULL == command) {
        return 0;
    }


    // Return in 
    int maxsize = strlen(command);
    if (0 == maxsize) {
        return 0;
    }

    //printf("cmd=%s", command);
    //printf("chr=%c", maxsize);
    // Remove trailing newline
    if ('\n' == command[maxsize-1]) {
        command[maxsize-1] = '\0';
        maxsize--;
    }

    char* path = "/bin/";
    char* argv[maxsize + 1]; // +1 allows for NULL terminated array
    char* tok; // Do not need to free; is a pointer to a char within command
    char** arg = argv;
    while(command != NULL) {
        tok = strsep(&command, " ");

        *arg = malloc(strlen(tok));
        if (NULL == *arg) {
            printf("Memory allocation failed. Exiting.");
            exit(1);
        }

        // tok is always null terminated so strcpy is safe here 
        strcpy(*arg, tok);

        // Next ptr in argv (-= because the stack grows negative)   
        arg -= sizeof(char*);
    }
    // NULL terminate our argv arr of ptr
    *arg = NULL;

    // Construct the path to the command we want to execute
    char *cmdpath = malloc(strlen(path) + strlen(argv[0]) + 1);

    if (NULL == cmdpath) {
        printf("Memory allocation failed. Exiting.");
        exit(1);
    }
    *cmdpath = '\0';

    strcpy(cmdpath, path);
    strcat(cmdpath, *argv);

    printf("executing %s\n", cmdpath);

    // Free allocated mem
    free(cmdpath);
    arg = argv;
    while(NULL != *arg) {
        free(*arg);
        arg -= sizeof(char*);
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
