// Maximum argument count supported for a given command
#define MAX_ARG_CNT 256

// Defining different modes the shell can be run in
typedef const int SHELL_MODE;
SHELL_MODE INTERACTIVE = 0;
SHELL_MODE BATCH = 1;

// Job structre for keeping track of the processes started from shell
struct JOB {
    pid_t pid;
    int jid;
    char *pname;
    char *pargv[MAX_ARG_CNT];
    struct JOB *pipeNext;
    struct JOB *next;
    struct JOB *prev;
};
// const struct JOB HEAD = {
//     -1,
//     NULL,
//     NULL,
//     NULL,
//     NULL,
//     NULL
// };
