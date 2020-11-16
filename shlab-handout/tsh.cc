// 
// tsh - A tiny shell program with job control
// 
// <Nicolas Mavromatis nima6629@colorado.edu>
//

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string>

#include "globals.h"
#include "jobs.h"
#include "helper-routines.h"

//
// Needed global variable definitions
//

static char prompt[] = "tsh> ";
int verbose = 0;

//
// You need to implement the functions eval, builtin_cmd, do_bgfg,
// waitfg, sigchld_handler, sigstp_handler, sigint_handler
//
// The code below provides the "prototypes" for those functions
// so that earlier code can refer to them. You need to fill in the
// function bodies below.
// 

void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

//
// main - The shell's main routine 
//
int main(int argc, char **argv) 
{
  int emit_prompt = 1; // emit prompt (default)

  //
  // Redirect stderr to stdout (so that driver will get all output
  // on the pipe connected to stdout)
  //
  dup2(1, 2);

  /* Parse the command line */
  char c;
  while ((c = getopt(argc, argv, "hvp")) != EOF) {
    switch (c) {
    case 'h':             // print help message
      usage();
      break;
    case 'v':             // emit additional diagnostic info
      verbose = 1;
      break;
    case 'p':             // don't print a prompt
      emit_prompt = 0;  // handy for automatic testing
      break;
    default:
      usage();
    }
  }

  //
  // Install the signal handlers
  //

  //
  // These are the ones you will need to implement
  //
  Signal(SIGINT,  sigint_handler);   // ctrl-c
  Signal(SIGTSTP, sigtstp_handler);  // ctrl-z
  Signal(SIGCHLD, sigchld_handler);  // Terminated or stopped child

  //
  // This one provides a clean way to kill the shell
  //
  Signal(SIGQUIT, sigquit_handler); 

  //
  // Initialize the job list
  //
  initjobs(jobs);

  //
  // Execute the shell's read/eval loop
  //
  for(;;) {
    //
    // Read command line
    //
    if (emit_prompt) {
      printf("%s", prompt);
      fflush(stdout);
    }

    char cmdline[MAXLINE];

    if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin)) {
      app_error("fgets error");
    }
    //
    // End of file? (did user type ctrl-d?)
    //
    if (feof(stdin)) {
      fflush(stdout);
      exit(0);
    }

    //
    // Evaluate command line
    //
    eval(cmdline);
    fflush(stdout);
    fflush(stdout);
  } 

  exit(0); //control never reaches here
}
  
/////////////////////////////////////////////////////////////////////////////
//
// eval - Evaluate the command line that the user has just typed in
// 
// If the user has requested a built-in command (quit, jobs, bg or fg)
// then execute it immediately. Otherwise, fork a child process and
// run the job in the context of the child. If the job is running in
// the foreground, wait for it to terminate and then return.  Note:
// each child process must have a unique process group ID so that our
// background children don't receive SIGINT (SIGTSTP) from the kernel
// when we type ctrl-c (ctrl-z) at the keyboard.
//
void eval(char *cmdline) 
{
  /* Parse command line */
  //
  // The 'argv' vector is filled in by the parseline
  // routine below. It provides the arguments needed
  // for the execve() routine, which you'll need to
  // use below to launch a process.
  //
  char *argv[MAXARGS];
    
    pid_t pid;
    //int jid; //job id
    struct job_t *job;//pointer to job struct

  // The 'bg' variable is TRUE if the job should run
  // in background mode or FALSE if it should run in FG
  //
    int bg = parseline(cmdline, argv);

    if (argv[0] == NULL)  
        return;   /* ignore empty lines */
    
    //execute built in command if it exists, else return 0.
    if (!builtin_cmd(argv))
    {   
        if (!bg)//foreground task
        {
            //call fork to create new process.
            if ((pid=fork())==0) //if child:
            {
                setpgid(0,0);//set group id to PID of child (from pdf)
                //execute program stored in filename argv[0], with args in argv
                if (execv(argv[0], argv)<0)//if command is invalid, returns negative.
                {
                    printf("%s: Command not found\n", argv[0]);
                    exit(0);//terminate child, which sends a sigchild signal to parent. 
                }
                return;
            }
            else //parent process
            {
                //add as a foreground job.
                addjob(jobs, pid, FG, cmdline);
                //waits until process PID is no longer foreground process.
                waitfg(pid); //wait for a child to terminate and reap it. 
            }
        }
        else //background task
        {
            if ((pid=fork())==0) //child process
            {
               
                setpgid(0,0);//set group id to PID of child
                if (execv(argv[0], argv)<0)//if command is invalid
                {
                    printf("%s: Command not found\n", argv[0]);
                    exit(0);//terminate child
                }
                return;
            }
            else //parent process
            {
                //add as background job
                addjob(jobs, pid, BG, cmdline);
                //store job in pointer, found by pid.
                job=getjobpid(jobs, pid);
                printf("[%d] (%d) %s", job->jid, job->pid, cmdline);
            }
        }
    }
  return;
}


/////////////////////////////////////////////////////////////////////////////
//
// builtin_cmd - If the user has typed a built-in command then execute
// it immediately. The command name would be in argv[0] and
// is a C string. We've cast this to a C++ string type to simplify
// string comparisons; however, the do_bgfg routine will need 
// to use the argv array as well to look for a job number.
//
int builtin_cmd(char **argv) 
{
  string cmd(argv[0]);
    
    if (cmd=="quit")
    {
        exit(0);
    }
    else if (cmd=="jobs")
    {
        listjobs(jobs);
        return 1;
    }
    else if (cmd=="bg")
    {
        do_bgfg(argv);
        return 1;
    }
    else if (cmd=="fg")
    {
        do_bgfg(argv);
        return 1;
    }
  return 0;     /* not a builtin command that is USER DEFINED*/
}

/////////////////////////////////////////////////////////////////////////////
//
// do_bgfg - Execute the builtin bg and fg commands
//
void do_bgfg(char **argv) 
{
  struct job_t *jobp=NULL;
    
  /* Ignore command if no argument */
  if (argv[1] == NULL) {
    printf("%s command requires PID or %%jobid argument\n", argv[0]);
    return;
  }
    
  /* Parse the required PID or %JID arg */
  if (isdigit(argv[1][0])) {
    pid_t pid = atoi(argv[1]);
    if (!(jobp = getjobpid(jobs, pid))) {
      printf("(%d): No such process\n", pid);
      return;
    }
  }
  else if (argv[1][0] == '%') {
    int jid = atoi(&argv[1][1]);
    if (!(jobp = getjobjid(jobs, jid))) {
      printf("%s: No such job\n", argv[1]);
      return;
    }
  }	    
  else {
    printf("%s: argument must be a PID or %%jobid\n", argv[0]);
    return;
  }

  //
  // You need to complete rest. At this point,
  // the variable 'jobp' is the job pointer
  // for the job ID specified as an argument.
  //
  // Your actions will depend on the specified command
  // so we've converted argv[0] to a string (cmd) for
  // your benefit.
  //
  string cmd(argv[0]);
  if(cmd=="bg")
  {
      jobp->state=BG; //change job state
      //send continue to all in job group. Runs job if previously stopped.
      kill(-(jobp->pid), SIGCONT);
      printf("[%d] (%d) %s", pid2jid(jobp->pid), jobp->pid, jobp->cmdline);
  }
  else
  {
      jobp->state = FG;//change job state to foreground.
      //send continue to all in set of pid. Runs job if previously stopped.
      kill(-(jobp->pid), SIGCONT);
      waitfg(jobp->pid); //wait for job to finish, because it is in FG;
  }

  return;
}

/////////////////////////////////////////////////////////////////////////////
//
// waitfg - Block until process pid is no longer the foreground process
//
//This allows foreground process to execute and complete first.
void waitfg(pid_t pid)
{
    struct job_t *job=getjobpid(jobs, pid);
    //if job is in foreground state, sleep to spend time.
    //When it is in a background state, stop sleeping.
    while(job->state==FG)
    {
        sleep(1);
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////
//
// Signal handlers
//


/////////////////////////////////////////////////////////////////////////////
//
// sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
//     a child job terminates (becomes a zombie), or stops because it
//     received a SIGSTOP or SIGTSTP signal. The handler reaps all
//     available zombie children, but doesn't wait for any other
//     currently running children to terminate.  
//
//Note Signals are all sent in main by default. 
void sigchld_handler(int sig) 
{
    pid_t pid;
    int status;
    //save then later restore old error number.
    int olderrno=errno;
    //-1 means wait set is the entire set of children. 
    //WNOHANG means do not wait if there is no terminated child.
    //WUNTRACED means wait until a process is stopped or terminated. 
    
    //Using 'or' between them means return return 0 if none of children in wait set has stopped or terminated
    //or return with PID of one of stopped/terminated children. 
    //the pid returned is the pid of the child that was reaped.
    
    //This makes sure it handles signals that can't be queued, because the while loop executes and returns 
    //greater than zero as there are still children being reaped. 
    while((pid=waitpid(-1, &status, WNOHANG|WUNTRACED)) > 0)
    {
        //job=getjobpid(jobs, pid);
        if (WIFEXITED(status)) //if child exited normally
        {
            deletejob(jobs, pid);//delete current job.
        }
        
        else if(WIFSIGNALED(status))//if child terminated by a signal that was not caught 
        {
            sigint_handler(2); //call sigint_handler, sending signal 2. 
        }
        
        else if(WIFSTOPPED(status))//if child that caused the return is stopped
        {
            //store job struct in pointer by pid.
            struct job_t *job=getjobpid(jobs, pid);
            //change state to ST (stopped) which is CRUCIAL for listjobs()
            job->state=ST;
            sigtstp_handler(20); //call sigtstp_handler, sending signal 20.
        }
    }
    errno=olderrno;
    return;
}

/////////////////////////////////////////////////////////////////////////////
//
// sigint_handler - The kernel sends a SIGINT to the shell whenver the
//    user types ctrl-c at the keyboard.  Catch it and send it along
//    to the foreground job.  
//
//This is a termination signal handler
void sigint_handler(int sig) 
{
    pid_t pid = fgpid(jobs); //store foreground PID
    if (pid != 0) //if !=0, it is valid and exists.
    {
        printf("Job [%d] (%d) terminated by signal %d\n", pid2jid(pid), pid, sig);
        //sends kill signal to every process in |pid| process group.
        kill(-pid, SIGINT);
        //remove job from joblist. 
        deletejob(jobs, pid);
    }
    return;
  
}

/////////////////////////////////////////////////////////////////////////////
//
// sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
//     the user types ctrl-z at the keyboard. Catch it and suspend the
//     foreground job by sending it a SIGTSTP.  
//
//This is a pause signal handler
void sigtstp_handler(int sig) 
{
    //save pid of current foreground job.
    //returns 0 if no foreground job, else greater than zero.
    pid_t pid=fgpid(jobs);
    if(pid!=0)
    {
        printf("Job [%d] (%d) stopped by signal %d\n", pid2jid(pid), pid, sig);
        //sends kill signal to every process in |pid| process group.
        kill(-pid, SIGSTOP);
    }
    return;
}

/*********************
 * End signal handlers
 *********************/




