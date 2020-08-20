#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
  struct node *Next;
  char * Element;
} node;

typedef struct node *position;
typedef struct node *list;

struct node *CreateNode() {
  struct node *tmp = malloc(sizeof(struct node));
  if (tmp == NULL) {
      printf("Out of space\n");
      exit(EXIT_FAILURE);
    }
    return tmp;
}

list CreateList() {
  struct node *l = CreateNode();
  l->Next = NULL;
  return l;
}

int IsEmptyList(list l) {return (l->Next == NULL);}

position First(list l) {return l->Next;}

position Next(position p) {return p->Next;}

int IsEndOfList(position p) {return (p==NULL);}

int IsLast(struct node *p) {return (p->Next == NULL);}

/*static position FindPrevious(list l, char x[], int (*comp)(const void *, const void *)) {
  struct node *p = l;
  while (p->Next != NULL && 0!=(*comp)(p->Next->Element, x))
    p = p->Next;
   return p ;
}*/

position Last(list l) {
  if (IsEmptyList(l)) {
    return l;
  } else {
    struct node *q = CreateNode();
    q = First(l);
    while (IsLast(q) == 0) {
      q=Next(q);
    }
    return q;
  }

}

void Insert(char * x, list l) {
  struct node *tmp = CreateNode();
  tmp->Element=strdup(x);
  tmp->Next = NULL;
  Last(l)->Next = tmp;
}

void printList(list l) {
    struct node *q = CreateNode();
    q = l;
    while (IsLast(q) == 0) {
      q=Next(q);
      printf("%s", q->Element);
    }
}

void removeLastItem(list l){
  struct node *deleteNode = CreateNode();

	if (IsEmptyList(l) == 1){
		return;
	}
	else if (Next(Next(l)) == NULL){
		l -> Next = NULL;
	}
	else {
		struct node *auxNode = CreateNode();
		deleteNode = l -> Next;
		while(Next(deleteNode) != NULL){

			auxNode = deleteNode;

			deleteNode = deleteNode -> Next;
		}

    free(auxNode->Next->Element);
		free(auxNode->Next);
		auxNode -> Next = NULL;

	}
}

void destroyList(list l) {
    while (Next(l) != NULL) {
        removeLastItem(l);
    }
}
