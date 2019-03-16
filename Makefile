all: clean mush

mush: mush.o execstages.o parsecommand.o
	gcc -ggdb -o mush mush.o execstages.o parsecommand.o

mush.o: mush.c mush.h execstages.h parsecommand.h
	gcc -ggdb -c mush.c

execstages.o: execstages.c execstages.h mush.h parsecommand.h
	gcc -ggdb -c execstages.c

parsecommand.o: parsecommand.c parsecommand.h
	gcc -ggdb -c parsecommand.c

clean:
	rm -f *.o mush
