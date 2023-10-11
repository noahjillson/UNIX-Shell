#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "./wsh.h"

static struct JOB* jobList;
static int jobListDirty;
static int shellfd;
pid_t shell_pgid;

void sigtstp_handler(int signo) {
    tcsetpgrp(shellfd, getpid());
    printf("Caught a sigstp fron %d in group %d\n", getpid(), getpgid(getpid()));
}

int nextjid() {
    int minUnused = 1;
    struct JOB *curr = jobList;
    
    while (curr) {
        // If jobList has been pruned restart search for jobid
        if (jobListDirty) {
            jobListDirty = 0;
            minUnused = 1;
            curr = jobList;
            continue;
        }

        if (minUnused == curr->jid) {
            minUnused++;
            curr = jobList;
        }
        curr = curr->next;
    }

    return minUnused;
}

void addjob(pid_t pid, char **argv, char *runAs) {
    int jobid = nextjid();
    struct JOB *newJob = malloc(sizeof(struct JOB));
    if (!newJob) {
        printf("malloc failed.\n");
        exit(1);
    }

    // Case where jobList is empty
    if (!jobList) {
        jobList = newJob;
        newJob->prev = NULL;
    }
    else {
        struct JOB *curr = jobList;
        while (curr->next) curr = curr->next;
        curr->next = newJob;
        newJob->prev = curr;
    }

    newJob->pid = pid;
    newJob->jid = jobid;
    newJob->runAs = runAs;
    newJob->pipeNext = NULL;
    newJob->next = NULL;

    newJob->pname = malloc(strlen(argv[0]));
    if (!(newJob->pname)) {
        printf("malloc failed.\n");
        exit(1);
    }
    newJob->pname = strcpy(newJob->pname, argv[0]);

    char **arg = argv;
    char **parg = newJob->pargv;
    while(*arg) {
        *parg = malloc(strlen(*arg));
        if (!*parg) {
            printf("malloc failed.\n");
            exit(1);
        }
        strcpy(*parg, *arg);
        arg++;
        parg++;
    }
}

void printjobs() {
    struct JOB *curr = jobList;
    if(!curr) {
        printf("JobList Empty.\n");
    }

    while (curr) {
        printf("JOB: %s; JID: %d\n", curr->pname, curr->jid);
        curr = curr->next;
    }
}

void editJob(struct JOB *job, pid_t pid, char* pipenext, char* argv[]) {
    job->pid = pid;
    job->pipeNext = pipenext;

    job->pname = malloc(strlen(argv[0]));
    if (!(job->pname)) {
        printf("malloc failed.\n");
        exit(1);
    }
    job->pname = strcpy(job->pname, argv[0]);

    char **arg = argv;
    char **parg = job->pargv;
    while(*arg) {
        *parg = malloc(strlen(*arg));
        if (!*parg) {
            printf("malloc failed.\n");
            exit(1);
        }
        strcpy(*parg, *arg);
        arg++;
        parg++;
    }
}

void removejob(pid_t pidw) {
    struct JOB *curr = jobList;
    struct JOB *prev = NULL;

    while (curr) {
        // Remove curr from jobList if exited
        if (curr->pid == pidw) {
            // if (!(curr->pipeNext)) {
            //     action(curr->pipeNext, curr);
            //     prev = curr;
            // }
            // else{
            // Skip current going forward
            if (prev) {
                prev->next = curr->next;
            }
            else {
                jobList = curr->next;
            }

            // Skip current going backwards
            if (curr->next){
                curr->next->prev = prev;
            }
            //}

        }
        else{
            prev = curr;
        }
        curr = curr->next;
    }
}


// int execute(char* command, struct JOB *callingJob) {
//     // Return 0 if NULL command was passed
//     if (NULL == command) {
//         return 0;
//     }

//     char* commandCpy = malloc(strlen(command));
//     if (!commandCpy) {
//         printf("malloc failed.\n");
//         exit(1);
//     }
//     strcpy(commandCpy, command);

//     // Return 0 if empty command was passed
//     int maxsize = strlen(command);
//     int bg = ('&' == command[maxsize-1]);
//     if (bg) {
//         command[maxsize-1] = '\0';
//         maxsize--;
//     }
//     if (0 == maxsize) {
//         return 0;
//     }

//     char* path = "/bin/";
//     char* argv[maxsize + 1]; // +1 allows for NULL terminated array
//     char* tok; // Do not need to free; is a pointer to a char within command
//     char** arg = argv;

//     char* pipetok = strsep(&command, "|");
//     char* pipenext = command;
//     char* pipenextcpy = malloc(strlen(command));
//     if (!pipenextcpy) {
//         printf("malloc failed.\n");
//         exit(1);
//     }
//     while (pipenext && *pipenext != '|') pipenext++;
//     if (*pipenext == '|') {
//         pipenextcpy = strcpy(pipenextcpy, pipenext);
//     }
//     else {
//         pipenextcpy = NULL;
//     }

    
//     while(pipetok != NULL) {
//         tok = strsep(&pipetok, " ");

//         if(0 == strlen(tok)) {
//             continue;
//         }

//         // Allocate mem and copy token into argument array
//         *arg = malloc(strlen(tok));
//         if (NULL == *arg) {
//             printf("Memory allocation failed. Exiting.\n");
//             exit(1);
//         }
//         strcpy(*arg, tok); // tok is always null terminated so strcpy is safe here 
        
//         arg += 1;
//     }
//     // NULL terminate our argv arr of ptr
//     *arg = NULL;

//     // Check if command is internally handled
//     if (builtin(argv[0], argv)) {
//         return 0;
//     }

//     // Construct the path to the command we want to execute
//     char *cmdpath = malloc(strlen(path) + strlen(argv[0]) + 1);
//     if (NULL == cmdpath) {
//         printf("Memory allocation failed. Exiting\n");
//         exit(1);
//     }
//     *cmdpath = '\0';
//     strcpy(cmdpath, path);
//     strcat(cmdpath, *argv);

//     // Verify file existance for cmdpath
//     if(-1 == access(cmdpath, F_OK)) {
//         //printf("File does not exist: %s\n", cmdpath);
//         //return 0;
//     }

//     // Verify execute permisions for cmdpath
//     if(-1 == access(cmdpath, X_OK) ) {
//         //printf("Execute permission denied for %s\n", cmdpath);
//         //exit(1);
//     }

//     // Execute command
//     int isParent = fork();
//     if (-1 == isParent) {
//         printf("Failed to fork.\n");
//         exit(1);
//     }
//     if (isParent) {
//         if (callingJob) {
//             editJob(callingJob, isParent, pipenextcpy, argv);
//         }
//         else {
//             addjob(isParent, argv, commandCpy, pipenextcpy);
//         }

//         // We should not wait if we are running a process with &
//         if(!bg){

//             // NOTE: MAJOR FIX (maybe) need to instead of changing the fg process group, I can add a process to the current fg process
//             // group then my shell stays in foreground and just waits for output. Thus when ctrl z is delivered, my shell can handle
//             // and the other processes can be suspended. Thus the shell will remain in foreground. As of now this does not happen
//             // The shell is moved to the bg and then when ctrl z is delivered, it does not move back into the fg.
//             if (setpgid(isParent, isParent)) {
//                 printf("Unable to set pgid of %d to %d.\n", isParent, isParent);
//                 exit(1);
//             }

//             // Set process to foreground
//             if (-1 == tcsetpgrp(shellfd, getpgid(isParent))) {
//                 printf("Unable to bring pgid %d to the foreground.\n", isParent);
//                 exit(1);
//             }

//             int stat = 1;
//             if (-1 == waitpid(isParent, &stat, WUNTRACED)) {
//                 printf("Error waiting for pid %d.\n", isParent);
//                 exit(1);
//             }

//             if (WIFEXITED(stat)) {
                
//                 removejob(isParent, &execute);
                
//             }

//             // Retrun shell to foreground
//             tcsetpgrp(shellfd, shell_pgid);
//         }
//     }
//     else {
//         // Set pgid of child to child pid (Done in parent and child to avoid race condition)
//         pid_t childpid = getpid();
//         if (setpgid(childpid, childpid)) {
//             printf("Unable to set pgid of %d to %d.\n", isParent, isParent);
//             exit(1);
//         }

//         if (!bg){
//             // Set process to foreground (Done in parent and child to avoid race condition)
//             if (-1 == tcsetpgrp(shellfd, getpgid(childpid))) {
//                 printf("Unable to bring pgid %d to the foreground.\n", isParent);
//                 exit(1);
//             }
//         }

//         // or cmdpath
//         if (-1 == execvp(argv[0], argv)) {
//             printf("Execution failed for %s\n", cmdpath);
//             exit(1);
//         }

//         // Precautionary but will never be reached 
//         exit(0);
//     }

//     // Free allocated mem
//     free(cmdpath);
//     arg = argv;
//     while(NULL != *arg) {
//         free(*arg);
//         arg += 1;
//     }

//     // return success
//     return 0;
// }


void prune() {
    struct JOB *curr = jobList;
    struct JOB *prev = NULL;

    while (curr) {
        int stat = 1;
        waitpid(curr->pid, &stat, WNOHANG);

        // Remove curr from jobList if exited
        if (WIFEXITED(stat)) {
            // if (!(curr->pipeNext)) {
            //     execute(curr->pipeNext, curr);
            //     prev = curr;
            // }
            //else {
            // Skip current going forward
            if (prev) {
                prev->next = curr->next;
            }
            else {
                jobList = curr->next;
            }

            // Skip current going backwards
            if (curr->next){
                curr->next->prev = prev;
            }
            //}

        }
        else{
            prev = curr;
        }
        curr = curr->next;
    }
}

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

struct JOB* largestJobFromArgs(char* argv[]) {
    int argc = getargc(argv);
    struct JOB *j = NULL;
    int jobid;

    if (!jobList) {
        return NULL;
    }

    if (argc == 1) {
        int max = 0;
        struct JOB *curr = jobList;
        while (curr) {
            if (max < curr->jid) {
                max = curr->jid;
                j = curr;
            }
            curr = curr->next;
        }
        return j;
    }
    else if (argc == 2) {
        jobid = atoi(argv[1]);
        struct JOB *curr = jobList;
        while (curr) {
            if (curr->jid == jobid) {
                return curr;
            }
            curr = curr->next;
        }
    }

    printf("Too many arguments passed to %s.\n", argv[0]);
    exit(1); 
}

void bg(char* argv[]) {
    struct JOB *j = largestJobFromArgs(argv);

    if (!j) return;

    kill(j->pid, SIGCONT);
}

void fg(char* argv[]) {
    struct JOB *j = largestJobFromArgs(argv);

    if (!j) return;

    tcsetpgrp(shellfd, getpgid(j->pid));
    kill(j->pid, SIGCONT);

    int stat = 1;
    waitpid(j->pid, &stat, WUNTRACED);
    if (WIFEXITED(stat)) {
        removejob(j->pid);
    }
}

void exitShell() {
    exit(0);
}

void jobs() {
    struct JOB *curr = jobList;
    while(curr) {
        char *last = &((curr->runAs)[strlen(curr->runAs)]);
        if (*last == '&') {
            *last = '\0';
            printf("%d: %s &\n", curr->jid, curr->runAs);
            *last = '&';
        }
        else {
            printf("%d: %s\n", curr->jid, curr->runAs);
        }
        
        curr = curr->next;
    }
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
        bg(argv);
    }
    else if (!strcmp(command, "fg"))
    {
        fg(argv);
    }
    else if (!strcmp(command, "jobs"))
    {
        jobs();
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


// int execute(char* command) {
//     // Return 0 if NULL command was passed
//     if (NULL == command) {
//         return 0;
//     }
//     char* commandCpy = malloc(strlen(command));
//     if (!commandCpy) {
//         printf("malloc failed.\n");
//         exit(1);
//     }
//     strcpy(commandCpy, command);

//     // Return 0 if empty command was passed
//     int maxsize = strlen(command);
//     int bg = ('&' == command[maxsize-1]);
//     if (bg) {
//         command[maxsize-1] = '\0';
//         maxsize--;
//     }
//     if (0 == maxsize) {
//         return 0;
//     }
// }

/**
 * Returns a NULL terminated array of substrings from command separated by sep
*/
// char** parseArgs(char *command, char *sep, char *(*argv)[]) {
//     // Return 0 if NULL command was passed
//     if (NULL == command) {
//         return NULL;
//     }

//     int maxsize = strlen(command);
//     int bg = ('&' == command[maxsize-1]);
//     if (bg) {
//         command[maxsize-1] = '\0';
//         maxsize--;
//     }
//     if (0 == maxsize) {
//         return NULL;
//     }

//     char* tok; // Do not need to free; is a pointer to a char within command
//     char** arg = *argv;
//     while(command != NULL) {
//         tok = strsep(&command, " ");

//         if(0 == strlen(tok)) {
//             continue;
//         }

//         // Allocate mem and copy token into argument array
//         *arg = malloc(strlen(tok));
//         if (NULL == *arg) {
//             printf("Memory allocation failed. Exiting.\n");
//             exit(1);
//         }
//         strcpy(*arg, tok); // tok is always null terminated so strcpy is safe here 
        
//         arg += 1;
//     }
//     // NULL terminate our argv arr of ptr
//     *arg = NULL;

//     return argv;
// }

// void createproc(char *(*args)[]) {
//     char *argv[MAX_ARG_CNT] = *args;

//     if (builtin(argv[0], argv)) {
//         return 0;
//     }

//     char *path = "/bin/";
//     char *cmdpath = malloc(strlen(path) + strlen(argv[0]) + 1);
//     if (NULL == cmdpath) {
//         printf("Memory allocation failed. Exiting\n");
//         exit(1);
//     }
//     *cmdpath = '\0';
//     strcpy(cmdpath, path);
//     strcat(cmdpath, *argv);

//     // Verify file existance for cmdpath
//     if(-1 == access(cmdpath, F_OK)) {
//         //printf("File does not exist: %s\n", cmdpath);
//         //return 0;
//     }

//     // Verify execute permisions for cmdpath
//     if(-1 == access(cmdpath, X_OK) ) {
//         //printf("Execute permission denied for %s\n", cmdpath);
//         //exit(1);
//     }

//     int isParent = fork();
//     if (-1 == isParent) {
//         printf("Failed to fork.\n");
//         exit(1);
//     }
//     if (isParent) {
//         addjob(isParent, argv, commandCpy);

//         // We should not wait if we are running a process with &
//         if(!bg){
//             if (setpgid(isParent, isParent)) {
//                 printf("Unable to set pgid of %d to %d.\n", isParent, isParent);
//                 exit(1);
//             }

//             // Set process to foreground
//             if (-1 == tcsetpgrp(shellfd, getpgid(isParent))) {
//                 printf("Unable to bring pgid %d to the foreground.\n", isParent);
//                 exit(1);
//             }

//             int stat = 1;
//             if (-1 == waitpid(isParent, &stat, WUNTRACED)) {
//                 printf("Error waiting for pid %d.\n", isParent);
//                 exit(1);
//             }

//             if (WIFEXITED(stat)) {
//                 removejob(isParent);
//             }

//             // Retrun shell to foreground
//             tcsetpgrp(shellfd, shell_pgid);
//         }
//     }
//     else {
//         // Set pgid of child to child pid (Done in parent and child to avoid race condition)
//         pid_t childpid = getpid();
//         if (setpgid(childpid, childpid)) {
//             printf("Unable to set pgid of %d to %d.\n", isParent, isParent);
//             exit(1);
//         }

//         if (!bg){
//             // Set process to foreground (Done in parent and child to avoid race condition)
//             if (-1 == tcsetpgrp(shellfd, getpgid(childpid))) {
//                 printf("Unable to bring pgid %d to the foreground.\n", isParent);
//                 exit(1);
//             }
//         }

//         // or cmdpath
//         if (-1 == execvp(argv[0], argv)) {
//             printf("Execution failed for %s\n", cmdpath);
//             exit(1);
//         }

//         // Precautionary but will never be reached 
//         exit(0);
//     }
// }

// createjob(char **commands) {
//     char *prev = NULL;
//     char *curr = *commands;

//     char *argvp[MAX_ARG_CNT];
//     char *argvc[MAX_ARG_CNT];
    
//     while(commands) {
//         parseArgs(curr, " ", &argvc);

//         if (builtin(argvc[0], argvc)) {
//             return 0;
//         }

//         // think we need to verify that the args exist here then fork recrusively or something
//     }
// }

// void executecommand(char* job) {
//     char *commands[MAX_ARG_CNT];
//     if(!parseArgs(job, "|", &commands)) {
//         return;
//     }

//     char **command = commands;
//     while (!command) {
//         char *argv[MAX_ARG_CNT];
//         parseArgs(command, " ", &argv);
//         createproc(&argv);
//         command++;
//     }

    
// }

int execute(char* command) {
    // Return 0 if NULL command was passed
    if (NULL == command) {
        return 0;
    }

    char* commandCpy = malloc(strlen(command));
    if (!commandCpy) {
        printf("malloc failed.\n");
        exit(1);
    }
    strcpy(commandCpy, command);

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

    char* path = "/bin/";

    char* pipetok; // = strsep(&command, "|");
    // char* pipenext = command;
    // char* pipenextcpy = malloc(strlen(command));
    // if (!pipenextcpy) {
    //     printf("malloc failed.\n");
    //     exit(1);
    // }
    // while (pipenext && *pipenext != '|') pipenext++;
    // if (*pipenext == '|') {
    //     pipenextcpy = strcpy(pipenextcpy, pipenext);
    // }
    // else {
    //     pipenextcpy = NULL;
    // }

    int numcommands = 1;
    char *c = command;
    while (*c) {
        if (*c == '|') {
            numcommands++;
        }
        c++;
    }

    int stdfd[2];
    pipe(stdfd);
    dup2(STDOUT_FILENO, stdfd[1]);
    dup2(STDIN_FILENO, stdfd[0]);

    int currcommand = 1;
    pid_t firstCommandPid = -1;
    while(command != NULL){
        pipetok = strsep(&command, "|");
        //printf("pipetok: %s\n", pipetok);
        
        char* argv[maxsize + 1]; // +1 allows for NULL terminated array
        char* tok; // Do not need to free; is a pointer to a char within command
        char** arg = argv;
        while(pipetok != NULL) {
            tok = strsep(&pipetok, " ");
            if(0 == strlen(tok)) {
                continue;
            }
            //printf("ARG TOK: %s\n", tok);

            // Allocate mem and copy token into argument array
            *arg = malloc(strlen(tok));
            if (NULL == *arg) {
                printf("Memory allocation failed. Exiting.\n");
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
            printf("Memory allocation failed. Exiting\n");
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

        int pipefd[2];
        if (numcommands > 1){ //) && currcommand > 1 && currcommand < numcommands) {
            pipe(pipefd);
        }
        // Execute command
        int isParent = fork();
        if (-1 == isParent) {
            printf("Failed to fork.\n");
            exit(1);
        }
        if (isParent) {
            if (numcommands > 1) {
                if (currcommand == numcommands) {
                    //dup2(stdfd[0], STDIN_FILENO);
                }
                else {
                    dup2(pipefd[0], STDIN_FILENO);
                }
            }

            if (firstCommandPid == -1) {
                firstCommandPid = isParent;
            }

            addjob(isParent, argv, commandCpy);

            // We should not wait if we are running a process with &
            if(!bg){
                if (setpgid(isParent, firstCommandPid)) {
                    printf("PUnable to set pgid of %d to %d.\n", isParent, firstCommandPid);
                    exit(1);
                }

                // Set process to foreground
                if (-1 == tcsetpgrp(shellfd, getpgid(firstCommandPid))) {
                    printf("Unable to bring pgid %d to the foreground.\n", isParent);
                    exit(1);
                }

                int stat = 1;
                if (-1 == waitpid(isParent, &stat, WUNTRACED)) {
                    printf("Error waiting for pid %d.\n", isParent);
                    exit(1);
                }

                if (WIFEXITED(stat)) {
                    removejob(isParent);
                }

                // Retrun shell to foreground
                tcsetpgrp(shellfd, shell_pgid);
            }
        }
        else {
            if (numcommands > 1) {
                if (currcommand == numcommands) {
                    //dup2(stdfd[1], STDOUT_FILENO);
                }
                else {
                    dup2(pipefd[1], STDOUT_FILENO);
                }
            }

            if (firstCommandPid == -1) {
                firstCommandPid = getpid();
            }
            
            // Set pgid of child to child pid (Done in parent and child to avoid race condition)
            pid_t childpid = getpid();
            if (setpgid(childpid, firstCommandPid)) {
                printf("CUnable to set pgid of %d to %d.\n", childpid, firstCommandPid);
                exit(1);
            }

            if (!bg){
                // Set process to foreground (Done in parent and child to avoid race condition)
                if (-1 == tcsetpgrp(shellfd, getpgid(firstCommandPid))) {
                    printf("Unable to bring pgid %d to the foreground.\n", isParent);
                    exit(1);
                }
            }

            // or cmdpath
            if (-1 == execvp(argv[0], argv)) {
                printf("Execution failed for %s\n", cmdpath);
                exit(1);
            }

            // Precautionary but will never be reached 
            exit(0);
        }
    
        // Free allocated mem
        free(cmdpath);
        arg = argv;
        while(NULL != *arg) {
            free(*arg);
            arg += 1;
        }
        currcommand++;
    }
    // return success

    dup2(stdfd[1], STDOUT_FILENO);
    dup2(stdfd[0], STDIN_FILENO);

    return 0;
}

void prompt(SHELL_MODE mode) {
    if (INTERACTIVE == mode) {
        printf("wsh> ");
        fflush(stdout);
        if (fflush(stdout)) {
            printf("Failed to flush output buffer\n.");
            exit(1);
        }
    }
}

int listen(SHELL_MODE mode) {
    int len = 0;
    size_t cap = 256;
    char* line = NULL;
    prompt(mode);

    // getline mallocs a new buffer for us and will realloc() as necessary to handle longer lines
    while (-1 != (len = getline(&line, &cap, stdin))) {
        //printf("pid: %d, pgid: %d, psid %d\n", getpid(), getpgid(getpid()), getsid(getpid()));
        //printf("fgpgid: %d\n", tcgetpgrp(0));
        // Remove trailing newline
        if ('\n' == line[strlen(line)-1]) {
            line[strlen(line)-1] = '\0';
        }
        printjobs();
        //execute line as a command and arguments
        execute(line);
        prompt(mode);
    }

    free(line);
    exit(0);
}

void sigtou_handler(int signo) {
    // Set shell to fg if it is not already the foreground process
    //printf("CAUGHT A SIGGTTOU\n");
    // if (tcgetpgrp(shellfd) != getpid()) {
    //     //printf("running this code: %d\n", tcgetpgrp(shellfd));
    //     tcsetpgrp(shellfd, getpid());
    // }
}



void sigchld_handler(int signo){
    // int warg = 1;
    // waitpid(-1 , &warg, WNOHANG);
    // if (WIFEXITED(warg)) {
        // Set dirty flag for jobsList
    jobListDirty = 1;
    //printf("pruning\n");
    prune();
    //}

    tcsetpgrp(shellfd, shell_pgid);
}

int main(int argc, char** argv) {
    // Set signal handlers
    signal(SIGTSTP, sigtstp_handler); // We ignore so that SIGTSTP is not propogated to child processes 
    signal(SIGCHLD, sigchld_handler);
    signal(SIGTTOU, SIG_IGN); // Initially ignore the SIGTTOU signal handler

    // Linked list of jobs started from the shell
    jobList = NULL;
    jobListDirty = 0;

    // Get name of the controlling terminal
    char *shellname = ttyname(STDIN_FILENO);
    if (!shellname) {
        printf("Error getting controlling tty name.\n");
        exit(1);
    }

     // Get file descriptor associated to the controlling terminal
    shellfd = open(shellname, O_RDWR); 
    if (!shellfd) {
        printf("Error obtaining file describtor of controlling tty.\n");
        exit(1);
    }

    //printf("%s\n", ttyname(STDIN_FILENO));

    shell_pgid = getpid();
    //printf("Bash pid: %d\n", getpgid(shell_pgid));
    setpgid(shell_pgid, shell_pgid);
    tcsetpgrp(shellfd, shell_pgid);
    //signal(SIGTTOU, sigtou_handler);

    //printf("TTY/SHELLFD: %d, fg: %d\n", shellfd, tcgetpgrp(shellfd));
    //printf("pid: %d, pgid: %d, psid %d\n", getpid(), getpgid(getpid()), getsid(getpid()));
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
