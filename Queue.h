#include<iostream>
#include<vector>
#include<string>

using namespace std;

typedef struct node{
    void* dataPtr;
    struct node* next;
}NODE;

typedef struct{
    int count;
    NODE* front;
    NODE* rear;
}List;

List* createList(){
    List* q = (List*)malloc(sizeof(List));
    
    if(q){
        q->count = 0;
        q->front = NULL;
        q->rear = NULL;
    }
    return q;
}

int listCount(List* list)
{
//	Statements
	return list->count;
}	

bool emptylist (List* list)
{
//	Statements
	return (list->count == 0);
}	

void enqueue(List* list,void* dataInPtr){
    NODE* newNode = (NODE*)malloc(sizeof(NODE));
    newNode->dataPtr = dataInPtr;
    newNode->next = NULL;
    if(list->count == 0){
        list->front = newNode;
        list->rear = newNode;
    }
    else{
       list->rear->next = newNode;
        list->rear = newNode;
    }
    list->count++;
}
bool dequeue (List* list, void** itemPtr)
{
//	Local Definitions
	NODE* pred,*dloc;
  pred = NULL;
  dloc = list->front;

//	Statements
	if (!list->count)
	    return false;
         
	if (list->count == 1)
	{   // Deleting only item in queue
     free (dloc);
     (list->count)--;
	   list->rear  = list->front = NULL;
  }   
	else
  {  
	   while(dloc != NULL)
     {
         if(((tree_node *)(dloc->dataPtr))->index == ((tree_node *)(*itemPtr))->index)
         {
             //cout << "del:"<<endl;
             if(dloc == list->front) list->front =  list->front->next;
             else if(dloc == list->rear) list->rear = pred;
             if(pred!=NULL)  pred->next= dloc->next;
	           free (dloc);
      	     (list->count)--;
             break;
         }
         else
         {
             //cout<<"next ";
             pred = dloc;
             dloc = dloc->next;
         } 
     }
  }


	return true;
}	// dequeue

void showList(List* list)
{
    NODE *ploc = list->front;
    while(ploc != NULL)
    {
        cout << ((tree_node *)(ploc->dataPtr))->value << " " <<  ((tree_node *)(ploc->dataPtr))->grouping <<endl;
        cout <<"element: ";
        for(int i=0;i<((tree_node *)(ploc->dataPtr))->link_in.size();++i)
        {
            cout << ((tree_node *)(ploc->dataPtr))->link_in[i]->value<<" ";
        }
        cout<<endl;
        ploc = ploc->next;
    }
}