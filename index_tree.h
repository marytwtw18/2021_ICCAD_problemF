#include<iostream>
#include<vector>
#include<string>

using namespace std;

//store graph infor and avl tree 
typedef struct tree
{
    //1.input 2.output 3.~input 4.~output 5.wire 6.~wire
    int index;  //symbol table轉化出來的值
    string op;
    string value;   //一開始的string 
    
    string grouping; //分類
    int cal; //logic 結果
    int gro_index1,gro_index2;
    
    struct tree* left;
    struct tree* right;

    vector<struct tree*> link_in;  //儲存相關之rhs變數狀況
    vector<struct tree*> link_out;  //儲存相關之lhs變數狀況

}tree_node;

typedef struct
{
	 int   count;
	 tree_node*  root;
   
} Avl_tree;

Avl_tree *create()
{
   Avl_tree* tree = new Avl_tree;
    
    if(tree)
    {
        tree->root = NULL;
        tree->count = 0;
    }
    
    return tree;
}
tree_node* insert(Avl_tree* tree,tree_node* root,tree_node* newptr);
void insert_infor(tree_node* root,int in_or_out);
void insert_op(tree_node* root,string op);

bool tree_insert( Avl_tree* tree,int index,string value)
{
    tree_node* newptr;
    newptr = new tree_node;
    
    if(!newptr) return false;
    
    newptr->right = NULL;
    newptr->left = NULL;
    newptr->index = index;
    newptr->value = value;
    newptr->grouping = " ";
    newptr->cal = -1;
    newptr->gro_index1 = -1;
    newptr->gro_index2 = -1;
    
    if(tree->count == 0)
    {
        tree->root = newptr;
    }
    else
    {
        insert(tree,tree->root,newptr);
    }
    
    (tree->count)++;
    return true;
}

tree_node* insert(Avl_tree* tree,tree_node* root,tree_node* newptr)
{
    if(!root)  return newptr;
    
    if(newptr->index >= root->index)
    {
        root->right = insert(tree,root->right,newptr);
        return root;
    }
    else
    {
        root->left = insert(tree,root->left,newptr);
        return root;
    }
    return root;
}

int tree_count(Avl_tree* tree)
{
    return (tree->count);
}

//inorder
//tree_node* inorder_()

void inorder(tree_node* root)
{
     if(root)
     {
         inorder(root->left);
         cout << root->index <<endl;
         inorder(root->right);
     }   
}

tree_node* find_index(tree_node* root,int index)
{
    while(root)
    {
        if(index == root->index)
        {
            return root;
        }
        else if(index >= root->index)
        {
            root = root->right;
        }
        else
        {
            root = root->left;
        }
    }
    return root;
    
}

void insert_tworhs(tree_node* LHS,tree_node* RHS1,tree_node* RHS2,string op)
{
    insert_op(LHS,op);
    (LHS->link_in).push_back(RHS1);
    (LHS->link_in).push_back(RHS2);
    (RHS1->link_out).push_back(LHS);
    (RHS2->link_out).push_back(LHS);
}

void insert_rhs(tree_node* LHS,tree_node* RHS)
{
    (LHS->link_in).push_back(RHS);
    (RHS->link_out).push_back(LHS);
}


//儲存的是index
void insert_tree_infor(tree_node* root,int index,int in_or_out,string op)
{
    if(root)
    {
        if(index == root->index)
        {
            //insert_infor(root,in_or_out);
            insert_op(root,op);
        }
        else if(index >= root->index)
        {
            insert_tree_infor(root->right,index,in_or_out,op);
        }
        else
        {
            insert_tree_infor(root->left,index,in_or_out,op);
        }
    }
}

void insert_op(tree_node* root,string op)
{
    root->op = op;
}
