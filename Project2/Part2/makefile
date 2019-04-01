CC=g++
DEPS = project2.cc
OUT = ext2.out


make: $(DEPS)
	$(CC) $(DEPS) -o  $(OUT)  
debug: $(DEPS)
	$(CC) -g $(DEPS) -o $(OUT) 

leak: debug
	valgrind --leak-check=yes ./$(OUT) '/home/mcolvin/Documents/filesystem/fstest' '/matt/note'

smash: debug
	valgrind --exp-sgcheck=yes ./$(OUT) '/home/mcolvin/Documents/filesystem/fstest' '/matt/note'


test: debug
	./$(OUT) '/home/mcolvin/Documents/filesystem/fstest' '/matt/note' > out.txt

spongebob: debug
	./$(OUT) '/home/mcolvin/Documents/filesystem/fstest' '/stuff/spongebob.avi' >  spongebob.avi

smallfile:debug
	./$(OUT) '/home/mcolvin/Documents/filesystem/fstest' '/test2/2dirs/smallfile.txt' >  smallout.txt

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


