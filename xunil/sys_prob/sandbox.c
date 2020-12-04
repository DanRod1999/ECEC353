/* 
 * Compile as follows: gcc -o sandbox sandbox.c -std=c99 -Wall
 * 
 * Authors: Daniel Rodriguez, Zoe Sucato
 *
 */



/* Includes from the C standard library */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/* POSIX includes */
#include <unistd.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <fcntl.h>

/* Linux includes */
#include <syscall.h>
#include <sys/ptrace.h>
#include <linux/ptrace.h>

int is_syscall_blocked(long orig_rax, long rdi, long rsi, long rdx, long r10, pid_t pid);
char * read_buffer_contents (pid_t pid, unsigned int count, long address);

int 
main (int argc, char **argv)
{
    if (argc != 2) {
        printf ("Usage: %s ./program-name\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    /* Extract program name from command-line argument (without the ./) */
    char *program_name = strrchr (argv[1], '/');
    if (program_name != NULL)
        program_name++;
    else
        program_name = argv[1];

    pid_t pid;
    pid = fork ();
    switch (pid) {
        case -1: /* Error */
            perror ("fork");
            exit (EXIT_FAILURE);

        case 0: /* Child code */
            /* Set child up to be traced */
            ptrace (PTRACE_TRACEME, 0, 0, 0);
            printf ("Executing %s in child code\n", program_name);
            execlp (argv[1], program_name, NULL);
            perror ("execlp");
            exit (EXIT_FAILURE);
    }

    /* Parent code. Wait till the child begins execution and is 
     * stopped by the ptrace signal, that is, synchronize with 
     * PTRACE_TRACEME. When wait() returns, the child will be 
     * paused. */
    waitpid (pid, 0, 0); 

    /* Send a SIGKILL signal to the tracee if the tracer exits.  
     * This option is useful to ensure that tracees can never 
     * escape the tracer's control.
     */
    ptrace (PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);

    /* Intercept and examine the system calls made by the tracee */

    while (1) {
        /* Wait for tracee to begin next system call */
        ptrace (PTRACE_SYSCALL, pid, 0, 0);
        waitpid (pid, 0, 0);

        /* Read tracee registers prior to syscall entry */
        struct user_regs_struct regs;
        ptrace (PTRACE_GETREGS, pid, 0, &regs);

        /* Check if syscall is allowed */
        int blocked = 0;
        if (is_syscall_blocked (regs.orig_rax, regs.rdi, regs.rsi, regs.rdx, regs.r10, pid) == 1) {
            blocked = 1;
            /* Set to invalid syscall and modify tracee registers */
            regs.orig_rax = -1;
            ptrace (PTRACE_SETREGS, pid, 0, &regs);
        }

        /* Execute system call and stop tracee on exiting call */
        ptrace (PTRACE_SYSCALL, pid, 0, 0);
        waitpid (pid, 0, 0);

        /* Get result of system call in register rax */
        if (ptrace (PTRACE_GETREGS, pid, 0, &regs) == -1) {
            if (errno == ESRCH)
                exit (regs.rdi);
            perror ("ptrace");
            exit (EXIT_FAILURE);
        }

        if (blocked) {
            regs.rax = -EPERM; /* Operation not permitted */
            ptrace(PTRACE_SETREGS, pid, 0, &regs);
        }

    }

    exit (EXIT_SUCCESS);

}

int is_syscall_blocked(long orig_rax, long rdi, long rsi, long rdx, long r10, pid_t pid) {
    // return 1 to block
    // return 0 to not block

    char* buffer;
    /* Allocate space to store the contents of the buffer */
    buffer = ( char *) malloc (sizeof ( char *) * 50);
    char tmp[5];
    long filename, flags;


    char temp_ref[] = "/tmp";

    // checks values for sys open and sys openat
    // depending on value different parameters are stored in different regs
    if (orig_rax == 257 || orig_rax == 2) {
        // sys openat 
        if (orig_rax == 257) {
            filename = rsi;
            flags = rdx;
        }
        // sys open
        else if (orig_rax == 2) {
            filename = rdi;
            flags = rsi;
        }

        buffer = read_buffer_contents(pid, 50, filename);
        printf("sys_open() is attempting directory :%s (len of %ld)\n", buffer, strlen(buffer));

        //check if create flag exist
        if ((flags & O_CREAT) == O_CREAT) {
            printf("Create flag found\n");
            strncpy(tmp, buffer, 4);
            tmp[4] = '\0';
            // check if directory is in tmp
            if (strcmp(tmp, temp_ref) == 0) {
                printf("Directory is in /tmp, syscall will not block\n");
                return 0;
            // if not then block
            } else {
                printf("Directory is not /tmp, syscall will block\n");
                return 1;
            }

        // ReadOnly flag is the only flag allowed for existing files
        // must check if O_ACCMODE contains read only
        } else if ((flags & O_ACCMODE) == O_RDONLY) { 
            printf("File exists and mode is read only, sys call will not block\n");
            return 0;
        } else  {
            printf("File exists but mode is not read only, sys call will block\n");
            return 1;
        }
    }

    // no opens were given so no blocking necessary
    else {
        return 0;
    }

}

/* Read contents of the buffer for the write() system call located at specified address */
char * 
read_buffer_contents (pid_t pid, unsigned int count, long address)
{
    char *c, *buffer;
    long data;
    unsigned int i, j, idx, num_words, lop_off;
        
    /* Allocate space to store the contents of the buffer */
    buffer = (char *) malloc (sizeof (char) * count);

    num_words = count/sizeof (long);
    lop_off = count - num_words * sizeof (long);
    idx = 0;
    
    /* Peek into the tracee's address space, each time returning a word (eight bytes) of data. 
     * Typecast the word into characters and store in our buffer.
     */
    for (i = 0; i < num_words; i++) {
        data = ptrace (PTRACE_PEEKDATA, pid, (void *) (address + i * sizeof (long)), 0);
        c = (char *) &data;
        for (j = 0; j < sizeof (long); j++)
            buffer[idx++] = c[j];
    }

    /* If the number of bytes is not a perfect multiple of the word length,
     * read and store the rest of the characters in our buffer.
     */
    if (lop_off > 0) {
        data = ptrace (PTRACE_PEEKDATA, pid, (void *) (address + i * sizeof (long)), 0);
        c = ( char *) &data;
        for (j = 0; j < lop_off; j++)
            buffer[idx++] = c[j];
    }

    return buffer;
}