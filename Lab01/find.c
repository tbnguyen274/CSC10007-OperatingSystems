#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define MAXPATH 512

// Extracts file name from a given path
char *fmtname(char *path) {
	char *p;

	// Find first character after last slash.
	for (p = path + strlen(path); p >= path && *p != '/'; p--)
		;
	p++;

	return p;
}

// Recursively search for files with the name <filename> starting from the given path
void find(char *path, char *filename) {
	char buf[MAXPATH], *p;
	int fd;
	struct dirent de;
	struct stat st;

	if ((fd = open(path, 0)) < 0) {
		fprintf(2, "find: cannot open %s\n", path);
		return;
	}

	if (fstat(fd, &st) < 0) {
		fprintf(2, "find: cannot stat %s\n", path);
		close(fd);
		return;
	}

	switch (st.type) {
	case T_FILE:
		// If it's a file, check if the name matches
		if (strcmp(fmtname(path), filename) == 0) {
			printf("%s\n", path);
		}
		break;

	case T_DIR:
		if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
			printf("find: path too long\n");
			break;
		}
		strcpy(buf, path);
		p = buf + strlen(buf);
		*p++ = '/';
		while (read(fd, &de, sizeof(de)) == sizeof(de)) {
			if (de.inum == 0)
				continue;

			// Copy the directory entry name into the buffer
			memmove(p, de.name, DIRSIZ);
			p[DIRSIZ] = 0;	// Null-terminate
			if (stat(buf, &st) < 0) {
				printf("find: cannot stat %s\n", buf);
				continue;
			}

			// Avoid recursing into "." and ".."
			if (strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0) {
				// Check if current entry matches the filename
				if (strcmp(de.name, filename) == 0) {
					printf("%s\n", buf);
				}
				// If it's a directory, recurse into it
				if (st.type == T_DIR) {
					find(buf, filename);
				}
			}
		}
		break;
	}
	
	close(fd);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(2, "Usage: find <directory> <filename>\n");
		exit(1);
	}

	find(argv[1], argv[2]);
	exit(0);
}