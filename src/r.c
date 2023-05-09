#include "func.h"

/*
 * Reader program:
 *	- Listen for Ctrl+C signal and kill other process
 *	- Activate Writer 2 program
 *	- Attach to shared memory region
 *	- Open semaphore
 *	- On every 2 seconds count number of letters
 *	- On every 10 seconds display stats
 *	- on Ctrl+C end w1 and w2 program and diplay stats last time
 */

struct shared_memory_region	*shm;
sem_t				*sem;
int				id,pidw1,pidw2;
int				count[20];


void
handler(int s);

void
calculate();

void
display_stats();


int
main(int argc,char *argv[])
{
	signal(SIGINT,handler);
	char	shm_id[8];	
	int	turn=0;
	
	kill(getppid(),SIGCONT);

	id=atoi(argv[3]);

	sprintf(shm_id,"%d", id);

	pidw2=atoi(argv[1]);
	pidw1=atoi(argv[2]);

	shm=(struct shared_memory_region*)attach(id);

	sem=sem_open(semname, 0);

	while(1){	

		//first sleep a little bit
		sleep(2);

		//enter critical section
		sem_wait(sem);

		calculate();
		
		turn++;

		if(turn==5){
			//display and reset
			display_stats();
			turn=0;
		}
		
		sem_post(sem);
	}
	
	return 0;

}

void
handler(int s)
{
	/*
	 * To do:
	 *	-kill W1 and W2
	 *	-display status again
	 *	-close and destroy semapore and shared memory segment
	 */
	 kill(pidw1,SIGINT);

	 kill(pidw2,SIGINT);

	 //clear the screen-Comment next line for easier debug
	 system("clear");

	 //display stats for the last time
	 display_stats();

	 //wait a little bit while w1 and w2 exit
	 sleep(1);

	 //close semaphore
	 sem_close(sem);

	 //destroy semaphore
	 sem_unlink(semname);

	 //destroy shared memory segment
	 shmctl( id, IPC_RMID,NULL);

	 //exit
	 printf("Bye,bye!\n");
	 exit(EXIT_SUCCESS);

}

void calculate()
{

	int        n,r,w,pos;


	r=shm->roff;
	w=shm->woff;

	n=READ_COUNT;


	
	if(r==256){
		shm->roff=0;
		r=0;
	}

	if(w>r)
		n=( r+n-1<w)? n : w-r;
	else if( w==0 && w==r && (shm->buf)[0]=='.')
		n=0;
	else
		;
	//use for debug and testing
	//printf("[R]{B}w: %d r: %d n: %d\n", w,r,n);
	if(r==0 && n>0){
		pos=((shm->buf)[0])-65;
		count[pos]++;
		(shm->buf)[0]='.';
		n--;
		r++;
	}
	while(n>0){
		pos=((shm->buf)[r])-65;
		count[pos]++;
		n--;
		r++;
	}
	shm->roff=r+n;
	//use for debug and testing
	//printf("[R]{A} w: %hi r:%hi\n", shm->woff,shm->roff);
}

void display_stats()
{
	printf("\n");
	int	i,d1,d2,d3,j,x;
	for(i=0;i<20;i++){

		x=count[i];

		printf("%c-%03d ",i+65,x);

		d1=x/100;
		x=x%100;

		d2=x/10;
		x=x%10;

		d3=x%100;

		if(d1){
			for(j=0;j<d1;j++)
				printf("*");
		}
		if(d2){
			for(j=0;j<d2;j++)
				printf("+");
		}
		if(d3){
			for(j=0;j<d3;j++)
				printf("-");
		}

		printf("\n");
	}
	printf("\n\n");
}
