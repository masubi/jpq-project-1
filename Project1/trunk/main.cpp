/* CPSC545 Spring2011 Project 1
* login: masuij(login used to submit)
* Linux
* date: 03/28/11
* name: Justin Masui, full_name2 (for partner(s))
* emails: masuij@seattleu.edu */

/*  
Basic idea is that each node's information is added ot the following maps

name            key     value
allNodesMap     id      node
parentToChild   id      vector<int> children
numParents      id      numParents
 
-  if numParents[id]=0, then the node is added to ready queue. 
-  when each node completes execution, for each element in allNodesMap[id].children the numParents[child] is decremented 1.  If this value is equal to 0 then move to ready queue.
-  execute all jobs in ready queue, move running jobs to finishq
-  repeat until all jobs in finishq

I think this gives an O(V+E) because populating the numParents is O(V+E).  Otherwise, every other operation is O(V).  This is equivalent complexity to a Topological sort, but unlike topological support it allows for parallelism in the task dependencies.
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

#include "graphexe.h"
#define BUFFER_SIZE 1024

using namespace std;

/*--------------------------
Holds all nodes for easy acces
key=Node->id, value=Node 
----------------------------*/
map<int, Node> allNodesMap; 

/*--------------------------
contains map of node to numberOfRemainingParents
key=node, value=number of parents
----------------------------*/
map<int,int> numParents; 

//key=os pid, value=line number
map<pid_t,int> pidToNodeIDMap;

/*--------------------------
queue
----------------------------*/
priority_queue<int> readyq;
priority_queue<int> ineligibleq;
queue<int> finishedq;
queue<pid_t> runningq;
queue<pid_t> checkq;


/*----------------------------
populates the numParents 
----------------------------*/
void addNodeToNumParents(Node node);
void handleFinishedPid(pid_t finishedPid);

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
	
    //iterate through nodes
	//update to status
	//add to readyq
	for(iter=numParents.begin(); iter!= numParents.end(); ++iter){
        if(iter->second == 0){
			allNodesMap[iter->first].status=READY;
			readyq.push(iter->first);
			cout<<"adding "<<iter->first<<" to readyq"<<endl;
		}else{
			allNodesMap[iter->first].status=INELIGIBLE;
			ineligibleq.push(iter->first);
			cout<<"adding "<<iter->first<<" to ineligibleq"<<endl;
		}
    }   
	
	/*------------------------
	Main loop for executing jobs
	------------------------*/

	while((int)finishedq.size()<numNodes){

		while(!readyq.empty()){
		
			//assume readyq not empty?
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
				char *read_line=(char *)malloc(BUFFER_SIZE);
				char *arg_list[BUFFER_SIZE];
				int background;
				
				strncpy(read_line,runningNode.prog,BUFFER_SIZE);
				
				int cnt=0;
				arg_list[cnt]=strtok(read_line," \r\n");
				//parse into arg_list
				while(arg_list[cnt]!=NULL)
				 {
				   //printf("parsed %s\n",arg_list[cnt]);
				   arg_list[++cnt]=strtok(NULL," \r\n");
				 }
				//handle background functionality
				if(arg_list[0]!=NULL && *arg_list[cnt-1]=='&'){
					background=1;
					arg_list[cnt-1]=NULL;
				}else{
					background=0;
				}

				execvp(arg_list[0], arg_list);
			}
		
			//parent
			if(p>0){
				pidToNodeIDMap[p]=r;
				runningq.push(p);
				cout<<"Forked NodeID "<<r<< " as pid "<<p<<endl;
			}
			
		}//end of while(!readyq.empty())
		
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
				//if status not available move pid to checkq
				checkq.push(runningq.front());
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
				printf("child exited, status=%d,pid=%d,Considering finished\n", WEXITSTATUS(status),testPID);
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
				printf("child , status=%d,pid=%d,Considering finished\n", WEXITSTATUS(status),testPID);
				runningq.pop();//remove from runningq
				handleFinishedPid(statusPid);			
			} else {
				printf("unhandled status=%d,pid=%d,Considering finished\n",status,testPID);
			}
			//TODO:  handle if process doesn't terminate
		}   
		
		//copy checkq back to runningq
		//printf("copying checkq(%d) to runningq(%d)\n",(int)checkq.size(),(int)runningq.size());
		while(!checkq.empty()){
			pid_t tmp=checkq.front();
			runningq.push(tmp);
			//printf("copy pid %d\n", checkq.front());
			checkq.pop();
		}


		
	}//end of while((int)finishedq.size()!=numNodes)
		
	printf("finishq.size()=%d, numNodes=%d\n",(int)finishedq.size(),numNodes);
}//end of main

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
}

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
