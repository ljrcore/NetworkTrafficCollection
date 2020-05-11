#include <stdio.h>

#include "table.h"
#include "pid2inode.h"
#include "inode2array.h"
//这个main.c是个测试我写的inode2array,和pid2inode的接口，是否正常工作。

long inodetable[MAX_INODE_NUM];
struct fiveArray array[MAX_ARRAY_NUM];

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("input argument is error!\n");
		return 0;
	}
	findInode(argv[1]);
	addprocinfo("/proc/net/tcp");
	for (int i = 0; i < MAX_INODE_NUM; i++) {
		if (0 != inodetable[i])
			printf("inodetable[%d] ID:%ld\n", i, inodetable[i]);
	}
	for (int j = 0; j < MAX_ARRAY_NUM; j++) {
		if (0 != array[j].inode) {
			printf("local_addr: %s\t local_port: %d\t---remote_addr: %s\t rem_port: %d\t sa_family: %d\t inode: %ld\t\n", 
			array[j].local_addr, array[j].local_port, array[j].remote_addr, array[j].rem_port, array[j].sa_family,array[j].inode);
		}
	}
	printf("print over !\n");
	return 1;
}
