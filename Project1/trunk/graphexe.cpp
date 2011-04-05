/* CPSC545 Spring2011 Project 1
* login: masuij(login used to submit)
* Linux
* date: 03/28/11
* name: Justin Masui, full_name2 (for partner(s))
* emails: masuij@seattleu.edu */

#include <iostream>
#include "graphexe.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

void print(Node a){
    cout<<"-----------------------------"<<endl;
    cout<<"id: "<< a.id <<endl; // corresponds to line number in graph text file
    cout<<"prog: "<< a.prog <<endl; // prog + arguments
    cout<<"input: "<< a.input <<endl; // filename
    cout<<"output: "<< a.output <<endl; // filename
    cout<<"children: ";
    for(int i=0;i<a.num_children;i++){
        cout<<a.children[i]<<" "; // children IDs
    }
    cout<<endl;
    cout<<"num_children: "<< a.num_children <<endl; // how many children this node has
    cout<<"status: "<< a.status <<endl; // ineligible/ready/running/finished
    //id_t pid; // track it when it's running
    cout<<"-----------------------------"<<endl;
}

Node createNode(string line, int id){
    Node aNode;
    vector<string> array;
    vector<int> intarray;

    trim(line);
	array=split(line,":");//splits into array by ":"
    if(array.size() == 4){
        //---------------
        //parse into node
        //---------------

        //id
        aNode.id=id;
        
        //prog
        strcpy(aNode.prog, array[0].c_str());

        //children
        if(array[1]=="none"){
            aNode.children.push_back(-99);
        }else{
            intarray=splitToInt(array[1].c_str()," ");
            for(int i=0;i<(int)intarray.size();i++){
                aNode.children.push_back(intarray[i]);
            }
        }

        //num_children
        aNode.num_children=intarray.size();

        //input/output
        strcpy(aNode.input, array[2].c_str());
        strcpy(aNode.output, array[3].c_str());

        //status
        aNode.status=INELIGIBLE;

        //print(aNode);

    }else{
        //Debugging purposes
        //cout<<"Line "<<id<<": incorrectly formatted, ignoring..."<<endl;
		aNode.id=-99;
    }
    return aNode;
}

vector<string> split(string str, string delimiter){
  vector<string> res;
  int end=0;
  int beg=0;
  while(end < (int)str.length()){
    end=(int)str.find(delimiter,beg);
    if(end==(int)string::npos){
      end=str.length();
    }
    //cout<<"beg:"<<beg<<" end: "<<end;
    //cout<<" substr:'"<<str.substr(beg, end-beg)<<"'"<<endl;
    res.push_back(str.substr(beg, end-beg));
    beg=beg+(end-beg)+delimiter.length();//beg= lengthof substring+lengthof delimiter
  }
  return res;                 
}

vector<int> splitToInt(string str, string delimiter){
  vector<int> res;
  int end=0;
  int beg=0;
  while(end < (int)str.length()){
    end=(int)str.find(delimiter,beg);
    if(end==(int)string::npos){
      end=str.length();
    }
    //cout<<"beg:"<<beg<<" end: "<<end;
    //cout<<" substr:'"<<str.substr(beg, end-beg)<<"'"<<endl;
    res.push_back( atoi((str.substr(beg, end-beg)).c_str()) );
    beg=beg+(end-beg)+delimiter.length();//beg= lengthof substring+lengthof delimiter
  }
  return res;                 
}

// helper function to trim extra whitespace from a string
void trim(string& str)
{
  string::size_type pos = str.find_last_not_of(' ');
  if(pos != string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of(' ');
    if(pos != string::npos) str.erase(0, pos);
  }
  else str.erase(str.begin(), str.end());
}
