/*
 * graphexe.h
 * 
 * Contains helper functions and data structures
 *
 */

#ifndef GRAPHEXE_H_
#define GRAPHEXE_H_

#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <string>

using std::cout;
using std::endl;
using std::string;
using std::vector;

enum Status { INELIGIBLE, READY, RUNNING, FINISHED };

struct Node {
    int id; // corresponds to line number in graph text file
    char prog[1024]; // prog + arguments
    char input[1024]; // filename
    char output[1024]; // filename
    vector<int> children; // children IDs
    int num_children; // how many children this node has
    Status status; // ineligible/ready/running/finished
    pid_t pid; // track it when it's running
};

void print(Node a);
Node createNode(string line, int id);

//parsing functions
vector<string> split(string str, string delimiter);
vector<int> splitToInt(string str, string delimiter);
void trim(string& str);

#endif
