#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "./wsh.h"

int execute(char* command) {
    printf("%s\n", command);
    return 0;
}

int listen(SHELL_MODE mode) {
    int len = 0;
    size_t cap = 256;
    char* line = NULL;
    // getline mallocs a new buffer for us and will realloc() as necessary to handle longer lines
    while (-1 != (len = getline(&line, &cap, stdin))) {
        if (INTERACTIVE == mode) {
            printf("wsh> ");
        }
        execute(line);
    }
    return 0;
}

int main(int argc, char** argv) {
    // interactive shell mode
    if (1 == argc) {
        return listen(INTERACTIVE);
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
        return listen(BATCH);
    }

    printf("Invalid usage of %s with %d arguments.\n", argv[0], argc);
    exit(1);
}
