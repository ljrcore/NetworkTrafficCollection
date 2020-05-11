#include <assert.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>

#include "table.h"
#include "pid2inode.h"

/*
pid2inode。c文件的作用：通过读取指定进程的fd目录，找出和socket有关的fd文件句柄。
读取对应文件句柄的inode号。最后，将inode号更新到inodetable表和array表中。
*/

const int MAX_PID_LENGTH = 20;
const int MAX_FDLINK = 10;

unsigned long str2ulong(const char *ptr) {
	unsigned long retval = 0;

	while ((*ptr >= '0') && (*ptr <= '9')) {
		retval *= 10;
		retval += *ptr - '0';
		ptr++;
	}
	return retval;
}

void saveInode(unsigned long inode) {
	long *itable = inodetable;
	
	int i = 0;
	for (i = 0; i < MAX_INODE_NUM; i++) {
		if (0 == inodetable[i]) {
			inodetable[i] = inode;
			break;
		}
	}
	if (i >= MAX_INODE_NUM) {
		printf("inodetable space is too small\n");
		return ;
	}
	
	struct fiveArray *tmp = array;
	for (i = 0; i < MAX_ARRAY_NUM; i++) {
		if (0 == tmp[i].inode) {
			tmp[i].inode = inode;
			break;
		}
	}
	if (i >= MAX_ARRAY_NUM) {
		printf("arraytable space is too small\n");
		return ;
	}
}




void get_inode_by_linkname(const char *linkname) {
	if (strncmp(linkname, "socket:[", 8) == 0) {
		saveInode(str2ulong(linkname + 8));
	}
}

int findInode(const char *pid) {
	char dirname[10 + MAX_PID_LENGTH];

	size_t dirlen = 10 + strlen(pid);
	snprintf(dirname, dirlen, "/proc/%s/fd", pid);

	DIR *dir = opendir(dirname);

	if (!dir) {
		printf("Couldn't open dir %s\n", dirname);
		return 0;
	}

  /* walk through /proc/%s/fd/... */
	struct dirent *entry;
	while ((entry = readdir(dir))) {
		if (entry->d_type != DT_LNK)
			continue;

		size_t fromlen = dirlen + strlen(entry->d_name) + 1;
		char fromname[10 + MAX_PID_LENGTH + 1 + MAX_FDLINK];
		snprintf(fromname, fromlen, "%s/%s", dirname, entry->d_name);
		//printf("%s\n", fromname);
		int linklen = 80;
		char linkname[linklen];
		int usedlen = readlink(fromname, linkname, linklen - 1);
		if (usedlen == -1) {
			continue;
		}
		assert(usedlen < linklen);
		linkname[usedlen] = '\0';
		get_inode_by_linkname(linkname);

	}
	closedir(dir);
	return 1;
}
