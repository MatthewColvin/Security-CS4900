#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm> 
#include <sstream>
#include <iomanip>
#include <time.h>
#include "ext2fs.h"

using namespace std;
int blocksize=0; // issues with class segfautling
ifstream ext2; 
int inode_size;                     // all of these variable were falling out of scope
ext2_super_block superblock;       // in my private member functions causeing
vector<ext2_group_desc> GroupDec; // segfaults and stack buffer overflows
vector <string> filenames;
vector<ext2_dir_entry_2> entries;

ext2_inode get_inode(int inode_index){
    unsigned int i_group = ( inode_index -1 ) / superblock.s_inodes_per_group;
    unsigned int i_index = ( inode_index -1 ) % superblock.s_inodes_per_group;

    unsigned int i_offset =  blocksize * GroupDec[i_group].bg_inode_table + (i_index * inode_size);
        
    ext2_inode ext2_in;
        
    ext2.seekg(i_offset);
    ext2.read((char*)&ext2_in, inode_size); 

    return ext2_in;
    }




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
        << "Inode size: "<< max((int)superblock.s_inode_size,(int)sizeof(ext2_inode)) << endl; 
    
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
    
    void to_block(size_t block){   
        size_t byte_offset = block*blocksize; // private class member casing segfault
        ext2.seekg(byte_offset);
    }
    int get_superblock(){
        ext2.seekg(1024);// superblock starts exactly 1024 into the ext2 filesystem
        ext2.read((char*)&superblock,sizeof(superblock));
        if (superblock.s_magic != 0xef53){
        cout << "NOT A VALID ext2 filesystem"<< endl;
        return -1;
        }
        blocksize = 1 << (10 + superblock.s_log_block_size);
        inode_size = max((int)superblock.s_inode_size,(int)sizeof(ext2_inode));
        return 0;
    }
    void open_filesystem(string& filesystem_name){
        ext2.open(filesystem_name, ios::binary | ios::in);
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


    string get_time_of_access(ext2_inode inode){
        time_t a_rawtime = inode.i_atime;
        char date[255];
        struct tm* atime_tm;
        atime_tm = localtime(&a_rawtime);
        strftime(date, 255, "%b %d %R", atime_tm);
        string return_date(date);
        return(return_date);
    }

    string get_permission(ext2_inode inode){
        const unsigned int rights[9]={S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP,
        S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
        stringstream ss;
        const char* crights[3] = {"r", "w", "x"};
        if(S_ISDIR(inode.i_mode)){
            ss << 'd';
        }else{
            ss << '-';
        }
        for(int i = 0; i < 9; i++){
            if(inode.i_mode & rights[i] ){
                ss << crights[i % 3];
            }else{
                ss << "-";
            }
        }
        string tmp = ss.str();
        return (ss.str());
    }


    void dump_inode_data(ext2_inode inode){
        vector<ext2_dir_entry_2> enteries;
        ext2_dir_entry_2* entry;
        int numbytesout=0;

        for (int j=0; j < 12 && numbytesout < inode.i_size ; j++){    // for all direct pointers
            if(inode.i_block[j]!=0){     // if is is equal to zero there is nothing being pointed at
                to_block(inode.i_block[j]);
                
                char block[blocksize];
                ext2.read(block,blocksize);
                
                string filename;
                for (int i =0 ; i < blocksize && numbytesout < inode.i_size ;i++){
                    cout << hex << block[i];
                    numbytesout++;
                    // inode memeber iblocks stores the # of blocks used to store the file but it is 512 byte blocks not blocksize
                }
            }
        }
    

        

        if(inode.i_size > blocksize*12){// check if we need single indirect pointer 
            if(numbytesout != blocksize*12){cout << " check for off by one error" << endl;};// the size of the first 12 direct pointers in bytes 
            int datablock;
            int indirectblock[blocksize/4];// block is interpreted as an array of intgers each 4 bytes so blocksize/4 is the size
            to_block(inode.i_block[12]); // data block full of ints that point to the actual data blocks
            ext2.read((char *)indirectblock,blocksize);

            for (int j=0; j < blocksize/4 && numbytesout < inode.i_size ; j++){ // blksize/4 = #of ints that can be represented in one block
                to_block(indirectblock[j]);

                char tmpblk[blocksize];
                ext2.read(tmpblk,blocksize);
                
                for (int i = 0 ; i < blocksize && numbytesout < inode.i_size ;i++){ // output every byte of the file for whole block or untill we reach the inode size
                    cout << hex << tmpblk[i];
                    numbytesout++;
                    // inode memeber iblocks stores the # of blocks used to store the file but it is 512 byte blocks not blocksize
                }
            }
        }
        
        if(inode.i_size > (blocksize*12) + (blocksize/4)*blocksize ){ // check if we need doubly indirect pointer.
        //if(sizeoffile >  direct pointers + (#)OfIndirectPointers)*blksize) all in bytes
            if(blocksize*12+(blocksize/4) * blocksize!=numbytesout){cout<<"Check for off by one error" << endl;}// the size of the first 12 direct pointers in bytes 
            int datablock;
            int doubleindirectblock[blocksize/4];// block is interpreted as an array of intgers each 4 bytes so blocksize/4 is the size
            to_block(inode.i_block[13]); // data block full of ints that point to the actual data blocks
            ext2.read((char *)doubleindirectblock,blocksize);

            for (int j=0; j < blocksize/4 && numbytesout < inode.i_size ; j++){ // blksize/4 = #of ints that can be represented in one block
                to_block(doubleindirectblock[j]); // every doubleindirectblock is treated as an array of ints that then point at data blocks
                int pointerblk[blocksize];
                ext2.read((char*)pointerblk,blocksize); // this will be an a block that points to the datablocks
                
                for(int i=0; i<blocksize/4 && numbytesout < inode.i_size; i++){
                    to_block(pointerblk[i]);
                    char data[blocksize];      // will hold data after 2 levels of indirection
                    ext2.read(data,blocksize);
                    for (int a=0;a<blocksize;a++){
                        cout << hex << data[a]; 
                        numbytesout++;
                    }


                }

            }

        }
        


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
                entry = (ext2_dir_entry_2*) &block[0];
                string filename;
                for (int i =0 ; i < blocksize; entrynumber++){
                    if(entry->inode != 0){   // may cause infinate loop if first entry is a 0
                        entry = (ext2_dir_entry_2*) &block[i];
                        filename=entry->name;
                        filename = filename.substr(0,entry->name_len);
                        i += entry->rec_len;
                        //cout << filename << endl;
                        enteries.resize(entrynumber+1);
                        enteries[entrynumber].file_type = entry->file_type;
                        enteries[entrynumber].inode = entry->inode;
                        strcpy(enteries[entrynumber].name,filename.c_str());
                        enteries[entrynumber].name_len = entry->name_len;
                        enteries[entrynumber].rec_len = entry->rec_len;
                    }else{
                        entry = (ext2_dir_entry_2*) &block[i];
                        i+=entry->rec_len;
                    }
                    
                    // inode memeber iblocks stores the # of blocks used to store the file but it is 512 byte blocks not blocksize
                }
            }
        }

        return enteries;
    }


    void display_info(ext2_inode inode){
        
        if (S_ISDIR(inode.i_mode)){ // inode is a directoy imitate ls -l output 
            vector<ext2_dir_entry_2> entries =
            get_directory_enteries(inode);
            cout << left ; // left justify output 
            cout << setw(15) << "Permissions" << setw(10) << "USER ID"<< setw(10) << "Size" << setw(20) << "Access Time" << setw(15) << "Name" << endl;
            cout << setfill('-') << setw(70) << '-' << setfill(' ') << endl ; 
            for (int i=0; i < entries.size();i++){
                cout << setw(15) <<  get_permission(get_inode(entries[i].inode)) ;                
                cout << setw(10) << get_inode(entries[i].inode).i_uid ;
                cout << setw(10) << get_inode(entries[i].inode).i_size;
                cout << setw(20) << get_time_of_access(get_inode(entries[i].inode));
                cout << setw(15) << entries[i].name << endl;
            }

        }
        if(S_ISREG(inode.i_mode)){ // regular file 
            dump_inode_data(inode);
        }
        // more macros are difined S_ISBLK() , S_ISLNK() 
    }

   

    void proccess_path(string& path){  
        
        ext2_inode inode2 = get_inode(2);
        ext2_inode endOfPath;
        endOfPath = inode2; // for loop below will not run if only root is provided then we want to proccess root inode

        stringstream *ss = new stringstream(path);
        string temp;
        
        
        
        while (getline(*ss,temp,'/')) filenames.push_back(temp); // parce path and store into filenames
        ss->str(string()); // clear the string stream it caused a segfualt
        delete ss;

        entries =
            get_directory_enteries(inode2); 

        static int i,j;
        bool founddir=true; // didn't actually find directory yet but loop needs to start
        for (j = 1; j < filenames.size() ; j++){
            string filename; 
            founddir = false; // now looking for the next directory 
            for(i=0; i < entries.size(); i++ ){   // iterate of all entries 
                filename = entries[i].name;     // modify so it is a proper string
                if (filenames[j] == filename){     // did we find the file?
                    if(j == filenames.size()-1){ // the whole path has been parced
                        endOfPath = get_inode(entries[i].inode);
                        founddir=true;
                        break;
                    }
                    ext2_inode next_inode ;
                    next_inode = get_inode(entries[i].inode);
                    entries = get_directory_enteries(next_inode);  // get directory entries at the inode specified by the entry with propername
                    founddir=true;
                } 
            }
            if(!founddir){
                cout << "PATH NOT FOUND" << endl; 
                break;
            }
        }

        if (founddir){
            display_info(endOfPath);
        }

    
    }

    void close_filesystem(){
        ext2.close();
    }




    // int inode_size;
// int inode_size;                     // all of these variable were falling out of scope
//ext2_super_block superblock;       // in my private member functions causeing
//vector<ext2_group_desc> GroupDec; // segfaults and stack buffer overflows



using namespace std;
int main(int argc, char *argv[]){
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

   

    open_filesystem(ext2_name);
    if(get_superblock() == -1){return -1 ;}// didnt get correct magic number
    get_group_decs();
    proccess_path(path);

    close_filesystem();

    return 0;
} 