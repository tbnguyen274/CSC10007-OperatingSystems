#include <kernel/types.h>
#include <kernel/stat.h>
#include <user/user.h>

int main(int argc, char *argv[]) {
    int p2c[2];  // parent to child
    int c2p[2];  // child to parent
    
    if (pipe(p2c) < 0 || pipe(c2p) < 0) {
        fprintf(2, "error: pipe creation failed\n");
        exit(1);
    }

    char buf = 'x';

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "error: fork failed\n");
        exit(1);
    }

    if (pid == 0) { // child
        close(p2c[1]); // close write end of parent->child
        close(c2p[0]); // close read end of child->parent

        if (read(p2c[0], &buf, 1) == 1) {
            fprintf(1, "%d: received ping\n", getpid());
        }
        else {
            fprintf(2, "error: child read failed\n");
        }

        if (write(c2p[1], &buf, 1) != 1) {
            fprintf(2, "error: child write failed\n");
        }

        close(p2c[0]);
        close(c2p[1]);
        exit(0);
    }
    else { // parent
        close(p2c[0]); // close read end of parent->child
        close(c2p[1]); // close write end of child->parent

        if (write(p2c[1], &buf, 1) != 1) {
            fprintf(2, "error: parent write failed\n");
        }

        if (read(c2p[0], &buf, 1) == 1) {
            fprintf(1, "%d: received pong\n", getpid());
        }
        else {
            fprintf(2, "error: parent read failed\n");
        }

        close(p2c[1]);
        close(c2p[0]);
        
        wait(0);  // wait for child to finish
    }

    exit(0);
}