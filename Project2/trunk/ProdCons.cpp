/* CPSC545 sPRING2011 Project 2
* login: assawaru(login used to submit)
* Linux
* date: 04/27/11
* name: Pichai Assawaruangchai, Quy Le
* emails: assawaru@seattleu.edu, quyvle@gmail.com */

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

typedef struct
{
	bool buf[BUFF_SIZE];   	/* shared var */
	int out;        		//pot number that baby eat
	int feedCount;			//number the mom refill all pot
	int eatCount;			//number baby eat
	sem_t full;     		//semaphore for number of full pot
	sem_t mutexMom; 		//mutex mom
	pthread_mutex_t m_Pot;		//mutex Pot
	pthread_mutex_t m_EatCount;	//mutex eatCount
	//sem_t mutexPot;    		//mutex pot
	//sem_t mutexCount;		//mutext eatCount
}sbuf_t;

sbuf_t shared;

void *Producer(void *arg)
{
	int i;
	pthread_mutex_t m_var;
	pthread_mutex_init(&m_var, NULL);

	while(shared.feedCount > 0)
	{
		//TODO make this section as atomic 
		//pthread_mutex_lock
		//Wait untill a baby wake her up
		sem_wait(&shared.mutexMom);

		//baby finish all pot 
		if(shared.eatCount >= BUFF_SIZE)
		{
			// Reset the eatCount value to 0
			//sem_wait(&shared.mutexCount);
			pthread_mutex_lock(&shared.m_EatCount);
			shared.eatCount = 0; 		
			pthread_mutex_unlock(&shared.m_EatCount);
			//sem_post(&shared.mutexCount);
			
			// Refill all pot, set value to true
			//sem_wait(&shared.mutexPot);
			pthread_mutex_lock(&shared.m_Pot);
			
			pthread_mutex_lock (&m_var);
			for (i=0; i < BUFF_SIZE; i++)
			{
				//sem_wait(&shared.empty);
				shared.buf[i] = true;
				sem_post(&shared.full);
			}
			pthread_mutex_unlock(&m_var);

			cout << "Mother eagle takes a nap" << endl; fflush(stdout);
			shared.feedCount--;
			//sem_post(&shared.mutexPot);
			pthread_mutex_unlock(&shared.m_Pot);
		}
	}
	pthread_mutex_destroy(&m_var);
	return NULL;
}

void *Consumer(void *arg)
{
	//TODO read input from command line "arg"

	int i, index;

	index = (long)arg;

	while(1)
	{    
		// Baby eagel chack wherther all pot empty, then it have to wake Mom to refill
		if(shared.eatCount <= BUFF_SIZE) // && shared.feedCount > 0)
		{
			for(int i = 0; i <= index; i++)
                	{
                        	cout << " ";
                	}
			cout << "Baby eagle " << index+1 << " sees all feeding pots are empty and wakes up the mother." << endl; fflush(stdout);
			cout << "Mother eagle is awork by " << index+1 << " and strats preparing the food" << endl; fflush(stdout);
			sem_post(&shared.mutexMom);
		}
		else if (shared.feedCount <= 0)
		{
			break;
		}

		sem_wait(&shared.full);
		//sem_wait(&shared.mutexPot);
		pthread_mutex_lock(&shared.m_Pot);
		cout << "Baby eagle " << index+1 << " is ready to eat." << "\n     ......" <<endl; fflush(stdout);
		if(shared.buf[shared.out] == true)
		{

			shared.buf[shared.out] = false;
			//printf("--------> [C%d] Consuming %d ...\n", index+1, shared.out); fflush(stdout);
			for(int i = 0; i <= index; i++)
                	{
                        	cout << " ";
                	}
			cout << "----> Baby eagle " << index+1 << " is eating using feeeding pot " << shared.out << "\n     ......" << endl; fflush(stdout);
			shared.out = (shared.out+1)%BUFF_SIZE;
			/* Release the buffer */
			//sem_post(&shared.mutexPot);
			pthread_mutex_unlock(&shared.m_Pot);
			/* Increment the number of eatCount */
			//sem_wait(&shared.mutexCount);
			pthread_mutex_lock(&shared.m_EatCount);
			shared.eatCount++;
			pthread_mutex_unlock(&shared.m_EatCount);
			//sem_post(&shared.mutexCount);
		        for(int i = 0; i <= index; i++)
                	{
                        	cout << " ";
                	}
			cout << "Baby eagle " << index+1 << " finishes eating." << "\n     ......" <<endl; fflush(stdout);
		}
	}
	return NULL;
}

int main()
{
	pthread_t idP, idC;
	int index;

	/* Initialize share variable value */
	shared.feedCount = NITERS;
	shared.eatCount = BUFF_SIZE;

	/* Initialize mutex*/
	sem_init(&shared.full, 0, 0);
	//sem_init(&shared.mutexPot, 0, 1);
	//sem_init(&shared.mutexCount, 0, 1);
	sem_init(&shared.mutexMom, 0, 0);
        pthread_mutex_init(&shared.m_Pot, NULL);
	pthread_mutex_init(&shared.m_EatCount, NULL);        

	// Mother eagel thread
	cout << "Mother eagle started.\n     ......" << endl; fflush(stdout);
	pthread_create(&idP, NULL, Producer, (void*)NULL);

	// Baby eagel thread 
	for (index = 0; index < NC; index++)
	{
		/* Create a new producer */
		for(int i = 0; i <= index; i++)
		{
			cout << " ";
		}
		cout << "Baby eagle " << index+1 << " started.\n     ......." << endl; fflush(stdout);

		pthread_create(&idC, NULL, Consumer, (void*)index);
	}

	pthread_join(idP, NULL);	

	// Destroy all semaphore
	sem_destroy(&shared.full);
	sem_destroy(&shared.mutexMom);
	//sem_destroy(&shared.mutexPot);
	//sem_destroy(&shared.mutexCount);
	pthread_mutex_destroy(&shared.m_Pot);
	pthread_mutex_destroy(&shared.m_EatCount);
	
}

