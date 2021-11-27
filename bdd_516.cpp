#include<iostream>
#include<fstream>  
#include <vector>    
#include<cstring>

#include "index_tree.h"

#include "util.h"
#include "cudd.h"

using namespace std;

//函數功能: 傳入一個字串s，以splitSep裡的字元當做分割字符，回傳vector<string>
vector<string> splitStr2Vec(string s, string splitSep)
{
    vector<string> result;
    int current = 0; //最初由 0 的位置開始找
    int next = 0;
    while (next != -1)
    {
        next = s.find_first_of(splitSep, current); //尋找從current開始，出現splitSep的第一個位置(找不到則回傳-1)
        if (next != current)
        {
            string tmp = s.substr(current, next - current);
            if(!tmp.empty())  //忽略空字串(若不寫的話，尾巴都是分割符會錯)
            {
                result.push_back(tmp);
            }
        }
        current = next + 1; //下次由 next + 1 的位置開始找起。
    }
    return result;
}

//symbol table
//讀入字串 回傳synbol
int symbol_table(string name,int input_count,int output_count,int wire_count)
{

    //order:0~input_count-1為input
    //input_count~input_count+output_count-1為output
    //input_count+output_count~2input_count+output_count-1為~input
    //2input_count+output_count~2input_count+2output_count-1為~output
    //2input_count+2output_count~2input_count+2output_count+wire_count-1為wire
    //2input_count+2output_count+wire_count~2input_count+2output_count+2wire_count-1為~wire
    //大於為常數
    int out_symbol = 0;
    if((name[0]=='i')&&(name[1]=='n'))
    {
        vector<string> sep = splitStr2Vec(name, "[]");
        out_symbol =  stoi(sep[1]);
        //cout <<out_symbol  <<endl;
    }
    else if((name[0]=='o')&&(name[1]=='u')&&(name[2]=='t'))
    {
        vector<string> sep = splitStr2Vec(name, "[]");
        out_symbol =  stoi(sep[1])+input_count;
    }
    else if((name[0]=='~')&&(name[1]=='i')&&(name[2]=='n'))
    {
        vector<string> sep = splitStr2Vec(name, "[]");
        out_symbol =  stoi(sep[1])+input_count+output_count;
    }
    else if((name[0]=='~')&&(name[1]=='o')&&(name[2]=='u')&&(name[3]=='t'))
    {
        vector<string> sep = splitStr2Vec(name, "[]");
        out_symbol = stoi(sep[1])+2*input_count+output_count;
    }
    else if((name[0]=='o')&&(name[1]=='r')&&(name[2]=='i'))
    {
        vector<string> sep = splitStr2Vec(name, "p ");
        out_symbol = stoi(sep[1])+2*input_count+2*output_count;
    }
    else if((name[0]=='~')&&(name[1]=='o')&&(name[2]=='r')&&(name[3]=='i'))
    {
        vector<string> sep = splitStr2Vec(name, "p ");
        out_symbol = stoi(sep[1])+2*input_count+2*output_count+wire_count;
    }
    else if((name[0]=='1')&&(name[2]=='b'))  //常數
    {
         vector<string> sep = splitStr2Vec(name, "b");
         out_symbol = stoi(sep[1])+2*input_count+2*output_count+2*wire_count;
    }
    else if((name[1]=='1')&&(name[3]=='b'))
    {
         vector<string> sep = splitStr2Vec(name, "b");
         out_symbol = stoi(sep[1])+2*input_count+2*output_count+2*wire_count+2;
    }
    return out_symbol;
}

   

int main(int argc, char* argv[])
{
    ifstream infile;
    ofstream outfile;
    FILE *output;
    output = fopen("bdd_out.txt", "w");
    
    string line,token;
    char line_c[80];
    
    const char *dem = " ,[";
    const char *dem_inout = ":";
    const char *dem_var = ",";
    const char *seperate_eq = "=;";
    const char *seperate_eq1 = "=; ";
     const char *seperate_eq_op = " ";
    const char *seperate_eq2 = ";";
    const char *rhs_seperate = "&^|;";  //rhs seperate = and or xor
    const char *deter_not = "~"; //如果出現~就把它invert
    char *first;
     
    
    /*deal with string*/
    //再對RHS進行分割,RHS會分成兩個bit邏輯的項
    vector<string> LHS,RHS;
    vector< vector<string> > sub_rhs;
    vector<string> RHS_flatten;
    
    vector<string> wire_name;
    int wire_count = 0;   //wire可以有很多個
    int assign_count = 0;
    int sels_count = 0; //總共有幾個in,out
    
    int sub_RHS_num = 1;
    vector<int> sub_RHS_num_vec;  //calculate how many sub rhs
    vector<string> operator_rhs;
    
    int module_num = 0,input_num(0),output_num(0),output_reg_num(0);
    
    /*graph相關參數*/
    bool tree_exist = false;
    vector<int> show_index;
    
    /*index tree相關參數*/
    bool bulid_table = false;
    int index_count = 0;
    
    /*bdd initialize*/
    DdManager *global_bdd_manager;
    global_bdd_manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0); //inintialize a new BDD manager
  
    DdNode *bdd,*x1;
    DdNode *var,*var2,*temp,*temp_neg;
    bdd  = Cudd_ReadOne(global_bdd_manager);
	  Cudd_Ref(bdd);
   
    //vector
    vector<tree_node**> table;
   
    /*file I/O block*/
    infile.open(argv[1]); 
    if(!infile)  cout << "error while loading" << endl;
 
    //build tree
    Avl_tree* root;
    root = create();
    
    //problem:if empty line exists,segmentation fault 
    while(!infile.eof())
    {
        //infile >> line;
        infile.getline(line_c,sizeof(line_c));
        //cout << line_c << endl;
        line.assign(line_c);  //char to string
        //cout << line << endl; 
        //cout << line.find(" ") << endl;      
       
        first = strtok(line_c,dem);
        //cout << first <<endl;
        
        //determine type
        //module,endmodule
        if( !strcmp(first,"module") )
        {
            //cout << "there is a new module"<<endl;
            ++module_num; 
            //if(module_num==2) break;
        }
        else if(!strcmp(first,"endmodule"))
        {
            //cout << "end of a module"<<endl<<endl;
            break;
        }
        else if(!strcmp(first,"  input")||!strcmp(first,"\tinput"))
        {
            first = strtok(NULL,dem_inout);
            input_num = atoi( first)+1;
        }
        else if(!strcmp(first,"  output")||!strcmp(first,"\toutput"))
        {
            first = strtok(NULL,dem_inout);
            output_num = atoi(first)+1;
        }
        //assign,主要判斷的部分
        else if( !strcmp(first,"assign") ||!strcmp(first,"\tassign"))
        {
            index_count = (input_num + output_num + wire_count)*2;   
            if(!bulid_table)
            {
                table.resize(index_count+4);
            }   
            bulid_table = true;
            tree_exist = false;
        
            //seperate into LHS,RHS,use bdd package to build tree and analyze
             assign_count++;
             first = strtok(NULL,seperate_eq1 );
             //first = strtok(NULL,seperate_eq1 );
             //cout << first <<endl;
             token.assign(first);      //char to string
             //cout <<token<<endl; 
             first = strtok(NULL,seperate_eq_op );
             for(int i=0; i<LHS.size(); i++)
             {    //確認token沒出現過再新增graph node
                 if(token == LHS[i]){
                    tree_exist = true;
                    //cout << token;
                 }
             }
             
             for(int i=0;i<sub_rhs.size();++i)
             {
                 for(int j=0;j<sub_rhs[i].size();++j){
                    if(token == sub_rhs[i][j]){
                         tree_exist = true;
                         //cout << token;
                    }
                 }
             }
             
             if(tree_exist == false)
             {    //token沒出現過,新增tree node
                 tree_insert(root,symbol_table(token,input_num,output_num,wire_count));         
                 cout << symbol_table(token,input_num,output_num,wire_count)<<endl;
                 show_index.push_back(symbol_table(token,input_num,output_num,wire_count));
                 //cout << token<<endl;
                 table[symbol_table(token,input_num,output_num,wire_count)] = new tree_node*;
                 *table[symbol_table(token,input_num,output_num,wire_count)] = find_index(root->root,symbol_table(token,input_num,output_num,wire_count));
             }
             
             tree_exist = false;
             LHS.push_back(token);
             
             first = strtok(NULL,seperate_eq2 );
             //cout << first <<endl;
             
             token.assign(first);     //char to string
             RHS.push_back(token);
             
             for(int i = 0;i < RHS.back().length();++i){
                //確認子函式數目
                //cout<<RHS[i][j];
                if( RHS.back()[i] == '|')
                {
                    operator_rhs.push_back("|");
                }
                else if( RHS.back()[i] == '&')
                { 
                    operator_rhs.push_back("&");
                }
                else if( RHS.back()[i] == '^') 
                {
                    operator_rhs.push_back("^");
                }      
             }
             //沒有符號
             if(operator_rhs.size()!= RHS.size())
             {
                 operator_rhs.push_back("@"); //沒有運算的意思
             }
             
             //cout << token <<endl;
             vector<string> sep = splitStr2Vec(token, " =&^|;");
             //for(int i=0;i<sep.size();++i)  cout<<sep[i]<<endl;
             
             for(int i=0; i<LHS.size(); i++)
             {    //確認token沒出現過再新增graph node
                 if(sep[0] == LHS[i])
                 {
                    tree_exist = true;
                 }
             }
             
             for(int i=0;i<sub_rhs.size();++i)
             {
                 for(int j=0;j<sub_rhs[i].size();++j)
                 {
                    if(sep[0] == sub_rhs[i][j])
                    {
                        tree_exist = true;
                    }
                 }
             }
             
             if(tree_exist== false)
             {    //token沒出現過,新增graph node
                 tree_insert(root,symbol_table(sep[0],input_num,output_num,wire_count));        
                 cout << symbol_table(sep[0],input_num,output_num,wire_count)<<endl;
                 show_index.push_back(symbol_table(sep[0],input_num,output_num,wire_count));
                 table[symbol_table(sep[0],input_num,output_num,wire_count)] = new tree_node*;
                 *table[symbol_table(sep[0],input_num,output_num,wire_count)] = find_index(root->root,symbol_table(sep[0],input_num,output_num,wire_count));
             }
             tree_exist = false;
             
             if(sep.size()==2)
             {
                 for(int i=0; i<LHS.size(); i++)
                 {    //確認token沒出現過再新增graph node
                     if(sep[1] == LHS[i])
                     {
                        tree_exist = true;
                        //cout << sep[1] <<endl;
                     }
                 }
             
                 for(int i=0;i<sub_rhs.size();++i)
                 {
                     for(int j=0;j<sub_rhs[i].size();++j)
                     {
                        if(sep[1] == sub_rhs[i][j])
                        {
                             tree_exist = true;
                        }
                     }
                 }
                 if(sep[1] == sep[0]) tree_exist = true;
                 if(tree_exist == false)
                 {    //token沒出現過,新增graph node
                     tree_insert(root,symbol_table(sep[1],input_num,output_num,wire_count));           
                     cout << symbol_table(sep[1],input_num,output_num,wire_count)<<endl;                     
                     show_index.push_back(symbol_table(sep[1],input_num,output_num,wire_count));
                     //cout << sep[1] <<endl;
                     table[symbol_table(sep[1],input_num,output_num,wire_count)] = new tree_node*;
                     *table[symbol_table(sep[1],input_num,output_num,wire_count)] = find_index(root->root,symbol_table(sep[1],input_num,output_num,wire_count));
                 }    
             }
             
             tree_exist == false;
             sub_rhs.push_back(sep);
             
             
             //cout <<endl<< sub_rhs.back().size() <<endl<<endl;
             
             if(sub_rhs.back().size()==2)
             {
                 insert_tworhs(*table[symbol_table(LHS.back(),input_num,output_num,wire_count)],*table[symbol_table(sub_rhs.back()[0],input_num,output_num,wire_count)],*table[symbol_table(sub_rhs.back()[1],input_num,output_num,wire_count)],operator_rhs.back());
             } 
             else if(sub_rhs.back().size()==1)
             {
                 insert_rhs(*table[symbol_table(LHS.back(),input_num,output_num,wire_count)],*table[symbol_table(sub_rhs.back()[0],input_num,output_num,wire_count)]);
             }         
        }   
        
        else if( !strcmp(first,"wire") ||!strcmp(first,"\twire"))
        {
            first = strtok(NULL,seperate_eq );
            wire_count++;
            token.assign(first); 
            //cout << first <<endl;
            wire_name.push_back(token);
        }
        
    }
    
    
    //to check whether LHS,RHS exist
    if(!LHS.empty())
    {
        //print out LHS,RHS
        //cout<< endl << "LHS:" << endl;
        for(int i = 0; i <  LHS.size(); i++) {
            //cout <<  LHS[i] << endl;
        }
    }
    //cout << "number of rhs: "<< sub_RHS_num <<endl;
    
    for(int i = 0; i <  sub_RHS_num_vec.size(); i++) 
    {
       // cout << "number of rhs "<<i<<" subfunction = "<<  sub_RHS_num_vec[i] << endl;
    }
    
    if(!wire_name.empty())
    {
        //print out LHS,RHS
        //cout<< endl << "wire:" << endl;
        for(int i = 0; i <  wire_name.size(); i++) {
            //cout <<  wire_name[i] << endl;
        }
    }
    
    
    
    //print sub_rhs
    //cout <<"sub_rhs:"<<endl;
    for(int i=0;i < sub_rhs.size(); ++i)
    {
        //cout<<"item "<<i<<":"<<endl;
        for(int j=0; j < sub_rhs[i].size();++j)
        {
            //cout <<  sub_rhs[i][j] <<endl;
            RHS_flatten.push_back(sub_rhs[i][j]);
        }
        //cout << endl;
    }
    
    
    //判斷有幾個 input+output(value)數目
    //現有的:LHS(一維vector),RHS_flatten(一維)
    vector<string> merge_lhs_plus_rhs;
    merge_lhs_plus_rhs.insert(merge_lhs_plus_rhs.begin(),LHS.begin(),LHS.end());
    merge_lhs_plus_rhs.insert(merge_lhs_plus_rhs.end(),RHS_flatten.begin(),RHS_flatten.end());
    
    //cout<< "merge rhs and lhs:" <<endl;
    for(int i = 0; i <   merge_lhs_plus_rhs.size(); i++) 
    {
        // cout <<   merge_lhs_plus_rhs[i] << endl;
    }
    
    
    
    //cout<<endl<<"operator:"<<endl;
    for(int i=0;i<operator_rhs.size();++i)
    {
        //cout<<operator_rhs[i]<<endl;
    }
    
    
      
    //test index tree
    /*  
    cout<<symbol_table("in[12]",input_num,output_num,wire_count)<<endl;
    cout<<symbol_table("out[1]",input_num,output_num,wire_count)<<endl;
    cout<<symbol_table("origtmp1",input_num,output_num,wire_count)<<endl;
    */
    
    cout<<endl<<"count of module: " << module_num << endl;
    cout<<"count of input_num: " << input_num <<endl;
    cout<<"count of output_num: " << output_num <<endl;
    cout<<"count of wire count: " << wire_count << endl;
    cout << "count of assign count: " << assign_count << endl;
    cout << "count of tree  node: " << tree_count(root) << endl;
    cout << "table size: " << table.size() << endl;
    
    inorder(root->root);
    cout<<endl;
    
    
    //cout<<(*table[0])->right->index<<endl;
    //cout<<(*table[41])->right->left->index<<endl;
    for(int i=0;i< show_index.size();++i)
    {
        cout<<endl<<show_index[i]<<" linked in:" <<endl;
        for(int j=0;j<(*table[show_index[i]])->link_in.size();++j)
        {
            cout<<(*table[show_index[i]])->link_in[j]->index<<endl;
        }
        if((*table[show_index[i]])->link_in.size()==2) cout<<"op:"<<(*table[show_index[i]])->op<<endl;
        cout<<endl<<show_index[i]<<" linked out:" <<endl;
        for(int j=0;j<(*table[show_index[i]])->link_out.size();++j)
        {
            cout<<(*table[show_index[i]])->link_out[j]->index<<endl;
        }
    }
    
    
    DdNode *variable[index_count+4];

    //宣告index_count個變數
    for(int i =0 ;i<index_count+4;++i)
    {
        variable[i] = Cudd_bddIthVar(global_bdd_manager, i); //create new bdd variable 
    }
    
    cout<<endl;
    for(int i=0;i< show_index.size();++i)
    {
        cout << show_index[i] <<endl;
    }
    cout<<endl;
    
    //variable[18] = Cudd_bddAnd(global_bdd_manager,variable[0],variable[41]);
    //Cudd_Ref(variable[18]);
    
    //variable[41] =variable[44];
    //Cudd_Ref(variable[41]);
    
    
    for(int i=0;i< show_index.size();++i)
    {
        if((*table[show_index[i]])->link_in.size()>0)
        {
            if((*table[show_index[i]])->link_in.size()==1)
            {
                cout << show_index[i]<<endl;
                cout <<(*table[show_index[i]])->link_in[0]->index <<endl;
                
                temp = variable[(*table[show_index[i]])->link_in[0]->index];
                //temp = variable[((*table[show_index[i]])->link_in[0]->index)];
            }
            else if((*table[show_index[i]])->link_in.size()==2)
            {
                cout << show_index[i]<<endl;
                cout << (*table[show_index[i]])->link_in[0]->index <<endl;
                cout << (*table[show_index[i]])->link_in[1]->index <<endl;
            
                var = variable[((*table[show_index[i]])->link_in[0]->index)];
                var2 =  variable[((*table[show_index[i]])->link_in[1]->index)]; 
            
                if((*table[show_index[i]])->op=="&")
                {
                     cout <<"&"<<endl;
                    temp =  Cudd_bddAnd(global_bdd_manager,var,var2);
                }
                else if((*table[show_index[i]])->op=="|")
                {
                    cout <<"|"<<endl;
                    temp =  Cudd_bddOr(global_bdd_manager,var,var2);
                    
                }
                else if((*table[show_index[i]])->op=="^")
                {
                    temp = Cudd_bddXor(global_bdd_manager,var,var2);
                }   
                Cudd_Ref(temp);
                Cudd_RecursiveDeref(global_bdd_manager,variable[show_index[i]]);
            }
            variable[show_index[i]] = temp;
            
        }
    }
    
    
    //plot
    variable[20] = Cudd_BddToAdd(global_bdd_manager, variable[20]);
    DdNode **node_array;
	  node_array = (DdNode**)malloc(sizeof(DdNode*));
    node_array[0] =  variable[20];
	  Cudd_DumpDot(global_bdd_manager, 1, node_array, NULL, NULL, output);
	  free(node_array);
   
    infile.close();
    outfile.close();
    fclose(output);
    Cudd_Quit(global_bdd_manager);
    
    return 0;
}
