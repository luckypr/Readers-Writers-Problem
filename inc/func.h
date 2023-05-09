#ifndef FUNC_H
#define FUNC_H

//what we need to include

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>

//define usefull data

#define		keyname		"w1"
#define		BLOCK_SIZE	256
#define		semname		"mysem"
#define		READ_COUNT	32


struct shared_memory_region{

	short	roff;
	short	woff;
	char	buf[BLOCK_SIZE];
};



//define usefull functions here

int *create_shm(key_t k, size_t size);

struct shared_memory_region *attach(int id_shm);



void report_err(char *on);


#endif
