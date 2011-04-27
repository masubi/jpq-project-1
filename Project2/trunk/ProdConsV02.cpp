#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include </usr/include/semaphore.h>

#define BUFF_SIZE   5		/* total number of pots*/
#define NP          1		/* total number of Mom */
#define NC          8		/* total number of Children */
#define NITERS      4		/* number of Mom refill */

using namespace std;

typedef struct {
	bool buf[BUFF_SIZE];   	/* shared var */
	int out;        	  	/* buf[out%BUFF_SIZE] is the first full slot */
	int iter;
	sem_t full;     	  	/* keep track of the number of full spots */
	sem_t empty;
	sem_t potLock;    		/* enforce mutual exclusion to shared data */
	sem_t momLock;
}sbuf_t;

sbuf_t shared;

void *Producer(void *arg)
{
	//sem_wait(&shared.momLock);
	int i;
	int fullPot;

	while(shared.iter > 0)
	{
		sem_getvalue(&shared.full, &fullPot);
		if(fullPot <= 0)
		{
			sem_wait(&shared.potLock);

			cout << "Mother eagle is awork and strats preparing the food" << endl;
			for (i=0; i < BUFF_SIZE; i++)
			{
				//sem_wait(&shared.empty);
				shared.buf[i] = true;
				sem_post(&shared.full);
			}
			
			cout << "Mother eagle takes a nap" << endl;
			shared.iter--;
			sem_post(&shared.potLock);
		}
		sem_wait(&shared.momLock);
	}
	return NULL;
}

void *Consumer(void *arg)
{
	int i, item, index, emptyPot;

	//int *temp = reinterpret_cast<int*>(arg);
	//index = *temp;
	//delete temp;

	index = (long)arg;

	while(1)
	{    

		/* Fill in the code here */  
		sem_wait(&shared.full);
		//sem_post(&shared.empty);
		sem_wait(&shared.potLock);
		//item = shared.buf[shared.out];
		shared.buf[shared.out] = false;
		printf("--------> [C%d] Consuming %d ...\n", index+1, shared.out); fflush(stdout);
		shared.out = (shared.out+1)%BUFF_SIZE;
		/* Release the buffer */
		sem_post(&shared.potLock);
		/* Increment the number of full slots */
		//sem_wait(&shared.lockMom);
		sem_getvalue(&shared.full, &emptyPot);
		if(emptyPot == 0)
		{
			if(shared.iter <= 0)
			{
				//exit(1);
				//pthread_exit(NULL); 
				break;
			}
			//cout << "Baby eagle " << index+1 << " sees all feeding pots are empty and wakes up the mother." << endl;
			sem_post(&shared.momLock);	
		}
	}
	//cout << "-------------------------" << endl;
	return NULL;
}

int main()
{
	pthread_t idP, idC;
	int index;

	shared.iter = NITERS;

	sem_init(&shared.full, 0, 0);
	sem_init(&shared.empty, 0, BUFF_SIZE);

	/* Insert code here to initialize mutex*/
	sem_init(&shared.potLock, 0, 1);
	sem_init(&shared.momLock, 0, 0);

	cout << "Mother eagle started." << endl; 
	pthread_create(&idP, NULL, Producer, (void*)NULL);

	/* Insert code here to create NC consumers */
	for (index = 0; index < NC; index++)
	{
		/* Create a new producer */
		//cout << "Create child therd " << index << endl;
		for(int i = 0; i < index; i++)
		{
			cout << " ";
		}
		cout << "Baby eagle " << index+1 << " started." << endl;

		pthread_create(&idC, NULL, Consumer, (void*)index);
	}

	sem_destroy(&shared.full);
	sem_destroy(&shared.empty);
	sem_destroy(&shared.potLock);
	sem_destroy(&shared.momLock);
	pthread_exit(NULL);
}

