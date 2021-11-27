#include<iostream>
#include<fstream>
#include<cstring>

#include "util.h"
#include "cudd.h"

using namespace std;

int main(int argc, char* argv[])
{
  ifstream infile;
  //ofstream outfile;
  FILE *output;
	output = fopen(argv[2], "w");
  
  
  string func1,num;
  
  int num_of_sub_func1(1),num_of_sub_func2(1),length(0);
  int index1(0),index1_inter(0);
  int index_subf1(0);
  int lines(0);
  int *order;
  
 	char temp_variable = '0';
  
  //terminal 1 and 2 element ./hw1_1 input.txt output.txt 
  infile.open(argv[1]);  //input.txt
  //outfile.open(argv[2]); //output.txt  
  
  DdManager *global_bdd_manager;
  global_bdd_manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0); 
  //inintialize a new BDD manager
  
  DdNode *bdd,*bdd2;
  DdNode **variable,*var,*temp,*temp_neg,**subf1,**subf2;
  
  bdd  = Cudd_ReadOne(global_bdd_manager);
	Cudd_Ref(bdd);
  
  //read file:the main function
  infile >> func1;
  
  //cout<<func1<<endl;
  //cout<<func2<<endl;  
 
  //to determine the subfunction's number in SOP(product's number)
  for(int i = 0;i < func1.length();++i)
  {
      if( func1[i] == '+')  ++num_of_sub_func1;
  }
  
  // add new element,new location
  subf1 = new DdNode*[num_of_sub_func1];

  while(!infile.eof()){
		infile >> num;
    //cout<<num<<endl;
		++lines;
    //cout<<lines<<endl;
	}
 
  length = num.length()-1;           //charactor's length
  infile.seekg(0, infile.beg);       //back to begining
	infile >> func1;                   //move to input line 2
  //cout<<func1<<endl;

   //create variable,the most 26(a or A ~ z or Z)
   variable = new DdNode*[length];
  for(int i =0 ;i<length;++i)
      variable[i] = Cudd_bddIthVar(global_bdd_manager, i); //create new bdd variable



  //deal with string
  for(index1 = 0;index1 < (func1.length()-1);++index1){
      for(index1_inter = 0; (func1[index1+index1_inter]!='+') && (func1[index1+index1_inter]!='.') ; ++index1_inter)
      {   //create subfunction
          if(func1[index1+index1_inter] >= 'a')
          {   // lowercase character
              var = variable[func1[index1+index1_inter] - 'a'];
              temp = Cudd_bddAnd(global_bdd_manager,var,bdd);
          }
          else if(func1[index1+index1_inter] < 'a' && func1[index1+index1_inter] >= 'A')
          {   // uppercase character,consider inverting
              var = variable[func1[index1+index1_inter] - 'A'];
              temp_neg = Cudd_Not(var);
              temp = Cudd_bddAnd(global_bdd_manager,temp_neg ,bdd);
          }
          else
          {
              cout<<"input something wrong" << endl;
          }
          
         	Cudd_Ref(temp);
			    Cudd_RecursiveDeref(global_bdd_manager, bdd); //It is used to dispose of a DD that is no longer needed.
			    bdd = temp;
          //cout<<func1[index1 + index1_inter];
      }
      index1 += index1_inter;
      subf1[index_subf1] = bdd;
      ++index_subf1;
      
      bdd  = Cudd_ReadOne(global_bdd_manager); //reset bdd to next conjunction
  }
   
  //adding subfunction
  bdd = subf1[0];
  for(int i = 1; i < num_of_sub_func1; ++i)
  {
      var = subf1[i];
		  temp = Cudd_bddOr(global_bdd_manager, var, bdd);
   	  Cudd_Ref(temp);
      Cudd_RecursiveDeref(global_bdd_manager, bdd); //It is used to dispose of a DD that is no longer needed.
      bdd = temp;
  }
  
  //reorder
  order = new int[length];
  
  //read length lines
  for(int i = 0; i < lines; ++i)
  {
		for(int j = 0; j < length; ++j){
			infile>>temp_variable; //1 charactor 1 charactor read
			order[j] = (temp_variable - 'a');
		}
		infile>>temp_variable;//temp = '.';
		Cudd_ShuffleHeap(global_bdd_manager, order); //Reorders variables according to given permutation.
		//outfile<<(Cudd_DagSize(bdd)) + 1<<endl; // Returns the number of nodes in the graph rooted at node. so+1
	}
 
   //-------plot--------//
	bdd = Cudd_BddToAdd(global_bdd_manager, bdd);
	DdNode **node_array;
	node_array = (DdNode**)malloc(sizeof(DdNode*));
  node_array[0] = bdd;
	Cudd_DumpDot(global_bdd_manager, 1, node_array, NULL, NULL, output);
	free(node_array);
  //-------------------//
 
  
  delete [] subf1;
  
  infile.close();
	fclose(output);
  Cudd_Quit(global_bdd_manager);
  return 0;
}

