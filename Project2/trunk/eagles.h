/*
 * eagles.h
 *
 *  Created on: Apr 22, 2011
 *
 */

#ifndef EAGLES_H_
#define EAGLES_H_
#include <pthread.h>

pthread_mutex_t lockPot;
pthread_mutex_t lockMother;
sem_t totalPots;

bool * pots;

void printArgumentErrorMessage(int argc) {
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
 *****************************************************************************/
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

#endif /* EAGLES_H_ */
