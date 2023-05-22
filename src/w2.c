#include "func.h"

/*
 * Plan for W2:
 *	- Spawn new proces => reader(Send him pid of W1 and W2)
 *	- Wait for signal that reader is spawned
 *	- Attach shared memory segment
 *	- Open semaphore
 *	- Start producing data
 */

sem_t				*sem;

struct shared_memory_region	*shm;




//singal handler functions

void
inthandler(int s);

void
conthandler(int s);

void
produce_data();

int
main(int argc,char *argv[])
{
	signal(SIGCONT, conthandler);
	signal(SIGINT, inthandler);

	int	check;
	int	mpid,ppid, id;
	char	shm_id[8];
	pid_t	p;

	mpid=getpid();

	ppid=getppid();

	id=atoi(argv[1]);

	strncpy(shm_id, argv[1], strlen(argv[1]));


	p=fork();

	if(p<0){
		//error
		printf("Manualy remove shared memory segment and named semaphore!\n");
		
		//kill parent [W1]
		kill(ppid,SIGINT);
		report_err("fork(): ");
				
	}else if(p>0){
		pause();

		//attach shared memory
		shm=(struct shared_memory_region*)attach(id);
		if(*(int*)shm==-1){
			printf("Manualy remove shared memory segment and named semaphore!\n");
			kill(ppid,SIGINT);
			report_err("attach: ");
		}
		//open semaphore
		sem=sem_open(semname,0);
		if(sem==SEM_FAILED){
			printf("Manualy remove shared memory segment and named semaphore!\n");
			kill(ppid,SIGINT);
			report_err("sem_open: ");
		}
		//start producing data
		produce_data();
	}else{
		//child-spawn new process
		char	mpidarg[16];
		char	ppidarg[16];

		sprintf(mpidarg,"%d", mpid);
		sprintf(ppidarg,"%d", ppid);

		char *arg[]={ "./r", mpidarg,ppidarg, shm_id, NULL };

		check=execvp(arg[0], arg);
		if(check==-1){
			kill(ppid,SIGINT);
			printf("Manualy remove named semaphore and destroy shm region!\n");
			report_err("R spawn failed: ");
		}
	}

	return 0;

}

void inthandler(int s)
{
	
	//terminate program
	sem_close(sem);
	shmdt(shm);
	exit(EXIT_SUCCESS);
}
void conthandler(int s)
{
	return;
}

void produce_data()
{
	/*
	 * Code for writer 2
	 * He will produce and write  random letter every 50 miliseconds
	 */
	int	off=65,n,w,r,ms=50;
	struct	timeval	tv;
	tv.tv_sec=0;
	srand(time(NULL));
	while(1){

		tv.tv_usec=(ms % 1000) * 1000;

		sem_wait(sem);
		
		w=shm->woff;

		r=shm->roff;

		n=((w)< BLOCK_SIZE) ? 1 : 0;
		if(n){
			if(w==0){
				if((shm->buf)[0]=='.')
					;
				else
					n=0;
			}else if(w>r || w<r)
				;
			else
				n=0;
			//Use for DEUBG
			//printf("[W2]{B}w: %d r: %d n:%d\n",w,r,n);
			if(n){
				//write in buf
				(shm->buf)[w]=off+rand()%20;
				w++;
			}
		}

		//update write offset position
		if(w>=BLOCK_SIZE)
			shm->woff=0;
		else
			shm->woff=w;
		//Debug
		//printf("[W2]{A}w:%hi r:%hi\n",shm->woff,shm->roff);
		//exit critical section
		sem_post(sem);

		//wait for 1/20 seconds
		select(0,NULL,NULL,NULL,&tv);
	}

	
}
