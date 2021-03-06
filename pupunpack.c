// Copyright 2012       Grazioli Giovanni <wargio@libero.it>
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "tools.h"
#include "types.h"
#include "little_endian.h"

#ifdef WIN32
#define MKDIR(x,y) mkdir(x)
#else
#define MKDIR(x,y) mkdir(x,y)
#endif

static u8 *pup = NULL;
static u32 file_count;

const  u64 PSV_HDR = 0x5343455546000001ull;

static int  read_only = 0;
static int  unknown_type = 0;
static char unknown_name[256];

static struct id2name_tbl t_names[] = {
	{0x100, "version.txt"},
	{0x101, "license.xml"},
	{0x200, "psp2swu.self"},
	{0x204, "psp2swu.sprx"},
	{0x221, "psv_package_data01.pkg"},
	{0x231, "psv_package_data02.pkg"},
	{0x301, "package_data01.pkg"},
	{0x302, "package_data02.pkg"},
	{0x303, "package_data03.pkg"},
	{0x304, "package_data04.pkg"},
	{0x305, "package_data05.pkg"},
	{0x306, "package_data06.pkg"},
	{0x307, "package_data07.pkg"},
	{0x308, "package_data08.pkg"},
	{0x309, "package_data09.pkg"},
	{0x30a, "package_data10.pkg"},
	{0x30b, "package_data11.pkg"},
	{0x30c, "package_data12.pkg"},
	{0x30d, "package_data13.pkg"},
	{0x30e, "package_data14.pkg"},
	{0x30f, "package_data15.pkg"},
	{0x310, "package_data16.pkg"},
	{0x311, "package_data17.pkg"},
	{0x312, "package_data18.pkg"},
	{0x313, "package_data19.pkg"},
	{0x314, "package_data20.pkg"},
	{0x315, "package_data21.pkg"},
	{0x316, "package_data22.pkg"},
	{0x317, "package_data23.pkg"},
	{0x318, "package_data24.pkg"},
	{0x319, "package_data25.pkg"},
	{0x31a, "package_data26.pkg"},
	{0x326, "debug_data00.pkg"},
	{0x327, "debug_data01.pkg"},
	{0x328, "debug_data02.pkg"},
	{0x329, "debug_data03.pkg"},
	{0x32a, "debug_data04.pkg"},
	{0x32b, "debug_data05.pkg"},
	{0x32c, "debug_data06.pkg"},
	{0x400, "package_scewm.wm"},
	{0x401, "package_sceas.as"},
	{0, NULL}
};

void Write(const char *file, u32 offset, u32 size){
	FILE *fp;
	fp = fopen(file, "wb");
	u32 i;
	for(i=0;i<size;i++){
		fseek(fp,i, SEEK_SET);
		fwrite(pup+offset+i, 1, 1, fp);
	}
	fclose(fp);
}

void Unpup(const char* folder){
	u32 i;
	char folder_2[200];
	if(read_only!=1){
		DIR  *dip;
		i=0;
		do{
			sprintf(folder_2,"%s_%u",folder,i++);
		}while((dip = opendir(folder_2)) != NULL);
		closedir(dip);
		MKDIR(folder_2, 0777);
		chdir(folder_2);
	}

	const char *file_name;
	if(read_only!=0)
		printf("Read Only Mode!\n");
	printf("Reading...\n");
	u64 HDR = be64(pup);

	if(HDR!=PSV_HDR)
		fail("\nERROR! Not a PlayStation Vita Update File (%08x%08x)",HDR>>32,HDR);

	u32 pkg_version = le32(pup+0x08);
	u32 img_version = le32(pup+0x10);
	file_count = le32(pup+0x18);
	u32 hdr_lenght = le32(pup+0x20);
	u32 pkg_lenght = le32(pup+0x28);

	dmsg("HDR          0x%08x%08x\n",HDR>>32,HDR);
	dmsg("PKG  VERSION 0x%08x\n",pkg_version);
	dmsg("IMG  VERSION 0x%08x\n",img_version);
      printf("N of Files   %u\n",file_count);
	dmsg("HDR   Lenght 0x%08x\n",hdr_lenght);
	dmsg("PKG   Lenght 0x%08x\n",pkg_lenght);
	dmsg("Table Lenght 0x%08x\n",0x80+(0x20*file_count));
	u32 entry,offset,size;
	for(i=0;i<file_count;i+=0x1){
		entry  = le32(pup+0x80+0x20*i);
		offset = le32(pup+0x80+0x20*i+0x08);
		size   = le32(pup+0x80+0x20*i+0x10);

		file_name = id2name(entry, t_names, NULL);
		if(file_name==NULL){
			dmsg("unknown entry id: 0x%08x | Offset: 0x%08x\n",entry,offset);
			snprintf(unknown_name,256,"unknown_data_0x%x_%02d.data",entry,unknown_type++);
			file_name = unknown_name;
		}
		printf("Found: %30s | size: %10u Bytes\n",file_name,size);
		if(read_only!=1)
			memcpy_to_file(file_name, pup + offset, size);
	}
	if(read_only!=1){
		dmsg("Writing security_1..");
		Write("security_1",0x30,0x50);
	}
	printf("Done!\n");
}

int main(int argc, char *argv[]){

	if (argc == 2) {
		pup = mmap_file(argv[1]);
		printf( "PUP Unpacker\n");
		Unpup(argv[1]);

	} else if(argc == 3) {
		if (strcmp(argv[1], "-d") != 0)
			if(strcmp(argv[1], "-r") !=0)
				if(strcmp(argv[1], "-rd") !=0)
					fail("invalid option: %s", argv[1]);
		
		if(strcmp(argv[1], "-d")==0)
			set_dbg_flag();

		if(strcmp(argv[1], "-r")==0)
			read_only=1;

		if(strcmp(argv[1], "-rd")==0){
			read_only=1;
			set_dbg_flag();
		}

		printf( "PUP Unpacker\n");
		pup = mmap_file(argv[2]);
		dmsg("PUP File: %s\n",argv[2]);
		Unpup(argv[2]);

	}else {
		fail("usage: %s PSP2UPDAT.PUP\n"
			"	-d	| debug messages\n"
			"	-r	| read only mode\n"
			"	-rd	| read only + debug messages"
			,argv[0]);
	}


	return 0;
}
