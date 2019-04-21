CC=g++
DEPS = ext2_extractor.cc
OUT = ext2_extractor


make: $(DEPS)
	$(CC) $(DEPS) -o  $(OUT)  -std=c++11
debug: $(DEPS)
	$(CC) -g $(DEPS) -o $(OUT) -std=c++11

leak: debug
	valgrind --leak-check=yes ./$(OUT) '/home/mcolvin/Documents/filesystem/fstest' 
smash: debug
	valgrind --exp-sgcheck=yes ./$(OUT) '/home/mcolvin/Documents/filesystem/fstest' 


test: debug
	./$(OUT) '/home/mcolvin/Documents/filesystem/fstest' 

spongebob: debug
	./$(OUT) '/home/mcolvin/Documents/filesystem/fstest' '/stuff/spongebob.avi' >  spongebob.avi

smallfile:debug
	./$(OUT) '/home/mcolvin/Documents/filesystem/fstest' '/test2/2dirs/smallfile.txt' >  smallout.txt

clean: 
	rm -f *.o
	rm -f $(OUT)

extractor_run_predel: make 
	./$(OUT) testfs_before_delete
extractor_run_post: make
	./$(OUT) testfs_after_delete


extractor_deleted_entries: make
	./$(OUT) testfs_before_delete | grep -a "Deleted Entry Name:" -B 0 -A 1



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


