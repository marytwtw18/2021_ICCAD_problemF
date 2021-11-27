#include<iostream>
#include<fstream>
#include<cstring>

#include "util.h"
#include "cudd.h"

using namespace std;

int main(int argc, char* argv[])
{
  ifstream infile;
  ofstream outfile;
  
  string func1,func2;
  
  int num_of_sub_func1(1),num_of_sub_func2(1);
  int index1(0),index2(0),index1_inter(0),index2_inter(0);
  int index_subf1(0),index_subf2(0);
  
  //terminal 1 and 2 element ./hw1_1 input.txt output.txt 
  infile.open(argv[1]);  //input.txt
  outfile.open(argv[2]); //output.txt
  
  DdManager *global_bdd_manager;
  global_bdd_manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0); //inintialize a new BDD manager
  
  DdNode *bdd,*bdd2,*x1,*x2;
  DdNode *variable[26],*var,*temp,*temp_neg,**subf1,**subf2;
  
  bdd  = Cudd_ReadOne(global_bdd_manager);
	Cudd_Ref(bdd);
 
  //create variable,the most 26(a or A ~ z or Z)
  for(int i =0 ;i<26;++i)
      variable[i] = Cudd_bddIthVar(global_bdd_manager, i); //create new bdd variable
  
  //read file:one line one function
  infile >> func1;
  infile >> func2;
  
  //cout<<func1<<endl;
  //cout<<func2<<endl;  
 
  //to determine the subfunction's number in SOP(product's number)
  for(int i = 0;i < func1.length();++i){
      if( func1[i] == '+')  ++num_of_sub_func1;
  }
  for(int i = 0;i < func2.length();++i){
      if( func2[i] == '+')  ++num_of_sub_func2;
  }
  
  // add new element,new location
  subf1 = new DdNode*[num_of_sub_func1];
  subf2 = new DdNode*[num_of_sub_func2];
  
  //deal with string
  for(index1 = 0;index1 < (func1.length());++index1){
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
  
  //deal with function2
  for(index2 = 0;index2 < (func2.length());++index2){
      for(index2_inter = 0; (func2[index2+index2_inter]!='+') && (func2[index2+index2_inter]!='.') ; ++index2_inter)
      {   //create subfunction
          if(func2[index2+index2_inter] >= 'a')
          {   // lowercase character
              var = variable[func2[index2+index2_inter] - 'a']; 
              temp = Cudd_bddAnd(global_bdd_manager,var,bdd); //and
          }
          else if(func2[index2+index2_inter] < 'a' && func2[index2+index2_inter] >= 'A')
          {   // uppercase character,consider inverting
              var = variable[func2[index2+index2_inter] - 'A'];
              temp_neg = Cudd_Not(var); //inversion
              temp = Cudd_bddAnd(global_bdd_manager,temp_neg ,bdd); //and
          }
          else
          {
              cout<<"input something wrong" << endl; //not english character
          }
          
         	Cudd_Ref(temp);
			    Cudd_RecursiveDeref(global_bdd_manager, bdd); //It is used to dispose of a DD that is no longer needed.
			    bdd = temp;
          //cout<<func2[index2 + index2_inter];
      }
      index2 += index2_inter;
      subf2[index_subf2] = bdd;
      ++index_subf2;
      
      bdd  = Cudd_ReadOne(global_bdd_manager); //reset bdd to next conjunction
      //Returns the one constant of the manager. The one constant is common to ADDs and BDDs.
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
  
  bdd2 = subf2[0];
  for(int i = 1; i < num_of_sub_func2; ++i)
  {
      var = subf2[i];
		  temp = Cudd_bddOr(global_bdd_manager, var, bdd2);
   	  Cudd_Ref(temp);
      Cudd_RecursiveDeref(global_bdd_manager, bdd2);//It is used to dispose of a DD that is no longer needed.
      bdd2 = temp;
  }
  
  if(bdd == bdd2)
  {
      outfile << "1" <<endl;
  }
  else
  {
      outfile << "0" <<endl;
  }
  
  delete [] subf1;
  delete [] subf2;
  
  infile.close();
	outfile.close();
  Cudd_Quit(global_bdd_manager);
  return 0;
}