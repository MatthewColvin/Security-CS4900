#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cmath>
#include "ext2fs.h"

using namespace std;


int main(int argc, char **argv){

    ifstream ext2;
    string ext2_name;
    
    if(argc < 2){  // no argument was given so just ask for it now
        cout << "Please enter the filesystem name now" << endl;
        cin >>  ext2_name;
    }else{
        ext2_name = argv[1];
    }
    
    ext2.open(ext2_name, ios::binary);
    if (ext2.fail()){
        cout<< "Sorry could not open file" << endl;
        return (-1);
    }

    ext2_super_block superblock;

    ext2.seekg(1024);// superblock starts exactly 1024 into the ext2 filesystem
    ext2.read((char*)&superblock,sizeof(superblock));

    if (superblock.s_magic != 0xef53){
        cout << "NOT A VALID ext2 filesystem"<< endl;
        return(-1);
    }    

    ext2_group_desc GroupDec;

    cout << "Superblock magic number:" << hex <<superblock.s_magic << endl
    << "Inodes Count: " << dec << superblock.s_inodes_count << endl
    << "Block Count: " << superblock.s_blocks_count <<endl
    << "Free Blocks Count: " << superblock.s_free_blocks_count<< endl
    << "Free Inodes Count: " << superblock.s_free_inodes_count<<endl
    << "First Data Block: " << superblock.s_first_data_block<< endl
    << "Block Size: " << (1024 >> superblock.s_log_block_size) <<endl 
    << "Fragment Size: " << (1024 >> superblock.s_log_frag_size) << endl
    << "Blocks Per Group: " << superblock.s_blocks_per_group << endl 
    << "Fragments Per Group: "<< superblock.s_frags_per_group << endl
    << "Inodes Per Group: " << superblock.s_inodes_per_group << endl 
    << "Number of Block Groups: " << ceil((float)superblock.s_blocks_count / (float)superblock.s_blocks_per_group) << endl
    << "Number of Pre-Allocate Blocks: " << superblock.s_prealloc_blocks << endl
    << "Number of Pre-Allocate Directory Blocks: " << superblock.s_prealloc_dir_blocks << endl
    << "Inode size: "<< superblock.s_inode_size << endl; 

    



    return 0;

} 