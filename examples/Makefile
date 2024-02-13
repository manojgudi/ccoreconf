CC=gcc
CFLAGS=-fdiagnostics-color=always -g -I/home/valentina/projects/lpwan_examples/build_libyang/install/include/ -L/home/valentina/projects/lpwan_examples/build_libyang/install/lib/ -lyang -I/home/valentina/projects/lpwan_examples/build_libcbor/install/include -L/home/valentina/projects/lpwan_examples/build_libcbor/install/lib -I/home/valentina/projects/lpwan_examples/build_libjansson/install/include/ -L/home/valentina/projects/lpwan_examples/build_libjansson/install/lib/ -ljansson -lm -lcbor

all: executable

executable: hashmap.o sid.o fileOperations.o ccoreconf.o demo_functionalities.o
	$(CC) $(CFLAGS) -o executable demo_functionalities.o hashmap.o sid.o fileOperations.o ccoreconf.o

demo_functionalities.o: demo_functionalities.c
	$(CC) $(CFLAGS) -c demo_functionalities.c

hashmap.o: ../src/hashmap.c
	$(CC) $(CFLAGS) -c ../src/hashmap.c

sid.o: ../src/sid.c
	$(CC) $(CFLAGS) -c ../src/sid.c

fileOperations.o: ../src/fileOperations.c
	$(CC) $(CFLAGS) -c ../src/fileOperations.c

ccoreconf.o: ../src/ccoreconf.c
	$(CC) $(CFLAGS) -c ../src/ccoreconf.c

clean:
	rm -f *.o executable
