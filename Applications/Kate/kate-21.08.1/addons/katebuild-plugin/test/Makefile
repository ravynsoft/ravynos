all: test

main.o: main.c
	gcc -Wall -g -Isubdir -c -o main.o main.c

enter_exit_subdir.o: subdir/enter_exit_subdir.c subdir/enter_exit_subdir.h
	cd ./subdir/ && make
	mv ./subdir/*.o ./

incl_from.o: incl_from.c incl_from.h incl_from_from.h
	gcc -Wall -g -c -o incl_from.o incl_from.c

test: clean enter_exit_subdir.o incl_from.o main.o
	gcc -Wall -g main.o incl_from.o -o test
    

clean:
	rm -rf *.o test