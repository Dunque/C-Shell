#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct mNode *mPosition;
typedef struct mNode *memList;

struct mNode *mCreateNode();
memList mCreateList();
int mIsEmptyList(memList l);
mPosition mFirst(memList l);
mPosition mNext(mPosition p);
int mIsEndOfList(mPosition p);
int mIsLast(struct mNode *p);
//mPosition mFindPrevious(memList l, char x[], int (*comp)(const void *, const void *));
mPosition mLast(memList l);
void mInsert(memList l, void* a, size_t s ,char* c, int clave, char* file);
void mPrintList(memList l);
void mPrintListParam(memList l, char* cmd);
void mRemoveLastItem(memList l);
void mDestroyList(memList l);

void* mPointer(mPosition p);
size_t mSize(mPosition p);
char* mCmd(mPosition p);
char* mFile(mPosition p);
int mClave(mPosition p);
time_t mTime(mPosition p);

mPosition mFindAdr(memList l, long int adr);
mPosition mFindPreviousAdr(memList l, long int adr);
void mDeleteAdr(memList l, long int adr);

mPosition mFindSize(memList l, int size);
mPosition mFindPreviousSize(memList l, size_t size);
void mDeleteSize(memList l, size_t size);

mPosition mFindClave(memList l, int clave);
mPosition mFindPreviousClave(memList l, int clave);
void mDeleteClave(memList l, int clave);

mPosition mFindFile(memList l, char* file);
mPosition mFindPreviousFile(memList l, char* file);
void mDeleteFile(memList l, char* file);
