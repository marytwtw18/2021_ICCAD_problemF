#include<iostream>
#include<fstream>  
#include <vector>    
#include<cstring>

#include "index_tree.h"
#include "Queue.h"

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
    
    //vector
    vector<tree_node**> table;
   
    /*file I/O block*/
    infile.open(argv[1]); 
    if(!infile)  cout << "error while loading file" << endl;
    
    outfile.open(argv[2]);
    if(!outfile) cout << "error while outputing file" << endl;
 
    //build tree
    Avl_tree* root;
    root = create(); 
    
    //grouping
    string boolean[4] ={"1'b0","~1'b0","1'b1","~1'b1"};
    int bool_val[4] = {0,1,1,0};
    bool rhs_show_in_lhs;
    
    //queue initialize
    List* list1;
    list1 = createList();
    
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
                 tree_insert(root,symbol_table(token,input_num,output_num,wire_count),token);         
                 //cout << symbol_table(token,input_num,output_num,wire_count)<<endl;
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
                 tree_insert(root,symbol_table(sep[0],input_num,output_num,wire_count),sep[0]);        
                 //cout << symbol_table(sep[0],input_num,output_num,wire_count)<<endl;
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
                     tree_insert(root,symbol_table(sep[1],input_num,output_num,wire_count),sep[1]);           
                     //cout << symbol_table(sep[1],input_num,output_num,wire_count)<<endl;                     
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
    
    //cout<<endl<<"count of module: " << module_num << endl;
    //cout<<"count of input_num: " << input_num <<endl;
    //cout<<"count of output_num: " << output_num <<endl;
    //cout<<"count of wire count: " << wire_count << endl;
    //cout << "count of assign count: " << assign_count << endl;
    //cout << "count of tree  node: " << tree_count(root) << endl;
    //cout << "table size: " << table.size() << endl;
    
    //inorder(root->root);
    //cout<<endl;
      
    //cout<<(*table[0])->right->index<<endl;
    //cout<<(*table[41])->right->left->index<<endl;
    for(int i=0;i< show_index.size();++i)
    {
        //cout<<endl<<(*table[show_index[i]])->value<<" linked in:" <<endl;
        for(int j=0;j<(*table[show_index[i]])->link_in.size();++j)
        {
            //cout<<(*table[show_index[i]])->link_in[j]->value<<endl;
        }
        
        //if((*table[show_index[i]])->link_in.size()==2) cout<<"op:"<<(*table[show_index[i]])->op<<endl;
        //cout<<endl<<(*table[show_index[i]])->value<<" linked out:" <<endl;
        for(int j=0;j<(*table[show_index[i]])->link_out.size();++j)
        {
           // cout<<(*table[show_index[i]])->link_out[j]->value<<endl;
        }
    }
    
     rhs_show_in_lhs = false;
     bool change_val;
     change_val = false;
     //after reading file,grouping
     //分類標記統一存在tree的LHS部分
     for(int i = 0; i <  LHS.size(); i++) {
         if((symbol_table(LHS[i],input_num,output_num,wire_count)<input_num)||(symbol_table(LHS[i],input_num,output_num,wire_count)>=input_num+output_num)) //out
         {
             if(sub_rhs[i].size()==2)  //item 2個
             {
                 for(int j=0;j<4;++j){
                     if(((sub_rhs[i][0] == boolean[j]) || (sub_rhs[i][1] == boolean[j])) && (!change_val))  //判別有無boolean
                     {
                         for(int k=0;k<4;++k)
                         {
                             if((sub_rhs[i][0] == boolean[j]) && (sub_rhs[i][1] == boolean[k]) && (!change_val))
                             {
                                 (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "BW";
                                 
                                 if( (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->op == "&")
                                 {
                                     (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal = bool_val[j] &  bool_val[k];
                                 }
                                 else if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->op == "|")
                                 {
                                     (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal = bool_val[j] |  bool_val[k];
                                 }
                                 else if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->op == "^")
                                 {
                                     (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal = bool_val[j] ^  bool_val[k];
                                 }
                                 change_val = true;
                             }
                             else if((sub_rhs[i][0] == boolean[k]) && (sub_rhs[i][1] == boolean[j]) && (!change_val))
                             {
                                 (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "BW";
                                 
                                 if( (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->op == "&")
                                 {
                                     (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal = bool_val[j] &  bool_val[k];
                                 }
                                 else if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->op == "|")
                                 {
                                     (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal = bool_val[j] |  bool_val[k];
                                 }
                                 else if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->op == "^")
                                 {
                                     (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal = bool_val[j] ^  bool_val[k];
                                 }
                                 change_val = true;
                             }
                         }   
                         if(!change_val)
                         {
                             for(int k=0;k < LHS.size();++k)
                             {
                                  if(sub_rhs[i][0] == LHS[k])
                                  {
                                      rhs_show_in_lhs = true;
                                      (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "DW";
                                      (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->gro_index1 = k;
                                      enqueue(list1,*table[symbol_table(LHS[i],input_num,output_num,wire_count)]);
                                      change_val = true;
                                  } 
                                  else if(sub_rhs[i][1] == LHS[k])
                                  {
                                      rhs_show_in_lhs = true;
                                      (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "DW";
                                      (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->gro_index1 = k;
                                      enqueue(list1,*table[symbol_table(LHS[i],input_num,output_num,wire_count)]);
                                      change_val = true;
                                  }
                             }
                             if(!rhs_show_in_lhs){
                                 (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "IW";
                                 change_val = true;
                             }
                         }
                     }
                 }
                 //沒有boolean
                 if(!change_val)
                 {
                     for(int k=0;k < LHS.size();++k)
                     {
                         if(sub_rhs[i][0] == LHS[k])
                         {
                             rhs_show_in_lhs = true;
                             (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "DW";
                             (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->gro_index1 = k;
                             enqueue(list1,*table[symbol_table(LHS[i],input_num,output_num,wire_count)]);
                             change_val = true;
                         } 
                         else if(sub_rhs[i][1] == LHS[k])
                         {
                             rhs_show_in_lhs = true;
                             (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "DW";
                             (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->gro_index2 = k;
                             enqueue(list1,*table[symbol_table(LHS[i],input_num,output_num,wire_count)]);
                             change_val = true;
                         }
                     }
                     if(!rhs_show_in_lhs){
                         (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "IW";
                         change_val = true;
                     }
                 }
                    
                 change_val =false;
                 rhs_show_in_lhs = false;            
             }
             else //item 1個
             {
                 //判別有無boolean
                 for(int j=0;j<4;++j){
                     if(sub_rhs[i][0] == boolean[j])
                     {
                         (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "BW";
                          change_val = true;
                         (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal = bool_val[j];
                     }
                 }
                 //無boolean
                 if(!change_val)
                 {
                     // cout << "no boolean" <<endl;
                     //判別有無項重複  
                     for(int j=0;j<LHS.size();++j)
                     {
                         if(sub_rhs[i][0] == LHS[j])
                         {
                              rhs_show_in_lhs = true;
                              change_val = true;
                              (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "DW";
                              (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->gro_index1 = j;
                              enqueue(list1,*table[symbol_table(LHS[i],input_num,output_num,wire_count)]);
                         } 
                     }
                     if(!rhs_show_in_lhs) {
                         (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "IW";
                         change_val = true;
                     }                         
                 }
                 rhs_show_in_lhs=false;
                 change_val = false;
             }
         }
         else  //wire
         {
             if(sub_rhs[i].size()==2)  //item 2個
             {
                 for(int j=0;j<4;++j){
                     if(((sub_rhs[i][0] == boolean[j]) || (sub_rhs[i][1] == boolean[j])) && (!change_val))  //判別有無boolean
                     {
                         for(int k=0;k<4;++k)
                         {
                             if((sub_rhs[i][0] == boolean[j]) && (sub_rhs[i][1] == boolean[k]) && (!change_val))
                             {
                                 (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "Out";
                                 
                                 if( (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->op == "&")
                                 {
                                     (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal = bool_val[j] &  bool_val[k];
                                 }
                                 else if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->op == "|")
                                 {
                                     (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal = bool_val[j] |  bool_val[k];
                                 }
                                 else if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->op == "^")
                                 {
                                     (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal = bool_val[j] ^  bool_val[k];
                                 }
                                 change_val = true;
                             }
                             else if((sub_rhs[i][0] == boolean[k]) && (sub_rhs[i][1] == boolean[j]) && (!change_val))
                             {
                                 (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "Out";
                                 
                                 if( (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->op == "&")
                                 {
                                     (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal = bool_val[j] &  bool_val[k];
                                 }
                                 else if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->op == "|")
                                 {
                                     (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal = bool_val[j] |  bool_val[k];
                                 }
                                 else if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->op == "^")
                                 {
                                     (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal = bool_val[j] ^  bool_val[k];
                                 }
                                 change_val = true;
                             }
                         }   
                         if(!change_val)
                         {
                             for(int k=0;k < LHS.size();++k)
                             {
                                  if(sub_rhs[i][0] == LHS[k])
                                  {
                                      rhs_show_in_lhs = true;
                                      (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "D";
                                      (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->gro_index1 = k;
                                      enqueue(list1,*table[symbol_table(LHS[i],input_num,output_num,wire_count)]);
                                      change_val = true;
                                  } 
                                  else if(sub_rhs[i][1] == LHS[k])
                                  {
                                      rhs_show_in_lhs = true;
                                      (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "D";
                                      (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->gro_index1 = k;
                                      enqueue(list1,*table[symbol_table(LHS[i],input_num,output_num,wire_count)]);
                                      change_val = true;
                                  }
                             }
                             if(!rhs_show_in_lhs){
                                 (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "ID";
                                 change_val = true;
                             }
                         }
                     }
                 }
                 //沒有boolean
                 if(!change_val)
                 {
                     for(int k=0;k < LHS.size();++k)
                     {
                         if(sub_rhs[i][0] == LHS[k])
                         {
                             rhs_show_in_lhs = true;
                             (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "D";
                             (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->gro_index1 = k;
                             enqueue(list1,*table[symbol_table(LHS[i],input_num,output_num,wire_count)]);
                             change_val = true;
                         } 
                         else if(sub_rhs[i][1] == LHS[k])
                         {
                             rhs_show_in_lhs = true;
                             (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "D";
                             (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->gro_index2 = k;
                             enqueue(list1,*table[symbol_table(LHS[i],input_num,output_num,wire_count)]);
                             change_val = true;
                         }
                     }
                     if(!rhs_show_in_lhs){
                         (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "ID";
                         change_val = true;
                     }
                 }
                    
                 change_val =false;
                 rhs_show_in_lhs = false;            
             }
             else //item 1個
             {
                 //判別有無boolean
                 for(int j=0;j<4;++j){
                     if(sub_rhs[i][0] == boolean[j])
                     {
                         change_val =true;
                         (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "Out";
                         (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal = bool_val[j];
                     }
                 }
                 //無boolean
                 if(!change_val)
                 {
                     //cout << "no boolean" <<endl;
                     //判別有無項重複  
                     for(int j=0;j<LHS.size();++j)
                     {
                         if(sub_rhs[i][0] == LHS[j])
                         {
                              rhs_show_in_lhs = true;
                              change_val = true;
                              (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "D";
                              (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->gro_index1 = j;
                              enqueue(list1,*table[symbol_table(LHS[i],input_num,output_num,wire_count)]);
                         } 
                     }
                     if(!rhs_show_in_lhs){
                         (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping = "ID";                         
                         change_val = true;
                     }
                 }
                 change_val = false;
                 rhs_show_in_lhs=false;
             }
         }
    }
    
    
    //洗過一次後 
    for(int i = 0; i <  LHS.size(); i++) {
        if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping=="BW") //前面有項重複
        {
             if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal)
             {
                 (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->value = "1'b1";
             }
             else if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal==0)
             {
                 (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->value = "1'b0";
             }
        }  
        else if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping=="Out")
        {
             if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal)
             {
                 (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->value = "1'b1";
             }
             else if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal==0)
             {
                 (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->value = "1'b0";
             }
        }
    }
    
    //cout<<endl<<endl; 
    for(int i=0; i < LHS.size();++i)
    {
       // cout <<  (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->value<<"  ";
       // cout <<  (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping<<"  ";
       // cout <<  (*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->cal<<endl;
    }
    
    //針對queue內容再做一次化簡
    cout<<endl;
    showList(list1);
    cout <<listCount(list1)<<endl;
    
    NODE* temp = new NODE;
    int operand[2];
    operand[0] = -1;
    operand[1] = -1;
    temp = ( NODE*)(list1->front);
    change_val = false;
    int count = 0;
    
    //dequeue (list1, &(temp->dataPtr));
    //cout<<endl;
    //showList(list1);
    
    int cannot_sim = 0;
    
    while((!emptylist(list1)))
    {
        if(temp== NULL) temp = ( NODE*)(list1->front);
        for(int i=0;i<((tree_node *)(temp->dataPtr))->link_in.size();++i)
        {
            for(int j=0;j<4;++j)
            {
                if(((tree_node *)(temp->dataPtr))->link_in[i]->value == boolean[j])
                {
                    operand[i] = bool_val[j];
                    //cout << "same " << boolean[j] <<endl;
                }
            }
        }
        
        if((operand[0]!=-1) && (operand[1]!=-1))
        {
            if(((tree_node *)(temp->dataPtr))->grouping == "DW")
            {
                ((tree_node *)(temp->dataPtr))->grouping = "BW";
            }
            else
            {
                ((tree_node *)(temp->dataPtr))->grouping = "Out";
            }
            if(((tree_node *)(temp->dataPtr))->op=="&")  ((tree_node *)(temp->dataPtr))->cal = operand[0] & operand[1];
            else if(((tree_node *)(temp->dataPtr))->op=="|") ((tree_node *)(temp->dataPtr))->cal = operand[0] | operand[1];
            else if(((tree_node *)(temp->dataPtr))->op=="^") ((tree_node *)(temp->dataPtr))->cal = operand[0] ^ operand[1];
            
            if(((tree_node *)(temp->dataPtr))->cal)  ((tree_node *)(temp->dataPtr))->value = "1'b1";
            else  ((tree_node *)(temp->dataPtr))->value = "1'b0";
            
            dequeue (list1, &(temp->dataPtr));
            change_val = true;
            //break;
        }
        else if(operand[0]!=-1 && (((tree_node *)(temp->dataPtr))->link_in.size()==1))
        {
            cout << "only one element"<<endl;
            if(((tree_node *)(temp->dataPtr))->grouping == "DW")
            {
                ((tree_node *)(temp->dataPtr))->grouping = "BW";
            }
            else
            {
                ((tree_node *)(temp->dataPtr))->grouping = "Out";
            }
            ((tree_node *)(temp->dataPtr))->cal = operand[0];
            if(((tree_node *)(temp->dataPtr))->cal)  ((tree_node *)(temp->dataPtr))->value = "1'b1";
            else  ((tree_node *)(temp->dataPtr))->value = "1'b0";
            dequeue (list1, &(temp->dataPtr));
            change_val = true;
        }
        if(!change_val)
        {
            //cout << "check" << endl;
            count++;
            for(int i=0;i<((tree_node *)(temp->dataPtr))->link_in.size();++i)
            {
                if((((tree_node *)(temp->dataPtr))->link_in[i])->grouping=="IW" || (((tree_node *)(temp->dataPtr))->link_in[i])->grouping=="ID")
                {
                    cannot_sim++;
                    //dequeue (list1, &(temp->dataPtr));
                }
                else if((((tree_node *)(temp->dataPtr))->link_in[i])->index<input_num)
                {
                    cannot_sim++;
                    //cout << "delete input" << endl;
                    //dequeue (list1, &(temp->dataPtr));
                }
                else if((((tree_node *)(temp->dataPtr))->link_in[i])->index<(input_num*2+output_num))
                {
                    if((((tree_node *)(temp->dataPtr))->link_in[i])->index>(input_num+output_num))
                    {
                        cannot_sim++;
                        //dequeue (list1, &(temp->dataPtr));
                    }
                }
                else if(((((tree_node *)(temp->dataPtr))->link_in[i])->cal==0)||((((tree_node *)(temp->dataPtr))->link_in[i])->cal==1))
                {
                    cannot_sim++;
                }
            }
            //確定一定無法化簡
            if(cannot_sim==((tree_node *)(temp->dataPtr))->link_in.size())
            {
                //cout << "delete"<<endl;
                dequeue (list1, &(temp->dataPtr));
                change_val = true;
            }
            if(count>1000)
            {
                break;
            }
        }
        
        
        temp = temp->next;
        
        cannot_sim = 0;
        change_val =false;
        operand[0] = -1;
        operand[1] = -1;
    }
    
    //cout<<endl;
    //showList(list1);
    
    
    //產生新檔案(化簡)
    outfile << "module out(out,in);"<<endl;
    outfile <<"\t" << "output["<<output_num-1<<":0] out;"<<endl;
    outfile <<"\t" << "input["<<input_num-1<<":0] in;"<<endl;
    
    for(int i=1;i<=wire_count;++i)
    {
        outfile <<"\t" << "wire origtmp"<<i<<";"<<endl;    
    }
    //outfile<<endl;
    
    for(int i=0;i<LHS.size();++i)
    {
        if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping != "BW")
        {
            if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->grouping == "Out")
            {
                outfile <<"\t" << "assign "<<LHS[i]<<" = "<<(*table[symbol_table(LHS[i],input_num,output_num,wire_count)])-> value<<";"<<endl;
            }
            else{
                //cout << sub_rhs[i][0] <<endl;
                if((*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->link_in.size()==1){
                    outfile <<"\t" << "assign "<<LHS[i]<<" = "<<(*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->link_in[0]-> value<<";"<<endl; 
                }
                else{
                    outfile <<"\t" << "assign "<<LHS[i]<<" = "<<(*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->link_in[0]-> value
                    <<" " <<(*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->op<<" "<<(*table[symbol_table(LHS[i],input_num,output_num,wire_count)])->link_in[1]-> value << ";"<<endl; 
                }
            }
        }
        
    }
    //outfile << endl;
    outfile << "endmodule";
    
    
    
    infile.close();
    outfile.close();

    return 0;
}
