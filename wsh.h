// Defining different modes the shell can be run in
typedef const int SHELL_MODE;
SHELL_MODE INTERACTIVE = 0;
SHELL_MODE BATCH = 1;

// Maximum argument count supported for a given command
const int MAX_ARG_CNT = 256;

// Job structre for keeping track of the processes started from shell
struct JOB {
    pid_t pid;
    int jid;
    char *pname;
    char *pargv[256];
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
