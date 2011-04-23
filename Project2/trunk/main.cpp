/*
 * main.cpp
 *
 *  Created on: Apr 22, 2011
 *
 */
#include <stdio.h>
#include <pthread.h>
#include "eagles.h"

/**
 * TODO: Pichai
 */
void * babyEagleThreadFunction(void * args)
{

}

int main(int argc, char * argv[])
{
	if (argc != 4)
		printArgumentErrorMessage(argc);

	pots = new bool[argv[1]];
	sem_init(&fullPots, argv[1], 0);
	lockPot = PTHREAD_MUTEX_INITIALIZER;
	lockMother = PTHREAD_MUTEX_INITIALIZER;

	for(i = 0; i < argv[2]; i++)
	{
		pthread_t babyEagleThread;       //thread_id
		//Creating thread properties
		//pthread_t tid;       //thread_id
		pthread_attr_t attr; //thread attributes
		//Get the default attributes
		pthread_attr_init(&attr);
		//Creating thread
		pthread_create(&babyEagleThread, &attr, babyEagleThreadFunction, NULL);
	}

	// TODO: Quy
	int t = argv[3];
	while (t != 0)
	{
		goto_sleep();
		Delay();
		food_ready();
		Delay();
		t--;
	}
	// check that all baby eagles have finished feeding
	// use int sem_getvalue(...) to get value of sem
	return 0;
}
