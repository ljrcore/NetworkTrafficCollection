#ifndef __TABLE_H
#define __TABLE_H

#define MAX_INODE_NUM 40
extern long inodetable[MAX_INODE_NUM];

struct fiveArray {
	char local_addr[50];
	int local_port;
	char remote_addr[50];
	int rem_port;
	short int sa_family;
	long inode;
};

#define MAX_ARRAY_NUM 40
extern struct fiveArray array[MAX_ARRAY_NUM];


#endif
