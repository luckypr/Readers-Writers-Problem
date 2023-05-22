 #include "func.h"

/*
 * So this is Writer -1.
 * He will:
 *	-Create shared memory segment,attach to it and initalize with values.
 *	-Create semaphore and give him value
 *	-Run writer 2 program [W2]
 *	-Produce data
 */

void
handler(int s);

void
produce_data();

//pointer to shared memory segment
struct shared_memory_region	*shm;

//pointer to named semaphore
sem_t				*sem;


int
main(int argc,char *argv[])
{
	signal(SIGINT, handler);
	/* variables to use */
	int	*id,check;
	char	shm_id[8];
	pid_t	p;
	key_t key;
	explicit_bzero(shm_id,sizeof(shm_id));
	

	//create and attach to shared memory segment

	key=ftok(keyname,0);
	
	if(key==-1)
		report_err("ftok(): ");
	
	id=create_shm(key,sizeof(struct shared_memory_region));
	if(*id==-1)
		report_err("Failed to create shared memory segment: ");

	//save shared memory id in string to pass for W2
	sprintf(shm_id, "%d", *id);	
	

	//attach to shared memory 	
	shm=(struct shared_memory_region*)attach(*id);
	if(*(int*)shm == -1){
		printf("Error happend!\n");
		report_err("Failed to attach shared memory segment: ");
	}
	//set shared memory default values
	shm->roff=0;
	shm->woff=0;
	explicit_bzero(shm->buf,sizeof(shm->buf));
	(shm->buf)[0]='.';

	//create semaphore
	sem=sem_open(semname,O_CREAT, 0666,1);
	if(sem==SEM_FAILED){
		printf("Manualy delete named semaphore and destroy shared memory segment!\n");
		report_err("Error creating named semaphore: ");
	}

	//time to spawn W2
	p=fork();

	if(p==0){
		/* Here spawn new proces [W2] */	
		char	*arg[]={ "./w2", shm_id,NULL };
		check=execvp(arg[0],arg);
		if(check==-1){
			printf("Manualy remove named semaphore and destroy shm region!\n");
			report_err("W2 spawn failed: ");
		}
	}else if(p>0){
		/* Generate 20 characters on every 2 seconds */
		produce_data();
	}else{
		/* ERROR HAPPENED! HANDLE IT!!! */
		printf("Error on fork()!\nYou have to manualy delete named semaphore and shm segment!\n");
		report_err("fork(): ");
	}
	

	return 0;
}

void
handler(int s)
{
	//do the stuff for exit
	sem_close(sem);

	shmdt(shm);

	exit(EXIT_SUCCESS);
}


void
produce_data()
{
	srand(time(NULL));
	char towrite[20];
	int  off=65;
	int  w,r,n,i,max_to_gen=20;
	while(1){

		/*
		 * Generate 20 random letter and write them in bufer
		 * Do that on every 2 seconds
		 */
		
		//empty temp buf

		explicit_bzero(towrite, 20);
		
		//enter the critical section
		sem_wait(sem);		

		w=shm->woff;

		r=shm->roff;	

		n=( (w+max_to_gen-1) < BLOCK_SIZE) ? max_to_gen: (BLOCK_SIZE - w);
		if(n>0){
			//we can write in bufer
			
			if(w==0){
				if( (shm->buf)[0]=='.')
					n=n;
				else
					n=0;
			}else if( w>r ){
				//okay write n items do not change anything;
				;
			}else if(w<r){
				//do not overwrite read off
				n=( (w+n-1) < r ) ? n : r-w;
			}else
				n=0;
			//FOR DEBUG
			//printf("[W1]{B} w:%d r:%d n:%d\n",w,r,n);
			//generate n random letters and write them in buf
			for(i=0;i<n;i++){
				(shm->buf)[w]=off+(rand()%20);
				w++;
			}
		}
		if(w>=BLOCK_SIZE)
			shm->woff=0;
		else
			shm->woff=w;
	
		//USe for debug
		//printf("[W1]{A} w:%hi r:%d\n",shm->woff ,shm->roff);
		//exit the critical section
		sem_post(sem);
		//sleep for 2 sec
		sleep(2);
		
	}
}
