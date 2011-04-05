/* CPSC545 Spring2011 Project 1
* login: masuij(login used to submit)
* Linux
* date: 03/28/11
* name: Justin Masui, full_name2 (for partner(s))
* emails: masuij@seattleu.edu */

/*
Basic idea is that each node's information is added ot the following maps

name            key     value
--------------- ---     --------------------
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
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "graphexe.h"

#define MAX_ARGUMENTS 10

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

/*--------------------------
queue
----------------------------*/
priority_queue<int> readyq;
priority_queue<int> ineligibleq;
queue<int> finishedq;
queue<int> runningq;

/*----------------------------
populates the numParents
----------------------------*/
void addNodeToNumParents(Node node);

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
	int numNodes;

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
	}
	numNodes=lineNum+1;

    map<int, int>::const_iterator iter;
    cout<<"-----------------------"<<endl;
    cout<<"node numberParents"<<endl;
    for(iter=numParents.begin(); iter!= numParents.end(); ++iter){
        cout<<iter->first<<"    "<<(iter->second)<<endl;
    }

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
	//key=os pid, value=line number
	map<pid_t,int> pidToNodeIDMap;
	while((int)finishedq.size()!=numNodes){
		while(!readyq.empty()) {
			int r = readyq.top();
			readyq.pop();
			Node runningNode = allNodesMap[r];

			pid_t p = fork();

			// Child
			if (p == 0) {
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
			// Parent
			} else if (p > 0) {
				// update pid to node map
				pidToNodeIDMap[p] = r;
			} else {
				perror("Fork failed.");
				// what to do in this case?
			}
		}

		//wait for pid to finish
		pid_t finishPid=wait(NULL);
		int finishedNodeID=pidToNodeIDMap[finishPid];

		//update numParents for each of the finishedNodeID's children
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
					//cout<<"adding "<<childToUpdate<<" to readyq"<<endl;
				}
			}
		}

		//update finishq
		finishedq.push(finishedNodeID);
		//cout<<"adding "<<finishedNodeID<<" to finishedq"<<endl;

		//update allNodesMap
		Node updateNode=allNodesMap[finishedNodeID];
		updateNode.status=FINISHED;
		allNodesMap[finishedNodeID]=updateNode;

	}//end of while((int)finishedq.size()!=numNodes)

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
