CC=g++
DEPS = tmufs.cc
OUT = tmufs.out


make: $(DEPS)
	$(CC) $(DEPS) -o  $(OUT)

debug: $(DEPS)
	$(CC) -g $(DEPS) -o $(OUT)

clean: 
	rm *.o
	rm $(OUT)
ex1: make
	./tmufs.out tmufs_example_1
ex2: make 
	./tmufs.out tmufs_example_2 
