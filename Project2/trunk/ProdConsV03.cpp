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
	int out;        	//pot number that baby eat
	int iter;		//number the mom refill all pot
	int eatCount;		//number baby eat
	sem_t full;     	//semaphore for number of full pot
	sem_t empty;		//semaphore for number of empty pot
	sem_t mutexPot;    	//mutex pot
	sem_t momLock; 		//mutex mom
	sem_t mutexCount;	//mutext eatCount
}sbuf_t;

sbuf_t shared;

void *Producer(void *arg)
{
	//sem_wait(&shared.momLock);
	int i;
	int fullPot;

	while(shared.iter > 0)
	{
		//sem_getvalue(&shared.full, &fullPot);
		// TODO make this section as atomic 

		//Wait untill a baby wake her up
		sem_wait(&shared.momLock);
		
		//baby finish all pot 
		if(shared.eatCount >= BUFF_SIZE)
		{
			// Reset the eatCount value to 0
			sem_wait(&shared.mutexCount);
			shared.eatCount = 0; 		//reset eatCount to 0
			sem_post(&shared.mutexCount);
			
			// Refill all pot, set value to true
			sem_wait(&shared.mutexPot);
			cout << "Mother eagle is awork and strats preparing the food" << endl;
			for (i=0; i < BUFF_SIZE; i++)
			{
				//sem_wait(&shared.empty);
				shared.buf[i] = true;
				sem_post(&shared.full);
			}
			
			cout << "Mother eagle takes a nap" << endl;
			shared.iter--;
			sem_post(&shared.mutexPot);
		}
	}
	return NULL;
}

void *Consumer(void *arg)
{
	int i, item, index, emptyPot;

	index = (long)arg;

	while(1)
	{    
		// Baby eagel chack wherther all pot empty, then it have to wake Mom to refill
                if(shared.eatCount <= BUFF_SIZE)
                {
                        if(shared.iter <= 0)
                        {
                                //exit(1);
                                //pthread_exit(NULL); 
                                break;
                        }
                        cout << "Baby eagle " << index+1 << " sees all feeding pots are empty and wakes up the mother." << endl;
                        sem_post(&shared.momLock);
                }
		else
		{

			sem_wait(&shared.full);
			sem_wait(&shared.mutexPot);
			//item = shared.buf[shared.out];
			if(shared.buf[shared.out] == true)
			{

				shared.buf[shared.out] = false;
				printf("--------> [C%d] Consuming %d ...\n", index+1, shared.out); fflush(stdout);
				shared.out = (shared.out+1)%BUFF_SIZE;
				/* Release the buffer */
				sem_post(&shared.mutexPot);
				/* Increment the number of eatCount */
				sem_wait(&shared.mutexCount);
                        	shared.eatCount++;
				sem_post(&shared.mutexCount);
 			}
		}
		
	}
	//cout << "-------------------------" << endl;
	return NULL;
}

int main()
{
	pthread_t idP, idC;
	int index;

	/* Initialize share variable value */
	shared.iter = NITERS;
	shared.eatCount = BUFF_SIZE;

	/* Initialize mutex*/
	sem_init(&shared.full, 0, 0);
	sem_init(&shared.empty, 0, BUFF_SIZE);
	sem_init(&shared.mutexPot, 0, 1);
	sem_init(&shared.mutexCount, 0, 1);
	sem_init(&shared.momLock, 0, 0);

	// Mother eagel thread
	cout << "Mother eagle started." << endl; 
	pthread_create(&idP, NULL, Producer, (void*)NULL);

	// Baby eagel thread 
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

	// Destroy all semaphore
	sem_destroy(&shared.full);
	sem_destroy(&shared.empty);
	sem_destroy(&shared.mutexPot);
	sem_destroy(&shared.momLock);
	sem_destroy(&shared.mutexCount);
	pthread_exit(NULL);
}

