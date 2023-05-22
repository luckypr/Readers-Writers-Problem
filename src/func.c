#include "func.h"


int *create_shm(key_t k, size_t size)
{
	int *ret=(int*)malloc(sizeof(int));
	*ret=shmget(k, size, 0666| IPC_CREAT);
	return ret;	
}
struct shared_memory_region *attach(int id)
{
	return shmat(id, NULL,0);
}


void report_err(char *on)
{
	perror(on);
	exit(1);
}
