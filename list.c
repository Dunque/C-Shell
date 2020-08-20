#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct line {
  void* address;
  size_t size;
  char *cmd;
  int clave;
  time_t tm;
  char *file;
} line;

typedef struct mNode {
  struct mNode *Next;
  line data;
} mNode;

typedef struct mNode *mPosition;
typedef struct mNode *memList;

struct mNode *mCreateNode() {
  struct mNode *tmp = malloc(sizeof(struct mNode));
  if (tmp == NULL) {
      printf("Out of space\n");
      exit(EXIT_FAILURE);
    }
    return tmp;
}

memList mCreateList() {
  struct mNode *l = mCreateNode();
  l->Next = NULL;
  return l;
}

int mIsEmptyList(memList l) {return (l->Next == NULL);}

mPosition mFirst(memList l) {return l->Next;}

mPosition mNext(mPosition p) {return p->Next;}

int mIsEndOfList(mPosition p) {return (p==NULL);}

int mIsLast(struct mNode *p) {return (p->Next == NULL);}

void* mPointer(mPosition p) {return p->data.address;}
size_t mSize(mPosition p) {return p->data.size;}
char* mCmd(mPosition p) {return p->data.cmd;}
char* mFile(mPosition p) {return p->data.file;}
int mClave(mPosition p) {return p->data.clave;}
time_t mTime(mPosition p) {return p->data.tm;}

mPosition mLast(memList l) {
  if (mIsEmptyList(l)) {
    return l;
  } else {
    struct mNode *q = mCreateNode();
    q = mFirst(l);
    while (mIsLast(q) == 0) {
      q=mNext(q);
    }
      return q;
  }
}

mPosition mFindAdr(memList l, long int adr) {
  struct mNode *p = l;
  while (p != NULL && (long) p->data.address != adr)
    p = p->Next;
  return p;
}

mPosition mFindPreviousAdr(memList l, long int adr) {
  struct mNode *p = l;
  while (p->Next != NULL && (long) p->Next->data.address != adr)
    p = p->Next;
  return p ;
}

void mDeleteAdr(memList l, long int adr) {
  struct mNode *tmp, *p = mFindPreviousAdr(l,adr);

  if (!mIsLast(p)) {
    tmp = p->Next;
    p->Next = tmp->Next;
    free(tmp->data.file);
    free(tmp->data.cmd);
    free(tmp);
  }
}

mPosition mFindSize(memList l, int size) {
  struct mNode *p = l;
  while (p != NULL && (int) p->data.size != size)
    p = p->Next;
  return p;
}

mPosition mFindPreviousSize(memList l, int size) {
  struct mNode *p = l;
  while (p->Next != NULL && (int) p->Next->data.size != size)
    p = p->Next;
  return p ;
}

void mDeleteSize(memList l, int size) {
  struct mNode *tmp, *p = mFindPreviousSize(l,size);

  if (!mIsLast(p)) {
    tmp = p->Next;
    p->Next = tmp->Next;
    free(tmp->data.file);
    free(tmp->data.cmd);
    free(tmp);
  }
}

mPosition mFindClave(memList l, int clave){
  struct mNode *p = l;
  while (p != NULL && (int) p->data.clave != clave)
    p = p->Next;
  return p;
}

mPosition mFindPreviousClave(memList l, int clave) {
  struct mNode *p = l;
  while (p->Next != NULL && p->Next->data.clave != clave)
    p = p->Next;
  return p ;
}

void mDeleteClave(memList l, int clave) {
  struct mNode *tmp, *p = mFindPreviousClave(l,clave);

  if (!mIsLast(p)) {
    tmp = p->Next;
    p->Next = tmp->Next;
    free(tmp->data.file);
    free(tmp->data.cmd);
    free(tmp);
  }
}

mPosition mFindFile(memList l, char* file) {
  struct mNode *p = l;
  while (p != NULL && strcmp(p->data.file,file) != 0)
    p = p->Next;
  return p;
}

mPosition mFindPreviousFile(memList l, char* file) {
  struct mNode *p = l;
  while (p->Next != NULL && strcmp(p->Next->data.file,file) != 0)
    p = p->Next;
  return p ;
}

void mDeleteFile(memList l, char* file) {
  struct mNode *tmp, *p = mFindPreviousFile(l,file);

  if (!mIsLast(p)) {
    tmp = p->Next;
    p->Next = tmp->Next;
    free(tmp->data.file);
    free(tmp->data.cmd);
    free(tmp);
  }
}

void mInsert(memList l, void* a, size_t s ,char *c, int clave, char *file) {
  struct mNode *tmp = mCreateNode();
  tmp->data.address = a;
  tmp->data.size = s;
  tmp->data.cmd=strdup(c);
  tmp->data.tm = time(NULL);
  tmp->data.clave = clave;
  tmp->data.file=strdup(file);
  tmp->Next = NULL;
  mLast(l)->Next = tmp;
}

void mPrintList(memList l) {
    struct mNode *q = mCreateNode();
    q = l;
      while (mIsLast(q) == 0) {
        q=mNext(q);
          printf("%p size:%ld %s", q->data.address, q->data.size, q->data.cmd);
          if (strcmp("mmap",q->data.cmd)==0)
            printf(" file:%s", q->data.file);
          else if (strcmp("shared",q->data.cmd)==0) {
            printf(" fd:%d", q->data.clave);}

          char date[50];
          struct tm *tm = localtime(&q->data.tm);
          strftime(date,sizeof date+10,"%a %b %d %H:%M:%S %Y",tm);
          printf(" %s\n", date);
      }
}

void mPrintListParam(memList l, char* cmd) {
    struct mNode *q = mCreateNode();
    q = l;
    while (mIsLast(q) == 0) {
      q=mNext(q);
      if (strcmp(cmd,q->data.cmd)==0){
        printf("%p size:%ld %s", q->data.address, q->data.size, q->data.cmd);
        if (strcmp("mmap",q->data.cmd)==0)
          printf(" file:%s", q->data.file);
        else if (strcmp("shared",q->data.cmd)==0) {
          printf(" fd:%d", q->data.clave);}

      struct tm *tm = localtime(&q->data.tm);
      char date[50];
      strftime(date,sizeof date+10,"%a %b %d %H:%M:%S %Y",tm);
      printf(" %s\n", date);
    }
  }
}

void mRemoveLastItem(memList l){
  struct mNode *deleteNode = mCreateNode();

	if (mIsEmptyList(l) == 1){
		return;
	}
	else if (mNext(mNext(l)) == NULL){
		l -> Next = NULL;
	}
	else {
		struct mNode *auxNode = mCreateNode();
		deleteNode = l -> Next;
		while(mNext(deleteNode) != NULL){

			auxNode = deleteNode;

			deleteNode = deleteNode -> Next;
		}

		free(auxNode->Next);
		auxNode -> Next = NULL;
	}
}

void mDestroyList(memList l) {
    while (mNext(l) != NULL) {
        mRemoveLastItem(l);
    }
}
