#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <cstring>
#include <map>
#include <queue>
#include <sys/wait.h>

#include "graphexe.h"
using namespace std;


int
main(int argc, char *argv[])
{
	while(1){
		pid_t p;
		//p=fork();

		//error
		if (p<0){
			printf("Fork failed");
		}

		//child
		if(p==0) {
			char programPlusArguments[1024];
			char arg[1024];
			char args_list[50][100];
			strcpy(programPlusArguments, runningNode.prog);
			char * tok = strtok(programPlusArguments, " ");
			while (tok != NULL) {
				cout << tok << endl;
				tok = strtok(NULL, " ");
			}
			char *arg_list[64];	
			execvp(arg_list[0], arg_list);
		}

		//parent
		if(p>0){
			pidToNodeIDMap[p]=r;
			cout<<"Forked NodeID "<<r<< " as pid "<<p<<endl;
		}
	}
}