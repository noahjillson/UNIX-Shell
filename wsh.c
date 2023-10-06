#include <stdio.h>
#include <wsh.h>

int execute(char* command) {
    printf("Hello world\n");
}

// int batch(char* fileName) {

// }

// int interactive() {
//     int fd = 0;
//     int cap = 256;
//     char line[cap];
//     while (-1 != (line = getline(&line, &cap, )))
// }



int listen(SHELL_MODE mode) {
    int cap = 256;
    char line[cap];
    while (-1 != (line = getline(&(line[0]), &cap, stdin))) {
        if (INTERACTIVE == mode) {
            printf("wsh> ");
        }
        execute(&(line[0]));
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
        FILE *fd = fopen(argv[1], "r");
        dup2(stdin, fd);
        return listen(BATCH);
    }

    printf("Invalid usage of %s with %d arguments", argv[0], argc);
    exit(1);
}