/* CPSC545  Spring2011 Project 2 
* login: assawaru(login used to submit)
* Linux
* date: 04/27/11
* name: Pichai Assawaruangchai, Quy Le
* emails: assawaru@seattleu.edu, quyvle@gmail.com */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <semaphore.h>

using namespace std;

typedef struct
{
	int potSize;   					// total number of pots
	int nBaby;						// total number of Children 
	int nFeed;     					// number of Mom refill 
	bool *pot;   					// Array of pot 
	int out;        				//pot number that baby eat
	int feedCount;					//number the mom refill all pot
	int eatCount;					//number baby eaten, it means the number of empty pot
	sem_t full;     				//semaphore for number of full pot
	sem_t mutexMom; 				//semaphore using by both Mom and baby to sync when to refill
	pthread_mutex_t m_Pot;			//mutex Pot lock during pot access by either mom or baby
	pthread_mutex_t m_EatCount;		//mutex EatCount, lock for updated value by either mom or baby
	pthread_mutex_t m_Print;		//mutex print, lock for print out the whole message
	pthread_mutex_t m_Wakeupmom;	//motex mom, lock for only one baby can wake mom
}sbuf_t;

sbuf_t shared;

/******************************************************************************  
checkForZeroArgument set the default value equal 10 if user input argument equal 0
******************************************************************************/  
void checkForZeroArgument(int * num) 
{
	if (*num == 0)
	{
		*num = 10;
	}
}

/******************************************************************************  
printArgumentErrorMessage print program usage argument
******************************************************************************/  
void printArgumentErrorMessage(int argc) 
{
	cout << "**** Usage: eaglefeed m n t" << endl;
	cout << "  where m = number of feeding pots" << endl;
	cout << "        n = number of baby eagles" << endl;
	cout << "        t = number of feedings" << endl;
	exit(1);
}

/******************************************************************************  
pthread_sleep takes an integer number of seconds to pause the current thread We 
provide this function because one does not exist in the standard pthreads library. We 
simply use a function that has a timeout.  
******************************************************************************/  
int pthread_sleep (int seconds)  
{  
	pthread_mutex_t mutex;  
	pthread_cond_t conditionvar;  
	struct timespec timetoexpire;  
	if(pthread_mutex_init(&mutex,NULL))  
	{  
		return -1;  
	}  
	if(pthread_cond_init(&conditionvar,NULL))  
	{  
		return -1;  
	}  
	//When to expire is an absolute time, so get the current time and add  
	//it to our delay time  
	timetoexpire.tv_sec = (unsigned int)time(NULL) + seconds;  
	timetoexpire.tv_nsec = 0;  
	return pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);  
}  

/******************************************************************************  
MotherEagleThreadFn 
- Mom will wait untill one of the baby eagle wake her up to refill the pot.
- Using mutex m_Wakeupmom to make sure that while mom is preparing food, no other 
  baby eagle can  wake up her again untill mom goes back to sleep
- After mom refill all the pot, mom will let all boby eagle know and baby can 
  start eating.
******************************************************************************/ 
void *MotherEagleThreadFn(void *arg)
{
	int i, count = 1;
	
	while(shared.feedCount > 0)
	{
		//Wait untill a baby wake her up
		sem_wait(&shared.mutexMom);

		//baby finish all pot 
		if(shared.eatCount >= shared.potSize)
		{
			// Reset the eatCount value to 0
			pthread_mutex_lock(&shared.m_EatCount);
			shared.eatCount = 0; 		
			pthread_mutex_unlock(&shared.m_EatCount);

			// Refill all pot, set value to true. Mother refill the whole pot at once.
			pthread_mutex_lock(&shared.m_Pot);
			for (i=0; i < shared.potSize; i++)
			{
				shared.pot[i] = true;
				sem_post(&shared.full);
			}
			pthread_mutex_unlock(&shared.m_Wakeupmom);
					
			pthread_sleep(1);
			pthread_mutex_lock(&shared.m_Print);
			
			//Mom allow baby eagles to eat.
			cout << "Mother eagle says \"Feeding (" << count << ")\"" << endl;
			pthread_mutex_unlock(&shared.m_Print);
			pthread_mutex_unlock(&shared.m_Pot);

			pthread_mutex_lock(&shared.m_Print);
			cout << "Mother eagle takes a nap." << endl; 
			pthread_mutex_unlock(&shared.m_Print);

			shared.feedCount--;
			count++;
		}
	}
	return NULL;
}

/******************************************************************************  
BabyEagleThredFn 
- Using pthread_mutex_trylock(&shared.m_Wakeupmom) to make sure that only one 
  baby eagle who find out the pot empty can wake mom up to refill
- Baby eagle will wait untile mom refilled and let them eat.
- Baby eagle thread will be terminate when the last pot of the last feeding empty
******************************************************************************/ 
void *BabyEagleThredFn(void *arg)
{
	int i, index;

	index = (long)arg;

	while(1)
	{    
		// Baby eagel chack wherther all pot empty, then it have to wake Mom to refill
		if(shared.eatCount >= shared.potSize)
		{
			if(shared.feedCount <= 0)
			{
				break;
			}

			if(pthread_mutex_trylock(&shared.m_Wakeupmom) == 0)
			{
				pthread_mutex_lock(&shared.m_Print);
				for(i = 0; i <= index; i++)
				{
					cout << " ";
				}

				cout << "Baby eagle " << index+1 << " sees all feeding pots are empty and wakes up the mother." << endl; 
				cout << "Mother eagle is awork by " << index+1 << " and strats preparing the food." << endl; 
				pthread_mutex_unlock(&shared.m_Print);
				
				//Let mom know that all pot empty
				sem_post(&shared.mutexMom);
			}
			else
			{
				continue;
			}
		}


		pthread_sleep(1);
		
		pthread_mutex_lock(&shared.m_Print);
		for(i = 0; i <= index; i++)
		{
			cout << " ";
		}
		cout << "Baby eagle " << index+1 << " is ready to eat." 
			<< "\n     ......" <<endl;
		pthread_mutex_unlock(&shared.m_Print);
		
		sem_wait(&shared.full);
		pthread_mutex_lock(&shared.m_Pot);
		
		//Baby eagle start eating if pots are filled with food
		if(shared.pot[shared.out] == true)
		{
			shared.pot[shared.out] = false;
			pthread_mutex_lock(&shared.m_Print);
			for(i = 0; i <= index; i++)
			{
				cout << " ";
			}
			cout << "Baby eagle " << index+1 << " is eating using feeeding pot " << shared.out 
				<< ".\n     ......" << endl; 
			pthread_mutex_unlock(&shared.m_Print);
			shared.out = (shared.out+1)%shared.potSize;
			
			//eat for a while 
			pthread_sleep(1);
			pthread_mutex_unlock(&shared.m_Pot);

			//Increment the number of eatCount, it means the number of empty pot
			pthread_mutex_lock(&shared.m_EatCount);
			shared.eatCount++;
			pthread_mutex_unlock(&shared.m_EatCount);
			
			pthread_mutex_lock(&shared.m_Print);
			for(i = 0; i <= index; i++)
			{
				cout << " ";
			}
			cout << "Baby eagle " << index+1 << " finishes eating." 
				<< "\n     ......" <<endl;
			pthread_mutex_unlock(&shared.m_Print);
		}
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc != 4) 
	{
		printArgumentErrorMessage(argc);
	}

	pthread_t idMom, idBaby;
	int index;

	// Initialize share variable value 
	shared.potSize = atoi(argv[1]);					// total number of pots
	checkForZeroArgument(&shared.potSize);	
	shared.nBaby = atoi(argv[2]);					// total number of Children 
	checkForZeroArgument(&shared.nBaby);
	shared.nFeed = atoi(argv[3]);    				// number of Mom refill 
	checkForZeroArgument(&shared.nFeed);
	shared.pot = new bool[shared.potSize]; 
	shared.feedCount = shared.nFeed;
	shared.eatCount = shared.potSize;

	cout << "MAIN: There are " << shared.nBaby << " baby eagles, " << shared.potSize 
		<< " feeding pots, and " << shared.nFeed << " feeding." << endl;
	cout << "MAIN: Game starts!!!!!" << "\n     ......" << endl;

	// Initialize mutex
	sem_init(&shared.full, 0, 0);
	sem_init(&shared.mutexMom, 0, 0);
	pthread_mutex_init(&shared.m_Pot, NULL);
	pthread_mutex_init(&shared.m_EatCount, NULL);     
	pthread_mutex_init(&shared.m_Print, NULL);
	pthread_mutex_init(&shared.m_Wakeupmom, NULL);

	// Mother eagel thread
	pthread_mutex_lock(&shared.m_Print);
	cout << "Mother eagle started.\n     ......" << endl; 
	pthread_mutex_unlock(&shared.m_Print);
	pthread_create(&idMom, NULL, MotherEagleThreadFn, (void*)NULL);

	// Baby eagel thread 
	for (index = 0; index < shared.nBaby; index++)
	{
		/* Create a new producer */
		pthread_mutex_lock(&shared.m_Print);
		for(int i = 0; i <= index; i++)
		{
			cout << " ";
		}
		cout << "Baby eagle " << index+1 << " started.\n     ......." << endl; 
		pthread_mutex_unlock(&shared.m_Print);

		pthread_create(&idBaby, NULL, BabyEagleThredFn, (void*)index);
	}

	pthread_join(idMom, NULL);	

	// wait untill Baby finish all the pots them Destroy all semaphore
	while(1)
	{
		if(shared.eatCount >= shared.potSize)
		{
			sem_destroy(&shared.full);
			sem_destroy(&shared.mutexMom);
			pthread_mutex_destroy(&shared.m_Pot);
			pthread_mutex_destroy(&shared.m_EatCount);
			pthread_mutex_destroy(&shared.m_Print);
			pthread_mutex_destroy(&shared.m_Wakeupmom);
		    break;
		}
	}
	return 0;
}
