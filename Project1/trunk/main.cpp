/* CPSC545 Spring2011 Project 1
* login: masuij(login used to submit)
* Linux
* date: 03/28/11
* name: Justin Masui, Pichai Assawaruangchai, Quy Le
* emails: veks11@gmail.com,assawaru@seattleu.edu,quyvle@gmail.com */

/*  
Basic idea is that each node's information is added ot the following maps

name            key     value
--------------- ---     --------------------
allNodesMap     id      node
numParents      id      numParents
 
-  if numParents[id]=0, then the node is added to ready queue. 
-  when each node completes execution, for each element in allNodesMap[id].children the numParents[child] is decremented 1.  If this value is equal to 0 then move to ready queue.
-  execute all jobs in ready queue, move running jobs to finishq
-  repeat until all jobs in finishq

This gives an O(V+E) because populating the numParents is O(V+E).  Otherwise, every other operation is O(V).  This is equivalent complexity to a Topological sort, but unlike topological support it allows for execution parallelism in the process dependencies.
*/


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
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "graphexe.h"

#define MAX_ARGUMENTS 10
#define BUFFER_SIZE 1024

using namespace std;

/*--------------------------
Holds all nodes for easy acces
key=Node->id, value=Node 
----------------------------*/
map<int, Node> allNodesMap; 

/*--------------------------
Map of node to numberOfRemainingParents
key=node, value=number of parents
----------------------------*/
map<int,int> numParents; 

/*--------------------------
Map of pid to Node ID
key=os pid, value=line number
----------------------------*/
map<pid_t,int> pidToNodeIDMap;

/*--------------------------
System Queues
----------------------------*/
priority_queue<int> readyq; //corresponds to NodeID's
priority_queue<int> ineligibleq; //corresponds to NodeID's, not really used
queue<int> finishedq; //corresponds to NodeID's
queue<pid_t> runningq;
queue<pid_t> checkedq;

/*----------------------------
functions 
----------------------------*/
/* 
For a given node updates numParents Map.  If a node has 2 children then each children's number of parents is iterated by 1.
*/
void addNodeToNumParents(Node node);

/* 
For each process get state.  If state is exited/signaled/killed handle and place in appropriate queue
*/
void checkAndHandleRunningqState();

/*
Called by checkAndHandleRunningqState.  For a give finished pid update:  
	numParents
	readyq
	finishq
	AllNodesMap
*/
void handleFinishedPid(pid_t finishedPid);

/*
Each check of a state moves a pid from runningq to checkedq, this function resets the runningq from the checkedq
*/
void resetRunningq();


int
main(int argc, char *argv[])
{
    if (argc != 2) {
    cerr << "Invalid command line - usage: <input file>" << endl;
    exit(-1);
    }

    // Open the input file.
    ifstream ifile(argv[1]);

    //vars for parsing
    string line;
    int lineNum=-1;
	int numNodes=0;

	Node rNode; //result node
	//------------------------
	//parse each line of the file into a node and add to data structures
    //example line:  ls -l:1 2:blank-file.txt:ls-output.txt
	//------------------------
    while(!ifile.eof()) {
		lineNum++;
        getline(ifile, line);
        if(line.length()>1023){
            cout<<"Line too long, ignoring..."<<line.length()<<endl;
            continue;
        }
		
        rNode=createNode(line,lineNum);//creates node from a line with id=lineNum    
		if(rNode.id==-99) continue;  //if rNode is valid, skip current loop
		
        //Contains all nodes key=id, value=node
        allNodesMap[lineNum]=rNode;
		
		//populate numParents     
		addNodeToNumParents(rNode);
		
		//increase numNodes
		numNodes++;
	}	

    map<int, int>::const_iterator iter;
    cout<<"-----------------------"<<endl;
    cout<<"node numberParents"<<endl;
    for(iter=numParents.begin(); iter!= numParents.end(); ++iter){
        cout<<iter->first<<"    "<<(iter->second)<<endl;
    }   
	cout<<"Total Nodes Read="<<numNodes<<endl;
	cout<<"-----------------------"<<endl;
    //iterate through nodes
	//update to status
	//add to readyq
	for(iter=numParents.begin(); iter!= numParents.end(); ++iter){
        if(iter->second == 0){
			allNodesMap[iter->first].status=READY;
			readyq.push(iter->first);
			cout<<"adding node "<<iter->first<<" to readyq"<<endl;
		}else{
			allNodesMap[iter->first].status=INELIGIBLE;
			ineligibleq.push(iter->first);
			cout<<"adding node "<<iter->first<<" to ineligibleq"<<endl;
		}
    }   
	
	/*------------------------
	Main loop for executing jobs
	------------------------*/

	while((int)finishedq.size()<numNodes){

		while(!readyq.empty()){
			int r=readyq.top();
			readyq.pop();
			Node runningNode=allNodesMap[r];

			pid_t p;

			p=fork();
			//error
			if (p<0){
				printf("Fork failed");
			}
		
			//child
			if(p==0) {
            	char * cmd[MAX_ARGUMENTS];
				char programPlusArguments[1024*3];
				strcpy(programPlusArguments, runningNode.prog);
				char * tok = strtok(programPlusArguments, " ");
				int i;
				for(i = 0; tok != NULL && i < MAX_ARGUMENTS; i++) {
					cmd[i] = tok;
					tok = strtok(NULL, " ");
 				}

				// replace last command with NULL if i = MAX_ARGUMENTS
				if (i == MAX_ARGUMENTS) {
					cmd[i-1] = NULL;
				} else {
					cmd[i] = NULL;
				}

				int inputFile = open(runningNode.input, O_RDONLY | O_CREAT, 0644);
				if (inputFile == -1) {
					char errorMsg[1024];
					createOpeningFileErrorMessage(errorMsg, "input", &runningNode);
					exit(1);
				}

				int dup2Code_stdin = dup2(inputFile, 0);
				if (dup2Code_stdin == -1) {
					perror("Unable to duplicate STDIN file descriptor");
					// should we exit?
				}

				if (close(inputFile) == -1) {
					perror("Error closing input file");
				}
				int outputFile = open(runningNode.output, O_WRONLY | O_CREAT, 0644);
				if (outputFile == -1) {
					char errorMsg[1024];
					createOpeningFileErrorMessage(errorMsg, "output", &runningNode);
					exit(1);
				}

				int dup2Code_stdout = dup2(outputFile, 1);
				if (dup2Code_stdout == -1) {
					perror("Unable to duplicate STDOUT file descriptor");
					// should we exit?
				}
				if (close(outputFile) == -1) {
					perror("Error closing output file");

				}
				execvp(cmd[0], cmd);
			} 
		
			//parent
			if(p>0){
				pidToNodeIDMap[p]=r;
				runningq.push(p);
				cout<<"Forked NodeID "<<r<< " as pid "<<p<<endl;
			}
			
		}//end of while(!readyq.empty())
		
		//checks each pid in runningq for status and updates other queues
		checkAndHandleRunningqState();
				
		//copy checkedq back to runningq
		resetRunningq();
		
	}//end of while((int)finishedq.size()<numNodes)
		
	printf("finishq.size()=%d, numNodes=%d\n",(int)finishedq.size(),numNodes);
}//end of main

void addNodeToNumParents(Node node){

	//check to see if node in NumParents
	if(numParents.count(node.id)==0){
		numParents[node.id]=0;
		//cout<<"adding child: "<<node.id<<" to numParents"<<endl;
	}
	
	//if there no valid children quit function
	if(node.children[0]==-99) return;  

	//add to numParents if not already in numParents	
	//increment each node.children[i]'s number of Parents by 1
	for(int i=0;i<(int)(node.children).size();i++){
		if(numParents.count(node.children[i])==0){
			numParents[node.children[i]]=0;
			//cout<<"adding child: "<<node.children[i]<<" to numParents"<<endl;
		}
		int tmp=numParents[node.children[i]];
		tmp++;
		numParents[node.children[i]]=tmp;
		//cout<<"numParents["<<node.children[i]<<"] = "<<numParents[node.children[i]]<<" to numParents"<<endl;
	}		
}

void checkAndHandleRunningqState(){
	//wait for pid to finish
	pid_t testPID;
	//printf("Checking PID Status, runningq.size=%d\n",(int)runningq.size());
	while(!runningq.empty()){
		int status;
		testPID=runningq.front();
		//printf("checking pid=%d,runningq.size()=%d\n", testPID,(int)runningq.size());

		pid_t statusPid=waitpid(testPID, &status, WNOHANG);

		if(statusPid==0) {
			//printf("status not available for pid=%d, continue\n", testPID);
			//if status not available move pid to checkedq
			checkedq.push(runningq.front());
			runningq.pop();
			continue;
		}
		
		if(statusPid==-1) {
			printf("signal delivered to pid=%d, Considering finished\n", testPID);
			runningq.pop();//remove from runningq
			handleFinishedPid(statusPid);
			continue;
		}
		
		//statusPid > 0
		if (WIFEXITED(status)) {
			printf("child exited, WIFEXITED(status)=%d,pid=%d,finished\n", WEXITSTATUS(status),testPID);
			runningq.pop();//remove from runningq
			handleFinishedPid(statusPid);
			continue;

		} else if (WIFSIGNALED(status)) {
			printf("child killed (signal %d),pid=%d,Considering finished\n", WTERMSIG(status),testPID);
			runningq.pop();//remove from runningq
			handleFinishedPid(statusPid);

		} else if (WIFSTOPPED(status)) {
			printf("child stopped (signal %d),pid=%d,Considering finished\n", WSTOPSIG(status),testPID);
			runningq.pop();//remove from runningq
			handleFinishedPid(statusPid);
			
		} else if (WEXITSTATUS(status)) {
			printf("child exited, WEXITSTATUS(status)=%d,pid=%d,Considering finished\n", WEXITSTATUS(status),testPID);
			runningq.pop();//remove from runningq
			handleFinishedPid(statusPid);			
		} else {
			printf("unhandled status=%d,pid=%d,Considering finished\n",status,testPID);
		}
		//TODO:  handle if process doesn't terminate
	}   
}

void handleFinishedPid(pid_t finishedPid){
	//update numParents for each of the finishedNodeID's children
	int finishedNodeID=pidToNodeIDMap[finishedPid];
	Node finishedNode=allNodesMap[finishedNodeID];
	if(finishedNode.children[0]!=-99){
		for(int i=0;i<(int)(finishedNode.children).size();i++){
			int childToUpdate=finishedNode.children[i];

			//update numParents for a given child
			int remainingParents=numParents[childToUpdate];//key=node, value=number of remaining Parents
			remainingParents--;
			numParents[childToUpdate]=remainingParents;

			//update readyq
			if(remainingParents==0){
				readyq.push(childToUpdate);
				cout<<"adding "<<childToUpdate<<" to readyq"<<endl;
			}
		}
	}
	
	//update finishq
	finishedq.push(finishedNodeID);
	cout<<"adding "<<finishedNodeID<<" to finishedq"<<endl;
	cout<<"finishedq.size()="<<finishedq.size()<<endl;

	//update allNodesMap
	Node updateNode=allNodesMap[finishedNodeID];
	updateNode.status=FINISHED;
	allNodesMap[finishedNodeID]=updateNode;
	
	//TODO:  update ineligible queue, not critical since not really used
}

void resetRunningq(){
	//printf("copying checkedq(%d) to runningq(%d)\n",(int)checkedq.size(),(int)runningq.size());
	while(!checkedq.empty()){
		pid_t tmp=checkedq.front();
		runningq.push(tmp);
		//printf("copy pid %d\n", checkedq.front());
		checkedq.pop();
	}
}
