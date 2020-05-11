#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "table.h"
#include "inode2array.h"

/*
这个。c文件的作用：读取/proc/net/tcp or tcp6 文件的内容，和inodetable表中inode节点进行比对，
比对成功，解析在/proc/net/tcp or tcp6文件的内容，取出和inode节点相关联的五元组的信息。
最后将五元组的信息更新到array表中。
*/
void addtoconninode(char *buffer) {
	short int sa_family;
	struct in6_addr result_addr_local = {};
	struct in6_addr result_addr_remote = {};

	char rem_addr[128], local_addr[128];
	int local_port, rem_port;
	struct in6_addr in6_local;
	struct in6_addr in6_remote;

	unsigned long inode;

	int matches = sscanf(buffer,
                       "%*d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %*X "
                       "%*X:%*X %*X:%*X %*X %*d %*d %ld %*512s\n",
                       local_addr, &local_port, rem_addr, &rem_port, &inode);

	if (matches != 5) {
		fprintf(stderr, "Unexpected buffer: '%s'\n", buffer);
		exit(0);
	}

	if (inode == 0) {
    /* connection is in TIME_WAIT state. We rely on
     * the old data still in the table. */
		return;
	}
  
	long *itable = inodetable;
	for (int i = 0; i < MAX_INODE_NUM; i++) {
		if (inode != itable[i]) {
			continue ;
		} else {
			
			if (strlen(local_addr) > 8) {
			/* this is an IPv6-style row */

			/* Demangle what the kernel gives us */
				sscanf(local_addr, "%08X%08X%08X%08X", &in6_local.s6_addr32[0],
						&in6_local.s6_addr32[1], &in6_local.s6_addr32[2],
						&in6_local.s6_addr32[3]);
				sscanf(rem_addr, "%08X%08X%08X%08X", &in6_remote.s6_addr32[0],
						&in6_remote.s6_addr32[1], &in6_remote.s6_addr32[2],
						&in6_remote.s6_addr32[3]);

				if ((in6_local.s6_addr32[0] == 0x0) && (in6_local.s6_addr32[1] == 0x0) &&
					(in6_local.s6_addr32[2] == 0xFFFF0000)) {
					/* IPv4-compatible address */
					result_addr_local.s6_addr32[0] = in6_local.s6_addr32[3];
					result_addr_remote.s6_addr32[0] = in6_remote.s6_addr32[3];
					sa_family = AF_INET;
				} else {
					/* real IPv6 address */
					// inet_ntop(AF_INET6, &in6_local, addr6, sizeof(addr6));
					// INET6_getsock(addr6, (struct sockaddr *) &localaddr);
					// inet_ntop(AF_INET6, &in6_remote, addr6, sizeof(addr6));
					// INET6_getsock(addr6, (struct sockaddr *) &remaddr);
					// localaddr.sin6_family = AF_INET6;
					// remaddr.sin6_family = AF_INET6;
					result_addr_local = in6_local;
					result_addr_remote = in6_remote;
					sa_family = AF_INET6;
				}
			} else {
				/* this is an IPv4-style row */
				sscanf(local_addr, "%X", (unsigned int *)&result_addr_local);
				sscanf(rem_addr, "%X", (unsigned int *)&result_addr_remote);
				sa_family = AF_INET;
			}
/*
			char *hashkey = (char *)malloc(HASHKEYSIZE * sizeof(char));
			char *local_string = (char *)malloc(50);
			char *remote_string = (char *)malloc(50);
			inet_ntop(sa_family, &result_addr_local, local_string, 49);
			inet_ntop(sa_family, &result_addr_remote, remote_string, 49);

			snprintf(hashkey, HASHKEYSIZE * sizeof(char), "%s:%d-%s:%d", local_string,
					local_port, remote_string, rem_port);
*/
					
			struct fiveArray *tmp = array;
			for (int j = 0; j < MAX_ARRAY_NUM; j++) {
				if (tmp[j].inode != inode) {
					continue;
				} else {
					tmp[j].local_port = local_port;
					tmp[j].rem_port = rem_port;
					tmp[j].sa_family = sa_family;
					inet_ntop(sa_family, &result_addr_local, tmp[j].local_addr, 49);
					inet_ntop(sa_family, &result_addr_remote, tmp[j].remote_addr, 49);
					break;
				}
			}
//			free(local_string);

//  conninode[hashkey] = inode;

  /* workaround: sometimes, when a connection is actually from 172.16.3.1 to
   * 172.16.3.3, packages arrive from 195.169.216.157 to 172.16.3.3, where
   * 172.16.3.1 and 195.169.216.157 are the local addresses of different
   * interfaces */
   
/*  
  for (class local_addr *current_local_addr = local_addrs;
       current_local_addr != NULL;
       current_local_addr = current_local_addr->next) {
	// TODO maybe only add the ones with the same sa_family 
    snprintf(hashkey, HASHKEYSIZE * sizeof(char), "%s:%d-%s:%d",
             current_local_addr->string, local_port, remote_string, rem_port);
    conninode[hashkey] = inode;
  }
*/
//			free(hashkey);
//			free(remote_string);
			break;
		}
	}
}





/* opens /proc/net/tcp[6] and adds its contents line by line */
int addprocinfo(const char *filename) {
  FILE *procinfo = fopen(filename, "r");

  char buffer[8192];

  if (procinfo == NULL)
    return 0;

  fgets(buffer, sizeof(buffer), procinfo);

  do {
    if (fgets(buffer, sizeof(buffer), procinfo))
      addtoconninode(buffer);
  } while (!feof(procinfo));

  fclose(procinfo);

  return 1;
}
