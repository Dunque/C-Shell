CCFLAGS = -Wall -g

all: clean list.o HeadLinkedList.o listPR.o
	gcc $(CCFLAGS) -o shell shell.c list.o HeadLinkedList.o listPR.o

list.o: list.c list.h
	gcc $(CCFLAGS) -c list.c

listPR.o: listPR.c listPR.h
	gcc $(CCFLAGS) -c listPR.c

HeadLinkedList.o: HeadLinkedList.c HeadLinkedList.h
	gcc $(CCFLAGS) -c HeadLinkedList.c

clean:
	rm -f *.o shell

