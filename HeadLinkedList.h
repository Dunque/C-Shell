#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//typedef struct node;
typedef struct node *position;
typedef struct node *list;

struct node *CreateNode();
list CreateList();
int IsEmptyList(list l);
position First(list l);
position Next(position p);
int IsEndOfList(position p);
int IsLast(struct node *p);
//position FindPrevious(list l, char x[], int (*comp)(const void *, const void *));
position Last(list l);
void Insert(char x[], list l);
void printList(list l);
void removeLastItem(list l);
void destroyList(list l);
