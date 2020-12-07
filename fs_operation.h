/*
 * @Author: Deng Cai 
 * @Date: 2019-09-09 16:09:14
 * @Last Modified by: Deng Cai
 * @Last Modified time: 2019-12-10 19:16:58
 */

//  in this code : \
    for the file system's space is so small, we don't need \
    group descriptors acctually, so i just put \
    some critical information in other structures and cut group \
    descriptors. what's more, the inode map and block map are \
    put into super block.
//  in some degrees, this file system can be seemed as a system \
    with only one block group, so we need just one super block \
    and one group descriptor. then why don't we put them together \
    and make them one single structure as "super_block"? that's \
    how i handle with it.

#ifndef _FS_OPERATION_H_
#define _FS_OPERATION_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define _FILE_ 0
#define _FOLDER_ 1

#define DEVICE_SIZE (1<<22)
#define BLOCK_SIZE 1024
#define MAX_FILE_BLOCK_NUM 6
#define MAX_INODE_NUM 1024

typedef struct inode {
    // 32 bytes;
    uint32_t size;
    // for folder, it's the number of blocks it have;\
        for file, it's the real size.
    uint16_t file_type;
    // 1->dir; 0->file;
    uint16_t link;  // it doesn't matter if you \
        don't know what this variable means.
    uint32_t block_point[MAX_FILE_BLOCK_NUM];
    // the blocks belonging to this inode.
} inode;

typedef struct super_block {
    // 656 bytes;
    int32_t system_mod;
    // use system_mod to check if it \
        is the first time to run the FS.
    int32_t free_block_count;
    // 2^12; 4096;
    int32_t free_inode_count;
    // 1024;
    int32_t dir_inode_count;
    uint32_t block_map[DEVICE_SIZE/BLOCK_SIZE/sizeof(uint32_t)/8]; // 128;
    // 512 bytes;
    uint32_t inode_map[MAX_INODE_NUM/sizeof(uint32_t)/8]; // 32;
    // 128 bytes;
} sp_block;
// 1 block;

typedef struct dir_item {
    // the content of folders.
    // 128 bytes;
    uint32_t inode_id;
    uint16_t item_count;
    // 1 means the last one;
    // it doesn't matter if you don't understand it.
    uint8_t type;
    // 1 represents dir;
    char name[121];
} dir_item;


FILE *fp;
sp_block *spBlock;
inode inode_table[MAX_INODE_NUM];
dir_item block_buffer[BLOCK_SIZE/sizeof(dir_item)]; // 8;
// 32 blocks;
int global_inode;
char global_name[512];


void print_information(int mode);
// do some pre-work when you run the FS.
void fs_init(sp_block *spBlock);
void ls(char *path);
void create_file(char *path,int size);
void create_dir(char *path);
void delete_file(char *path);
void delete_dir(char *path);
void move(char *from,char *to);
void cd(char *path);
void copy(char *ori,char *dest);

#endif