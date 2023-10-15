# UNIX Shell
A basic UNIX shell written in C. The shell supports basic job control, pipes, and signal handling (SIGTSTP [ctrl+z], SIGINT [ctrl+c], and SIGQUIT [ctrl+d]).

## Building
Compilation and execution of the shell are handled within the `Makefile`. The shell can be compiled using either `make` or `make wsh`, and the entire shell can be launched using `make run`.

Additionally, the source can be packed into a tar using `make pack`.

## Usage
### Job Control:
Jobs can be run in the foreground and background. By default, job execution occurs in the foreground processing group. An ampersand (`&`) can be appended to the end of a command to immediately execute the job in the background. Spacing for the ampersand does not matter as long as it is the final character of the command. For example both of the below commands result in the same execution of the job in the background.
```
wsh> cat example.txt | tail -n 10 &
```
```
wsh> cat example.txt | tail -n 10&
```
#### 1) jobs (command)
All jobs executed with wsh as their controlling terminal can be monitored with the `jobs` command. The output of this command is formatted as follows:
```
<job id>: <program 1 name> <arg1> … <argN> | <program 2 name> <arg1> … <argN> | … [&]
```
The Job ID of a job is unique to the wsh shell and is independent of the process and process group ids. A job will be assigned the smallest available Job ID at the time of execution starting from 1. Once a job ID is in use, it cannot be changed or reused until the current job assigned that ID has been terminated. Jobs will maintain their Job ID indefinetly upon suspension.
#### 2) fg (command)
Jobs can be brought into the foreground using the fg command. This command takes either 0 or 1 argument(s). When called with 0 arguments, `fg` will bring the background job with the largest Job ID into the foreground. When called with 1 argument, the argument must be valid Job ID specifying the Job that is to be foregrounded. If the job is currently suspended, `fg` will send a SIGCONT to all processes attached to the job to continue its execution.

#### 3) bg (command)
Job execution can be continued as a background job through the use of `bg`. Similarly to `fg`, this command can take either 0 or 1 argument(s). If an argument is given, it must be a valid Job ID. Otherwise, the largest Job ID is used. Using `bg` results in a SIGCONT being sent to all processes attached to the job being sent to the background.

### Pipes
This shell supports the standard in and standard out redirection through the use of the pipe operator `|`. Thus the program on the left side of the pipe operator will have all of its output sent to the standard out file descriptor redirected to the standard in file descriptor of the program on the right side of the operator. An example of this is the following:
```
wsh> cat example.txt | tail -n 10 | sort -o sorted.txt
```

In fashion similar to the `&` operator, spacing on eitherside of the pipe operator does not affect the execution of the command. 

### Changing Directories:
The current working directory can be changed through the use of `cd`. Both absolute and relative paths can be used with the `cd` command to specify which directory to change into.

### Exiting:
The shell can be exited using either the `exit` command or by sending an EOF through ctrl+d. Both will cause the shell execution to terminate with exit code 0.

## Signals
The shell is set to ignore SIGTSTP and SIGINT signals when it is in the foreground. These signals will be sent to the foreground processing group allowing the foreground processes to handle them accordingly. SIGQUIT will be caught by the shell and lead to the termination its termination.
