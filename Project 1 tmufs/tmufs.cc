#include<iostream>
#include <cstring>
#include <fstream>
#include <iomanip>

using namespace std;

#define TMUFS_ATTRIBUTE_ASCII    0x01
#define TMUFS_ATTRIBUTE_BINARY   0x02
#define TMUFS_ATTRIBUTE_HIDDEN   0x04
#define TMUFS_ATTRIBUTE_DELETED  0x08

struct bin_format_t {
    int id;
    char name[16];
    float weight;
    float height;
};

struct tmufs_header_t {
    unsigned short magic_number; // magic number (0x1234) tells us that we have indeed a TMUFS
    unsigned long number_dentries; // total number of directory entries in the TMUFS
    unsigned long data_area_start; // byte offset at which the data area of the TMUFS starts
    unsigned long data_checksum; // contains a checksum over the whole data -> error checking
};
const int TMUFS_FN_LENGTH = 10;
struct  tmufs_dentry_t{
    unsigned long file_size; // file size in bytes
    unsigned char file_attributes; // attribute characterize the file (see above)
    unsigned long data_start; // byte offset at which the file contents start
    char filename[TMUFS_FN_LENGTH];// name of the file (max 9 characters (+null byte))
};

void skip_zeros(ifstream& fin){
    char input;    
    fin.get(input);
    while(input==0){
        fin.get(input);
    }
    fin.seekg(fin.tellg()-1); // just used loop and moved past zero by 1
    cout<< fin.tellg() << endl;
}
void print_divider(){
    cout << "-----------------------------------------------------------------------------" <<endl; 
}


int main(int argc, char *argv[]){
    
bin_format_t bin;
strcpy (bin.name,argv[1]);  //store the file name in bin.name
ifstream fin;

fin.open(bin.name, ios::in | ios::binary);
if(!fin.is_open()){
    cout<<"Sorry file could not be found(check spelling)."<<endl;
}

tmufs_header_t header;
char input='a';
int int_input;



fin.read((char*)& header.magic_number,sizeof(header.magic_number));
skip_zeros(fin);
fin.read((char*)& header.number_dentries,sizeof(header.number_dentries));
skip_zeros(fin);
fin.read((char*)& header.data_area_start,sizeof(header.data_area_start));
skip_zeros(fin);
fin.read((char*)& header.data_checksum,sizeof(header.data_checksum));

//cout<< "tell g is :"<< fin.tellg();
///////// used to check accuracy of the header read
/*
cout << hex <<"magic number is : " << header.magic_number << endl;
cout << dec << "directory entries:" << header.number_dentries <<endl << endl;
cout << dec << "data start:" <<header.data_area_start <<endl << endl;
cout << dec << "check sum: " << header.data_checksum <<endl << endl;
*/

if(header.magic_number != 0x1234){
    cout << "Error TUMFS not found" << endl;
    return(-1);
}

fin.seekg(32); // move past header

tmufs_dentry_t entries[header.number_dentries]; // make an array of entries 

for (int i=0;i < header.number_dentries;i++){
    skip_zeros(fin);
    fin.read((char*)& entries[i].file_size,sizeof(entries[i].file_size));
    skip_zeros(fin);
    fin.read((char*)& entries[i].file_attributes,sizeof(entries[i].file_attributes));
    skip_zeros(fin);
    fin.read((char*)& entries[i].data_start,sizeof(entries[i].data_start));
    skip_zeros(fin);
    fin.read((char*)& entries[i].filename,sizeof(entries[i].filename));
    skip_zeros(fin);
}


print_divider();
cout<< "TMUFS HEADER" << endl;
print_divider();

cout << "Magic Number: " << hex << header.magic_number << endl 
<< "Number of Directory Entries: " << dec << header.number_dentries << endl
<<"Byte Offset of Data Area in" << bin.name << " : " << header.data_area_start <<endl
<< "Checksum of the Dara Area:" <<  header.data_checksum <<endl;

print_divider();

// i am sure there is some way to print this so everything is nice and lined up but it didnt seem that important so i didnt.

cout<< "File number       File Name       Size      Start Offset    Attributes" << endl << endl ;
for (int i = 0 ; i< header.number_dentries; i++){
    cout << " " << i+1 << "               " << entries[i].filename <<"         " <<entries[i].file_size  << "          " << entries[i].data_start << "          ";
    if (entries[i].file_attributes & TMUFS_ATTRIBUTE_BINARY){
        cout << "BINARY";
    }
    if(entries[i].file_attributes & TMUFS_ATTRIBUTE_ASCII){
        cout << "ASCII"; 
    }
    if(entries[i].file_attributes & TMUFS_ATTRIBUTE_DELETED){
        cout << "DELETED";
    }
    if(entries[i].file_attributes & TMUFS_ATTRIBUTE_HIDDEN){
        cout << "HIDDEN";
    }
    cout<<endl<<endl;
}

print_divider();

cout << endl <<endl ;

for(int i=0; i < header.number_dentries ; i++){
    cout<< "Contents of file : " << entries[i].filename << endl;
    print_divider();

    fin.seekg(entries[i].data_start); // go to where the data of the file starts 
    
    if (entries[i].file_attributes & TMUFS_ATTRIBUTE_ASCII ){
        for(int j=0; j < entries[i].file_size; j++){
            cout << (char)fin.get();
        }
    }

    if (entries[i].file_attributes & TMUFS_ATTRIBUTE_BINARY){
        cout << "[HEXDUMP]" << endl << endl;
        for(int j=0; j < entries[i].file_size; j++){
            cout << hex << " " << fin.get();
        }
    }
    
    cout <<endl;

    print_divider();

    cout << endl << endl; 
}

 // used to test that the detnties were being loaded correctly 
/*
for (int i=0;i < header.number_dentries;i++){
    cout<<"entrie # " << i << " file size: " << entries[i].file_size << endl; 
    cout<<"entrie # " << i << " file attributes: " << hex << entries[i].file_attributes << endl; 
    cout<<"entrie # " << i << " data start: " << dec <<  entries[i].data_start << endl; 
    cout<<"entrie # " << i << " filename: " << entries[i].filename << endl; 
    cout <<endl << endl;
}
*/



}