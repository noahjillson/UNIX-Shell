// Maximum argument count supported for a given command
#define MAX_ARG_CNT 256

#define MAX_PIPE_CNT 256

// Defining different modes the shell can be run in
typedef const int SHELL_MODE;
SHELL_MODE INTERACTIVE = 0;
SHELL_MODE BATCH = 1;

// Job structre for keeping track of the processes started from shell
struct JOB {
    pid_t pgid;
    pid_t pid_list;
    int jid;
    char *pname;
    char *pargv[MAX_ARG_CNT];
    char *runAs;
    struct JOB *next;
    struct JOB *prev;
};
