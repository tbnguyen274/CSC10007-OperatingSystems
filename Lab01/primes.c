#include <kernel/types.h>
#include <kernel/stat.h>
#include <user/user.h>

// avoid warning treated as error
void primes(int left_pipe[2]) __attribute__((noreturn));

void primes(int left_pipe[2]) {
    close(left_pipe[1]);  // close write end of left pipe
    
    int prime;
    
    // read first number from left neighbor
    if (read(left_pipe[0], &prime, sizeof(int)) != sizeof(int)) {
        close(left_pipe[0]);
        exit(0);  // no more numbers, exit
    }
    
    printf("prime %d\n", prime);
    
    int right_pipe[2];
    int forked = 0;
    int num;
    
    // read remaining numbers
    while (read(left_pipe[0], &num, sizeof(int)) == sizeof(int)) {
        // not divisible by prime
        if (num % prime != 0) {
            if (!forked) {
                // create pipe and fork child on first non-divisible number
                if (pipe(right_pipe) < 0) {
                    fprintf(2, "error: pipe creation failed\n");
                    exit(1);
                }
                
                // child process for next stage
                if (fork() == 0) {
                    close(left_pipe[0]);  // child doesn't need left pipe
                    primes(right_pipe);   // recursively handle next stage
                }
                
                close(right_pipe[0]);  // parent doesn't need read end
                forked = 1;
            }
            
            // pass number to right neighbor
            write(right_pipe[1], &num, sizeof(int));
        }
    }
    
    close(left_pipe[0]);
    
    if (forked) {
        close(right_pipe[1]);  // close write end to signal EOF
        wait(0);               // wait for child to finish
    }
    
    exit(0);
}

int main(int argc, char *argv[]) {
    int p[2];
    
    if (pipe(p) < 0) {
        fprintf(2, "error: pipe creation failed\n");
        exit(1);
    }
    
    if (fork() == 0) {
        // first primes process
        primes(p);
    }
    
    // main process: feed numbers 2-280 into pipeline
    close(p[0]);  // close read end
    
    for (int i = 2; i <= 280; i++) {
        write(p[1], &i, sizeof(int));
    }
    
    close(p[1]);  // close write end to signal EOF
    wait(0);      // wait for first primes process to finish
    
    exit(0);
}