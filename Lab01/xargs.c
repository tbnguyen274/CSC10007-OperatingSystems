#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAXLINE 512

// Process a line and execute
static void process_and_run(char *buf, char *xargv[], int xargc) {
    // Set up arguments: original command args + line from stdin
	xargv[xargc] = buf;
	xargv[xargc + 1] = 0; // Null-terminate the argument array

	// Fork and execute the command
	int pid = fork();
	if (pid < 0) {
		fprintf(2, "xargs: fork failed\n");
		exit(1);
	}
	if (pid == 0) {
		exec(xargv[0], xargv);

		// Only run on exec failure
		fprintf(2, "xargs: exec %s failed\n", xargv[0]);
		exit(1);
	}

	wait(0);	// Wait for child to complete
}


int main(int argc, char *argv[]) {
	char buf[MAXLINE];
	char *xargv[MAXARG];
	int i, n, xargc;
	char c;

	// Check for at least one argument (the command to execute)
	if (argc < 2) {
		fprintf(2, "Usage: xargs command [args...]\n");
		exit(1);
	}

	// Copy the command and its arguments to xargv
	// Note: argv[0] is "xargs", argv[1] is the command, argv[2]... are command args
	for (i = 1; i < argc; i++) {
		xargv[i - 1] = argv[i];
	}

	xargc = argc - 1;

	// Read lines from stdin and execute
	while (1) {
		i = 0;

		// Read a character at a time (until '\n' or EOF)
		while (1) {
			n = read(0, &c, 1);

			// EOF
			if (n <= 0) {
				// Unfinished line
				if (i > 0) {
					buf[i] = 0;	// Null-terminate
					process_and_run(buf, xargv, xargc);
				}
				
				exit(0);
			}

			// End of line -> break out to process
			if (c == '\n') {
				break;
			}

			// Avoid buffer overflow
			if (i >= MAXLINE - 1) {
				fprintf(2, "xargs: line too long\n");
				exit(1);
			}

			buf[i++] = c;
		}

		buf[i] = 0;	// Null-terminate

		// Skip empty lines
		if (i == 0) {
			continue;
		}

		process_and_run(buf, xargv, xargc);
	}

	exit(0);
}