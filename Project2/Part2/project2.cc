#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm> 
#include "ext2fs.h"

using namespace std;


ext2_inode get_inode(int i){

}

class ext2fs {

    public:

    void to_block(int block){   
        ext2.seekg(blocksize * block);
    }


    int get_superblock(){
        ext2.seekg(1024);// superblock starts exactly 1024 into the ext2 filesystem
        ext2.read((char*)&superblock,sizeof(superblock));
        if (superblock.s_magic != 0xef53){
        cout << "NOT A VALID ext2 filesystem"<< endl;
        return -1;
        }
        blocksize = 1 << (10 + superblock.s_log_block_size);
        inode_size = max((int)superblock.s_inode_size,(int)sizeof(struct ext2_inode));
        return 0;
    }
    void open_filesystem(string filesystem_name){
        ext2.open(filesystem_name, ios::binary);
        if (ext2.fail()){
            cout<< "Sorry could not open file" << endl;
        }
    }
    void get_group_decs(){
        if(superblock.s_log_block_size == 0 ){ // the block size is 1024 bytes so the group descriptor table starts at logic block 2  
            to_block(2);
        }else{ //starts at block 1 otherwize
            to_block(1);
        }
        int NumGroupDecs = 1 + (superblock.s_blocks_count-1) / superblock.s_blocks_per_group; // **********better understand this formula*****
            GroupDec.resize(NumGroupDecs);
            for (int i=0;i<NumGroupDecs; i++){
                ext2.read((char*)&GroupDec[i],sizeof(GroupDec[i]));
            }
    }
    ext2_inode get_inode(int inode_index){
        unsigned int i_group = ( inode_index -1 ) / superblock.s_inodes_per_group;
        unsigned int i_index = ( inode_index -1 ) % superblock.s_inodes_per_group;

        unsigned int i_offset =  blocksize * GroupDec[i_group].bg_inode_table + (i_index * inode_size);
        
        struct ext2_inode ext2_in;
        
        ext2.seekg(i_offset);
        ext2.read((char*)&ext2_in, sizeof(struct ext2_inode));

        return ext2_in;
    }

    vector<ext2_dir_entry_2*> get_directory_enteries(ext2_inode inode){
        vector<ext2_dir_entry_2*> enteries;
        ext2_dir_entry_2* entry;
        char block[blocksize];
        
        for (int j=0; j <= 12 ; j++){    // for all direct pointers
            if(inode.i_block[j]!=0){     // if is is equal to zero there is nothing being pointed at
                to_block(inode.i_block[0]);
                ext2.read(block,blocksize);

                entry = (struct ext2_dir_entry_2*) &block[0];
                string filename;
                for (int i =0 ; i < blocksize; ){
                
                    if(entry->inode != 0){
                        entry = (struct ext2_dir_entry_2*) &block[i];
                        enteries.push_back(entry);
                        filename=entry->name;
                        i += entry->rec_len;
                        cout << filename << endl;
                    }
                }
            }
        }

        return enteries;
    }


    private:

    int blocksize,inode_size;
    ext2_super_block superblock;
    vector<ext2_group_desc> GroupDec;
    ifstream ext2;

};



int main(int argc, char **argv){
    ifstream ext2;
    string ext2_name,path;
    switch (argc){
        case 3:
            ext2_name = argv[1];
            path = argv[2];
            break;
        case 2:
            ext2_name = argv[1];
            cout << "Please enter the filesystem path" << endl;
            cin >> path;
            break;
        case 1:
            cout << "Please enter the filesystem name now" << endl;
            cin >>  ext2_name;
            cout << "Please enter the filesystem path" << endl;
            cin >> path;
    }

    ext2fs ext2fs1;

    ext2fs1.open_filesystem(ext2_name);
    if(ext2fs1.get_superblock() == -1){return -1 ;}// didnt get correct magic number
    ext2fs1.get_group_decs();

    ext2_inode test = ext2fs1.get_inode(2);

    
    ext2fs1.get_directory_enteries(test);
    


    return 0;
} 