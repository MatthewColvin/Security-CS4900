#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cmath>
#include <algorithm> 
#include "ext2fs.h"

using namespace std;

void to_block(int block, ifstream& ext2, ext2_super_block &superblock){   
    ext2.seekg((1024 >> superblock.s_log_block_size) * block);
}


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
    << "Number of Pre-Allocate Blocks: " << (int)superblock.s_prealloc_blocks << endl // had to cast to int to get it to display the 0 hope it dosent mess up anything.
    << "Number of Pre-Allocate Directory Blocks: " << (int)superblock.s_prealloc_dir_blocks << endl
    << "Inode size: "<< max((int)superblock.s_inode_size,(int)sizeof(struct ext2_inode)) << endl; 

    
    if(superblock.s_log_block_size == 0 ){ // the block size is 1024 bytes so the group descriptor table starts at logic block 2  
        to_block(2,ext2,superblock);
    }else{ //starts at block 1 otherwize
        to_block(1,ext2,superblock);
    }
    // stream pointer now at the group descriptor 

    int NumGroupDecs = 1 + (superblock.s_blocks_count-1) / superblock.s_blocks_per_group; // **********better understand this formula*****

    ext2_group_desc GroupDec[NumGroupDecs];
    for (int i=0;i<NumGroupDecs; i++){
        ext2.read((char*)&GroupDec[i],sizeof(GroupDec[i]));
    }

    cout << endl << endl;

    for (int j=0; j<NumGroupDecs;j++){
        cout << "Information for Block Group " << j << endl
        << "-------------------------------------------------------------------------------" << endl
        << "Group Range: " << (superblock.s_blocks_per_group*j)+1 << " to "<< min(superblock.s_blocks_count-1 , (superblock.s_blocks_per_group*(j+1))) << endl
        << "Block Bitmap Block at: " << GroupDec[j].bg_block_bitmap << endl
        << "Inodes Bitmap Block at: " << GroupDec[j].bg_inode_bitmap << endl
        << "Free Blocks Count: " << GroupDec[j].bg_free_blocks_count << endl
        << "Free Inodes Count: " << GroupDec[j].bg_free_inodes_count << endl
        << "Used Directories Count: " << GroupDec[j].bg_used_dirs_count << endl
        << "-------------------------------------------------------------------------------" << endl << endl;
    }
    return 0;
} 