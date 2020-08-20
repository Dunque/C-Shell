#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include "listPR.h"


int Senal(char *sen) { /*devuel el numero de senial a partir del nombre*/
  int i;
  for (i = 0; sigstrnum[i].nombre != NULL; i++)
    if (!strcmp(sen, sigstrnum[i].nombre))
      return sigstrnum[i].senal;
  return -1;
}

char *NombreSenal(int sen) { /*devuelve el nombre senal a partir de la senal para sitios donde no hay sig2str*/
  int i;
  for (i = 0; sigstrnum[i].nombre != NULL; i++)
    if (sen == sigstrnum[i].senal)
      return sigstrnum[i].nombre;
  return ("SIGUNKNOWN");
}

void pCreateList(procList * l)
{

	(*l) = (procList)malloc(sizeof(pList));

	if (l != NULL)
		(*l)->size = -1;
	else
		perror("Couldn't create array");
}

void pDeleteAt(procList l, int pos)
{

	free(l->procList[pos].cmd);
    while (pos < l->size)
    {
      l->procList[pos] = l->procList[pos + 1];
      pos++;
    }
    l->size--;
}

void pDeleteTerminated(procList l){
	int i;
	for (i=0;i<=l->size; i++){
		if ((l->procList[i].signal == 0) && (l->procList[i].state == 0)) {
      pDeleteAt(l, i);
      i--;
    }
	}
}

void pDeleteSignal(procList l){
	int i;
  for (i=0; i<=l->size; i++) {
		if (l->procList[i].signal != 0) {
      pDeleteAt(l, i);
      i--;
    }
	}
}

void pDestroyList(procList * l)
{
	free(l);
	l = NULL;
}

int pIsEmptyList(procList l)
{

	if (l->size < 0)
		return 1;
	else
		return 0;
}

int pSearchPid(procList l, pid_t pid){

	int pos = -1;
	int found = 0;

	while (pos <= l->size && found == 0) {
		pos++;
		if (l->procList[pos].pid == pid && l->procList[pos].state == 1)
			found = 1;
	}
	if (found == 0)
		return -1;
	else
		return pos;
}


int getSignal(int estado) {
  if (WIFSIGNALED(estado))
    return WTERMSIG(estado);
  else
    return 0;
}

int getState(int state) {
	if (WIFEXITED(state))
		return 1;
	else if (WIFSIGNALED(state))
		return 2;
	else if (WIFSTOPPED(state))
		return 3;
	else if (WIFCONTINUED(state))
		return 0;
  else return 4;
}

void pUpdateNodes(procList l){
	int estado;
	int pos = 0;
	while (pos <= l->size) {
		if(l->procList[pos].pid==waitpid(l->procList[pos].pid,&estado,WNOHANG|WUNTRACED|WCONTINUED)) {
      l->procList[pos].state=getState(estado);
      l->procList[pos].signal = getSignal(estado); }
		l->procList[pos].prior=getpriority(PRIO_PROCESS,l->procList[pos].pid);
		pos++;
	}
}

int pInsert(procList l, pid_t pid, char *cmd)
{
	int pos = l->size + 1;
	if (pos < MAX_SIZE)
	{
		l->procList[pos].pid = pid;
		l->procList[pos].prior = getpriority(PRIO_PROCESS, pid);
		l->procList[pos].cmd = strdup(cmd);
		l->procList[pos].time = time(0);
		l->procList[pos].state = 0;
    l->procList[pos].signal = 0;
		l->size++;
		return 1;
	}
	else
		return 0;
}

void printState(int state) {

	if (state == 1)
		printf("TERMINATED\n");
	else if (state == 2)
		printf("SIGNALED\n");
	else if (state == 3)
		printf("STOPPED\n");
	else if (state == 0)
		printf("ACTIVE\n");
}


void printElement(procList l, int pos)
{
	struct tm *info = localtime(&(l->procList[pos].time));
	char fecha[50];
  char* t = "";
  if (l->procList[pos].signal != 0) {
    t =  NombreSenal(l->procList[pos].signal);
  }
	strftime(fecha, 50, "%a %b %d %H:%M:%S %Y", info);
	printf(" %d p=%d %s %s %s ", l->procList[pos].pid, l->procList[pos].prior, fecha, l->procList[pos].cmd, t);
  printState(l->procList[pos].state);
}

void pPrintList(procList l)
{
	int i;

	if (!pIsEmptyList(l)) {
		pUpdateNodes(l);
		for (i = 0; i <= l->size; i++)
			printElement(l, i);
	}
}
