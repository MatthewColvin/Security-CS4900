CC=g++
DEPS = project2.cc
OUT = extdump.out


make: $(DEPS)
	$(CC) $(DEPS) -o  $(OUT)

debug: $(DEPS)
	$(CC) -g $(DEPS) -o $(OUT)

clean: 
	rm -f *.o
	rm -f $(OUT)

fspart_large: make
	./$(OUT) givenfiles/fspart_large
fspart_small: make
	./$(OUT) givenfiles/fspart_small
fspart_small_empty: make
	./$(OUT) givenfiles/fspart_small_empty
fspart_small_full: make
	./$(OUT) givenfiles/fspart_small_full
fspart_small_one_file: make
	./$(OUT) givenfiles/fspart_small_one_file
fspart_small_some_files: make
	./$(OUT) givenfiles/fspart_small_some_files


