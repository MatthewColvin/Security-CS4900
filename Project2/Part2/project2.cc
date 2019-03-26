#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm> 
#include "ext2fs.h"

using namespace std;

class ext2fs {

    public:

    void dump(){
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
    
        for (int j=0; j<GroupDec.size();j++){
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
    }

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
        ext2.read((char*)&ext2_in, sizeof(struct ext2_inode)); // may produce bugs inode size

        return ext2_in;
    }

    vector<ext2_dir_entry_2> get_directory_enteries(ext2_inode inode){
        vector<ext2_dir_entry_2> enteries;
        ext2_dir_entry_2* entry;
        char block[blocksize];
        
        for (int j=0; j <= 12 ; j++){    // for all direct pointers
            if(inode.i_block[j]!=0){     // if is is equal to zero there is nothing being pointed at
                to_block(inode.i_block[j]);
                ext2.read(block,blocksize);
                int entrynumber=0;
                entry = (struct ext2_dir_entry_2*) &block[0];
                string filename;
                for (int i =0 ; i < blocksize; entrynumber++){
                    if(entry->inode != 0){   // may cause infinate loop if first entry is a 0
                        entry = (struct ext2_dir_entry_2*) &block[i];
                        filename=entry->name;
                        filename = filename.substr(0,entry->name_len);
                        i += entry->rec_len;
                        cout << filename << endl;
                        enteries.resize(entrynumber+1);
                        enteries[entrynumber].file_type = entry->file_type;
                        enteries[entrynumber].inode = entry->inode;
                        strcpy(enteries[entrynumber].name,filename.c_str());
                        enteries[entrynumber].name_len = entry->name_len;
                        enteries[entrynumber].rec_len = entry->rec_len;
                    }else{
                        entry = (struct ext2_dir_entry_2*) &block[i];
                        i+=entry->rec_len;
                    }
                    

                }
            }
        }

        return enteries;
    }

    void get_data_at_path(string path){  
        string filetofind,filename;
        
        const char* delim = "/";
        char* path1 = &path[0];
        filetofind = strtok(path1,delim);
       
        vector<ext2_dir_entry_2> entries=
        get_directory_enteries(get_inode(2));

        while (strtok != NULL){
            for(int i=0; i < entries.size(); i++ ){   // iterate of all entries 
                filename = entries[i].name;     // modify so it is a proper string
                if (filename == filetofind){     // did we find the file?
                    entries = get_directory_enteries(get_inode(entries[i].inode));  // get directory entries at the inode specified by the entry with propername
                }else{
                    cout << "FILE OR DIRECORY NOT FOUND" << endl; 
                }
                
            }
            filetofind = strtok(NULL,delim);
        }

    
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

    ext2fs1.get_data_at_path(path);

    


    return 0;
} 