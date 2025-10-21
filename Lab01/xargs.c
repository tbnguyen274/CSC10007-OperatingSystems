#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAXLINE 512

int main(int argc, char *argv[]) {
	char buf[MAXLINE];
	char *xargv[MAXARG];
	int i, n, xargc;
	char c;
	int line_pos;

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

	// Read lines from stdin and execute command for each line
	while (1) {
		line_pos = 0;

		// Read a character at a time (until '\n' or EOF)
		while (1) {
			n = read(0, &c, 1);
			if (n <= 0) {
				// EOF
				if (line_pos > 0) {
					// Process the last line if there's content
					buf[line_pos] = 0;
					xargv[xargc] = buf;
					xargv[xargc + 1] = 0;

					if (fork() == 0) {
						exec(xargv[0], xargv);
						fprintf(2, "xargs: exec %s failed\n", xargv[0]);
						exit(1);
					}

					wait(0);
				}
				
				exit(0);
			}

			if (c == '\n') {
				break;
			}

			if (line_pos >= MAXLINE - 1) {
				fprintf(2, "xargs: line too long\n");
				exit(1);
			}

			buf[line_pos++] = c;
		}

		// Null-terminate the line
		buf[line_pos] = 0;

		// Skip empty lines
		if (line_pos == 0) {
			continue;
		}

		// Set up arguments: original command args + the line from stdin
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
			fprintf(2, "xargs: exec %s failed\n", xargv[0]);	// Only run on exec failure
			exit(1);
		}

		// Wait for child to complete
		wait(0);
	}

	exit(0);
}