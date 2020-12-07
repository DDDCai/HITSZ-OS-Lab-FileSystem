/*
 * @Author: Deng Cai 
 * @Date: 2019-09-09 15:58:51
 * @Last Modified by: Deng Cai
 * @Last Modified time: 2020-12-05 16:56:45
 */
#include "fs_operation.h"

// mode 1: be uesd when opening system;
// mode 0: print some key information when running.
void print_information(int mode)
{
    if(mode){
    printf("---------------------------------------------------------------------\n");
    printf("-------------------------------WELCOME!------------------------------\n");
    printf("---------------------------------------------------------------------\n");
    printf("In this FileSystem :\n");
    printf("Supports: \\cd, \\ls, \\create, \\delete, \\move and \\information.\n");
    printf("The maximum size of a single file is 6KB ;\n");
    printf("The maximum number of files and folders a single\
 folder can contain is 46 ;\n");
    printf("The whole file system can contain mostly 1024\
 files and folders ;\n");
    }
    printf("**It has %d folders and %d files in this system now ;\n",\
    spBlock->dir_inode_count,1024-(spBlock->dir_inode_count)\
    -(spBlock->free_inode_count));
    printf("**It has %dKB free space now ;\n",spBlock->free_block_count);
    printf("**And it can accept another %d new files or folders .\n",\
    spBlock->free_inode_count);
    if(mode){
    printf("---------------------------------------------------------------------\n");
    printf("!!!!!!! **The instruction should be shorter than 512 bytes** !!!!!!!!\n");
    printf("---------------------------------------------------------------------\n");
    }
}

static int check_text(char *text)
{
    int i = 0;
    char a = text[0];
    while(a!='\0')
    {
        if((a>='a'&&a<='z')||(a>='A'&&a<='Z')||(a>='0' && a<='9')\
        ||a=='_'||a=='.')
            a = text[++i];
        else 
        {
            printf("Illegal name (Only numbers ,letters ,_ and . can be accepted)\n");
            return 1;
        }
    }
    return 0;
}

// return the last folder's inode according to the path, 
// and store the file's name in tmp.
static int go_through_the_file_path(char *path,char *tmp)
{   
    int i_id,i,j=0;
    if(path[0]=='/')
    {
        i = 1;
        i_id = 0;
    }
    else 
    {
        i = 0;
        i_id = global_inode;
    }

    while(1)
    {
        if(path[i]!='/')
        {
            tmp[j++] = path[i];
        }
        if(path[i]=='\0')
        {
            break;
        }
        if(path[i]=='/')
        {
            tmp[j] = '\0';
            j = 0;
            int success;
            for(int k=0;k<inode_table[i_id].size;k++)
            {
                fseek(fp,BLOCK_SIZE*inode_table[i_id].block_point[k],SEEK_SET);
                fread(block_buffer,BLOCK_SIZE,1,fp);
                for(int l=0;l<8;l++)
                {
                    if(block_buffer[l].type && !strcmp(tmp,block_buffer[l].name))
                    {
                        k = 1024;
                        i_id = block_buffer[l].inode_id;
                        success = 1;
                        break;
                    }
                    if(block_buffer[l].item_count)
                    {
                        success = 0;
                        break;
                    }
                }
            }
            if(!success)
            {
                printf("Don't have dir \"%s\"\n",tmp);
                return -1;
            }
        }
        i++;
    }
    return i_id;
}

// return the the second-to-last folder's inode \
// according to this path, and store the last \
// folder's name in tmp. \
//  mode: to decide if keep the last folder's name when \
//      meeting "//" (acctually keep it when executing "cd"). \
//      return '\0' tmp when mode == 1. (i.e. ignore '\\' \
//      when mode is NOT set). \
            when mode == 0, the end of path must be '\\' and \
            it must be called from instruction cd.
static int go_through_the_dir_path(char *path,char *tmp)
{
    int i_id,i,j=0;
    if(path[0]=='/')
    {
        i_id = 0;
        i = 1;
    }
    else
    {
        i_id = global_inode;
        i = 0;
    }
    
    while(1)
    {
        if(path[i]!='/')
        {
            tmp[j++] = path[i];
        }
        if(path[i]=='\0')
            break;
        if(path[i]=='/' && path[i+1]!='\0')
        {
            tmp[j] = '\0';
            j = 0;
            int success;
            for(int k=0;k<inode_table[i_id].size;k++)
            {
                fseek(fp,BLOCK_SIZE*inode_table[i_id].block_point[k],SEEK_SET);
                fread(block_buffer,BLOCK_SIZE,1,fp);
                for(int l=0;l<8;l++)
                {
                    if(block_buffer[l].type==_FOLDER_ && !strcmp(tmp,block_buffer[l].name))
                    {
                        k = inode_table[i_id].size;
                        i_id = block_buffer[l].inode_id;
                        success = 1;
                        break;
                    }
                    if(block_buffer[l].item_count)
                    {
                        success = 0;
                        break;
                    }
                }
            }
            if(!success)
            {
                printf("Don't have dir \"%s\"\n",tmp);
                return -1;
            }
        }
        if(path[i]=='/' && path[i+1]=='\0')
        {
            tmp[j] = '\0';
            break;
        }
        i++;
    }
    return i_id;
}

static int go_through_the_dir_path_to_end(char *path,char *tmp)
{
    int i_id,i,j,w=0;
    if(path[0]=='/')
    {
        i_id = 0;
        i = 1;
        strcpy(tmp,"/");
        j = 1;
    }
    else
    {
        i_id = global_inode;
        i = 0;
        strcpy(tmp,global_name);
        strcat(tmp,"/");
        j = strlen(tmp);
    }
    char p[121];
    
    while(1)
    {
        if(path[i]!='/')
        {
            tmp[j++] = path[i];
            p[w++] = path[i];
        }
        if(path[i]=='\0')
            break;
        if(path[i]=='/' && path[i+1]!='\0')
        {
            tmp[j++] = '/';
            p[w] = '\0';
            w = 0;
            int success;
            for(int k=0;k<inode_table[i_id].size;k++)
            {
                fseek(fp,BLOCK_SIZE*inode_table[i_id].block_point[k],SEEK_SET);
                fread(block_buffer,BLOCK_SIZE,1,fp);
                for(int l=0;l<8;l++)
                {
                    if(block_buffer[l].type==_FOLDER_ && !strcmp(p,block_buffer[l].name))
                    {
                        k = inode_table[i_id].size;
                        i_id = block_buffer[l].inode_id;
                        success = 1;
                        break;
                    }
                    if(block_buffer[l].item_count)
                    {
                        success = 0;
                        break;
                    }
                }
            }
            if(!success)
            {
                printf("Don't have dir \"%s\"\n",path);
                return -1;
            }
            else
            {
                j -= 2;
                if(tmp[j]=='.')
                {
                    j --;
                    if(tmp[j]=='.')
                    {
                        j -= 2;
                        while(tmp[j]!='/')
                            j --;
                    }
                    j ++;
                }
                else 
                    j += 2;
            }
        }
        if(path[i]=='/' && path[i+1]=='\0')
        {
            tmp[j-1] = '\0';
            break;
        }
        i++;
    }
    return i_id;
}

// mode 0: return a free inode;
// mode 1: return block_num free blocks with num.
// mode 2: both mode 0 and 1.
static int get_inode_and_block(int mode,int block_num,int *num)
{
    if(mode!=1)
        if(!spBlock->free_inode_count)
        {
            printf("There is no free inode \n");
            return -1;
        }
    if(mode!=0)
        if(spBlock->free_block_count<block_num)
        {
            printf("There are not enough blocks \n");
            return -1;
        }

    int i;
    if(mode!=0)
    {
        for(i=0;i<128;i++)
        {
            if(spBlock->block_map[i]==(~0))
                continue;
            uint32_t mask = 0x80000000;
            for(int j=0;j<32;j++)
            {
                if((mask & spBlock->block_map[i])) ;
                else 
                {
                    spBlock->block_map[i] |= mask;
                    spBlock->free_block_count--;
                    (*num) = ((i<<5)+j);
                    if(--block_num)
                        num++;
                    else 
                    {
                        if(mode==1)
                            return 1;
                        else 
                        {
                            i = 128;
                            break;
                        }
                    }
                }
                mask >>= 1;
            }
        }
    }

    if(mode!=1)
    {
        for(i=0;i<32;i++)
        {
            if(spBlock->inode_map[i]==(~0))
                continue;
            uint32_t mask = 0x80000000;
            for(int j=0;j<32;j++)
            {
                if((mask & spBlock->inode_map[i]))
                    mask >>= 1;
                else 
                {
                    spBlock->inode_map[i] |= mask;
                    spBlock->free_inode_count--;
                    return ((i<<5)+j);
                }
            }
        }
    }
    return -1;
}

//  i: the last item's position in his block;
//  i_id: the folder pointed now;
//  new_inode: the new dir_item's inode;
//  tmp: the new dir_item's name;
//  type: the new dir_item's file type.
static void create_a_new_dir_item(int i,int i_id,int new_inode,char *tmp,int type)
{
    if(i!=7)
    {
        // the last block is not full.
        block_buffer[i].item_count = 0;
        i++;
        fseek(fp,-BLOCK_SIZE,SEEK_CUR);
    }
    else 
    {
        //the last block is full, so it has to get a new free block.
        if(inode_table[i_id].size==6)
        {
            printf("The number of files and dirs in this dir has met the maxmuim\n");
            return ;
        }
        int a;
        if(get_inode_and_block(1,1,&a)<0)
            return ;
        inode_table[i_id].size++;
        inode_table[i_id].block_point[inode_table[i_id].size-1] = a;
        block_buffer[7].item_count = 0;
        fseek(fp,-BLOCK_SIZE,SEEK_CUR);
        fwrite(block_buffer,BLOCK_SIZE,1,fp);
        memset(block_buffer,0,BLOCK_SIZE);
        i = 0;
        fseek(fp,BLOCK_SIZE*a,SEEK_SET);
    }
    block_buffer[i].inode_id = new_inode;
    block_buffer[i].type = type;
    block_buffer[i].item_count = 1;
    strcpy(block_buffer[i].name,tmp);
    fwrite(block_buffer,BLOCK_SIZE,1,fp);
}

static void free_a_block(int block_id)
{
    uint32_t mask = block_id & 0x1f;
    spBlock->block_map[block_id>>5] ^= mask;
    spBlock->free_block_count++;
}

static void delete_file_inode(int i_id)
{
    int i;
    uint32_t mask;
    int block_num = inode_table[i_id].size/BLOCK_SIZE;
    if((inode_table[i_id].size-block_num*BLOCK_SIZE)!=0)
        block_num++;
    for(i=0;i<block_num;i++)
    {
        free_a_block(inode_table[i_id].block_point[i]);
    }

    mask = 0x80000000 >> (i_id&0x1f);
    spBlock->inode_map[i_id>>5] ^= mask;
    spBlock->free_inode_count++;
}

//  it does be a complex function which makes me crazy;
//  i_id: the inode that this dir_item belonging to;
//  i: the order number of the block this dir_item belonging to;
//  j: the order number of the dir_item in his block.
static void delete_a_dir_item(int i_id,int i,int j)
{
    if(block_buffer[j].item_count)
    {
        // if it is the last item.
        if(j!=0)
        {
            // and it is not the only one in this block.
            block_buffer[j-1].item_count = 1;
        }
        else 
        {
            // it is the only item in this block.
            free_a_block(inode_table[i_id].block_point[i]);
            inode_table[i_id].size--;
            fseek(fp,BLOCK_SIZE*inode_table[i_id].block_point[inode_table[i_id].size-1],SEEK_SET);
            fread(block_buffer,BLOCK_SIZE,1,fp);
            block_buffer[7].item_count = 1;
        }
    }
    else 
    {
        // it is not the last item.
        if(i == inode_table[i_id].size-1)
        {
            // if it is in the last block, just move the last item \
            // to its place.
            int k = j+1;
            while(!block_buffer[k].item_count)
                k++;
            block_buffer[k].item_count = 0;
            block_buffer[j] = block_buffer[k];
            block_buffer[k-1].item_count = 1;
        }
        else 
        {
            // if it is not in the last block.
            dir_item dir_tmp;
            fseek(fp,BLOCK_SIZE*inode_table[i_id].block_point[inode_table[i_id].size-1],SEEK_SET);
            fread(block_buffer,BLOCK_SIZE,1,fp);
            if(block_buffer[0].item_count)
            {
                // if the last block has only one item, we need to delete this block after \
                // we move the last item to the place of deleted item.
                dir_tmp = block_buffer[0];
                dir_tmp.item_count = 0;
                free_a_block(inode_table[i_id].block_point[inode_table[i_id].size-1]);
                inode_table[i_id].size--;
                fseek(fp,BLOCK_SIZE*inode_table[i_id].block_point[i],SEEK_SET);
                fread(block_buffer,BLOCK_SIZE,1,fp);
                block_buffer[j] = dir_tmp;
                block_buffer[7].item_count = 1;
                int point_tmp = inode_table[i_id].block_point[i];
                inode_table[i_id].block_point[i] = inode_table[i_id].block_point[inode_table[i_id].size-1];
                inode_table[i_id].block_point[inode_table[i_id].size-1] = point_tmp;
            }
            else 
            {
                // similar to the previous case, but it doesn't delete the block.
                int k=1;
                while(!block_buffer[k].item_count)
                    k++;
                dir_tmp = block_buffer[k];
                dir_tmp.item_count = 0;
                block_buffer[k-1].item_count = 1;
                fseek(fp,-BLOCK_SIZE,SEEK_CUR);
                fwrite(block_buffer,BLOCK_SIZE,1,fp);
                fseek(fp,BLOCK_SIZE*inode_table[i_id].block_point[i],SEEK_SET);
                fread(block_buffer,BLOCK_SIZE,1,fp);
                block_buffer[j] = dir_tmp;
            }
        }
    }
    fseek(fp,-BLOCK_SIZE,SEEK_CUR);
    fwrite(block_buffer,BLOCK_SIZE,1,fp);
}

// it is a recursive function because delete a folder \
// means deleting all folders and files in it.
//  del_id: the deleting folder's inode;
//  back_pos: we need to restore the block_buffer by it \
//      after deleting this folder.
static void del_sub_dir(int del_id,int back_pos)
{
    int i,j;
    for(i=0;i<inode_table[del_id].size;i++)
    {
        fseek(fp,BLOCK_SIZE*inode_table[del_id].block_point[i],SEEK_SET);
        fread(block_buffer,BLOCK_SIZE,1,fp);
        for(j=0;j<8;j++)
        {
            if((!strcmp(block_buffer[j].name,"."))||(!strcmp(block_buffer[j].name,".."))) ;
            else if(block_buffer[j].type)
                del_sub_dir(block_buffer[j].inode_id,BLOCK_SIZE*inode_table[del_id].block_point[i]);
            else 
                delete_file_inode(block_buffer[j].inode_id);
            if(block_buffer[j].item_count)
            {
                i = 100;
                break;
            }
        }
    }
    for(i=0;i<inode_table[del_id].size;i++)
    {
        free_a_block(inode_table[del_id].block_point[i]);
    }
    uint32_t mask = 0x80000000 >> (del_id&0x1f);
    spBlock->inode_map[del_id>>5] ^= mask;
    spBlock->free_inode_count++;
    spBlock->dir_inode_count--;
    fseek(fp,back_pos,SEEK_SET);
    fread(block_buffer,BLOCK_SIZE,1,fp);
}

// initialize the system when opening it.
void fs_init(sp_block *spBlock)
{
    if(spBlock->system_mod==0x7ba4190e)
    {
        fseek(fp,BLOCK_SIZE,SEEK_SET);
        fread(inode_table,sizeof(inode),1024,fp);
    }
    else 
    {
        spBlock->system_mod = 0x7ba4190e;
        spBlock->free_block_count = 4096 - 34;
        // 1 for super block, 32 for inode table, 1 for folder "root".
        spBlock->free_inode_count = 1023;
        spBlock->dir_inode_count = 1;
        memset(spBlock->inode_map,0,128);
        memset(spBlock->block_map,0,512);
        spBlock->inode_map[0] = (0x80000000);
        spBlock->block_map[0] = ~0;
        spBlock->block_map[1] = (0xc0000000);

        inode_table[0].file_type = _FOLDER_;
        inode_table[0].size = 1;
        inode_table[0].link = 0;
        inode_table[0].block_point[0] = 33;

        block_buffer[0].inode_id = 0;
        block_buffer[0].item_count = 1;
        block_buffer[0].type = 1;
        strcpy(block_buffer[0].name,".");
        fseek(fp,BLOCK_SIZE*33,SEEK_SET);
        fwrite(block_buffer,BLOCK_SIZE,1,fp);
    }
    global_inode = 0;
    print_information(1);
    return ;
}

// to check if there is a duplicate name of tmp in i_id \
// folder, and type represents file or folder.
// if exists, return its -inode_id.(<0)
// else, return the last dir_item's position in its block.(>=0)
static int deduplicate_name(char *tmp,int i_id,int type)
{
    int i,j;
    for(j=0;j<inode_table[i_id].size;j++)
    {
        fseek(fp,BLOCK_SIZE*\
            inode_table[i_id].block_point[j],SEEK_SET);
        fread(block_buffer,BLOCK_SIZE,1,fp);
        for(i=0;i<8;i++)
        {
            if(block_buffer[i].type==type)
                if(!strcmp(block_buffer[i].name,tmp))
                    return -block_buffer[i].inode_id;
            if(block_buffer[i].item_count)
                break;
        }
    }
    return i;
}

void ls(char *path)
{
    int i_id,i,j=0;
    if(path[0]=='/')
    {
        i_id = 0;
        i = 1;
    }
    else 
    {
        i_id = global_inode;
        i = 0;
    }
    char tmp[121];

    while(path[i]!='\0')
    {
        if(path[i]!='/')
        {
            tmp[j++] = path[i];
        }
        if(path[i]=='/' || path[i+1]=='\0')
        {
            tmp[j] = '\0';
            j = 0;
            int success;
            for(int k=0;k<inode_table[i_id].size;k++)
            {
                fseek(fp,BLOCK_SIZE*inode_table[i_id].block_point[k],SEEK_SET);
                fread(block_buffer,BLOCK_SIZE,1,fp);
                for(int l=0;l<8;l++)
                {
                    if(block_buffer[l].type && !strcmp(tmp,block_buffer[l].name))
                    {
                        k = 1024;
                        i_id = block_buffer[l].inode_id;
                        success = 1;
                        break;
                    }
                    if(block_buffer[l].item_count)
                    {
                        success = 0;
                        break;
                    }
                }
            }
            if(!success)
            {
                printf("Don't have folder \"%s\"\n",tmp);
                return ;
            }
        }
        i++;
    }

    int k;
    fseek(fp,BLOCK_SIZE*inode_table[i_id].block_point[0],SEEK_SET);
    fread(block_buffer,BLOCK_SIZE,1,fp);
    if(i_id)
        k = 1;
    else 
        k = 0;
    if(!block_buffer[k].item_count)
    {
        for(j=k+1;j<8;j++)
        {
            if(block_buffer[j].type)
                printf("*");
            printf("%s\t",block_buffer[j].name);
            if(block_buffer[j].item_count)
                break;
        }
        printf("\n");
    }
    //  avoid printing "." and "..".
    for(k=1;k<inode_table[i_id].size;k++)
    {
        fseek(fp,BLOCK_SIZE*inode_table[i_id].block_point[k],SEEK_SET);
        fread(block_buffer,BLOCK_SIZE,1,fp);
        for(j=0;j<8;j++)
        {
            if(block_buffer[j].type)
                printf("*");
            printf("%s\t",block_buffer[j].name);
            if(block_buffer[j].item_count)
            {
                k = 1024;
                break;
            }
        }
        printf("\n");
    }

    return ;
}

void create_file(char *path,int size)
{
    char tmp[121];
    int i_id = go_through_the_file_path(path,tmp);
    if(i_id<0)
        return ;
    int i,j;

    if(tmp[0]=='\0')
    {
        printf("Wrong path \n");
        return ;
    }
    if(check_text(tmp))
        return ;

    i = deduplicate_name(tmp,i_id,_FILE_);   // this i is useful later ;
    if(i<0)
    {
        printf("There is already a file named \"%s\" in this path !\n",tmp);
        return ;
    }
    
    int block_num = size/BLOCK_SIZE;
    if(size%BLOCK_SIZE)
        block_num++;
    if(block_num>6)
    {
        printf("The file is too big \n");
        return ;
    }
    int num[block_num];
    int new_inode = get_inode_and_block(2,block_num,num);
    if(new_inode<0)
        return ;
    inode_table[new_inode].file_type = _FILE_;
    inode_table[new_inode].size = size;
    inode_table[new_inode].link++;
    for(j=0;j<block_num;j++)
        inode_table[new_inode].block_point[j] = num[j];
  
    create_a_new_dir_item(i,i_id,new_inode,tmp,_FILE_);
}

void create_dir(char *path)
{
    char tmp[121];
    int i,j;
    int i_id = go_through_the_dir_path(path,tmp);
    if(i_id<0 || tmp[0]=='\0')
    // tmp[0] may equal to '\0' if the path is being like "/home/..../dc//".
        return ;

    if(check_text(tmp))
        return ;

    i = deduplicate_name(tmp,i_id,1);  // this i is useful later ;
    if(i<0)
    {
        printf("There is already a folder named \"%s\" in this path !\n",tmp);
        return ;
    }

    int b;
    int new_inode = get_inode_and_block(2,1,&b);
    if(new_inode<0)
        return ;
    inode_table[new_inode].file_type = 1;
    inode_table[new_inode].size = 1;
    inode_table[new_inode].link++;
    inode_table[new_inode].block_point[0] = b;
    spBlock->dir_inode_count++;
    
    create_a_new_dir_item(i,i_id,new_inode,tmp,1);

    // different from creating a file, as it has to create "." and "..".
    memset(block_buffer,0,BLOCK_SIZE);
    block_buffer[0].inode_id = new_inode;
    block_buffer[0].type = 1;
    block_buffer[0].item_count = 0;
    strcpy(block_buffer[0].name,".");
    block_buffer[1].inode_id = i_id;
    block_buffer[1].type = 1;
    block_buffer[1].item_count = 1;
    strcpy(block_buffer[1].name,"..");
    fseek(fp,BLOCK_SIZE*b,SEEK_SET);
    fwrite(block_buffer,BLOCK_SIZE,1,fp);
}

void delete_file(char *path)
{
    char tmp[121];
    int i_id = go_through_the_file_path(path,tmp);
    if(i_id<0)
        return ;
    int i,j;
    if(tmp[0] == '\0')
    {
        printf("Wrong path \n");
        return ;
    }
    if(check_text(tmp))
        return ;
    
    for(i=0;i<inode_table[i_id].size;i++)
    {
        fseek(fp,BLOCK_SIZE*inode_table[i_id].block_point[i],SEEK_SET);
        fread(block_buffer,BLOCK_SIZE,1,fp);
        for(j=0;j<8;j++)
        {
            if(block_buffer[j].type) ;
            else if(!strcmp(block_buffer[j].name,tmp))
            {
                int del_id = block_buffer[j].inode_id;
                if(--inode_table[del_id].link)
                    return ;
                delete_file_inode(del_id);
                delete_a_dir_item(i_id,i,j);
                return ;
            }
            if(block_buffer[j].item_count)
            {
                i = 100;
                break;
            }
        }
    }
    printf("Can't find a file named \"%s\" in this path\n",tmp);
}

void delete_dir(char *path)
{
    char tmp[121];
    int i,j,back_pos,back_i,back_j;
    int i_id = go_through_the_dir_path(path,tmp);
    if(i_id<0||tmp[0]=='\0')
    {
        if((path[0] == '/'&&path[1]=='\0')||((!strcmp(tmp,"."))&&(global_inode==0)))
            printf("Can not delete folder \"root\" \n");
        return ;
    }
    if(check_text(tmp))
        return ;
    
    int del_id = 0;
    for(i=0;i<inode_table[i_id].size;i++)
    {
        fseek(fp,BLOCK_SIZE*inode_table[i_id].block_point[i],SEEK_SET);
        fread(block_buffer,BLOCK_SIZE,1,fp);
        for(j=0;j<8;j++)
        {
            if(block_buffer[j].type)
            {
                if(!strcmp(block_buffer[j].name,tmp))
                {
                    back_i = i;
                    back_j = j;
                    back_pos = BLOCK_SIZE*inode_table[i_id].block_point[i];
                    del_id = block_buffer[j].inode_id;
                    if(del_id==global_inode)
                    {
                        printf("Please quit folder \"%s\" before deleting it.\n",tmp);
                        return ;
                    }
                    i = 100;
                    break;
                }
            }
            if(block_buffer[j].item_count)
            {
                i = 100;
                break;
            }
        }
    }
    if(!del_id)
    {
        printf("Can't find a folder named \"%s\" in this path\n",tmp);
        return ;
    }

    if(--inode_table[del_id].link)
        return ;
    del_sub_dir(del_id,back_pos);
    delete_a_dir_item(i_id,back_i,back_j);
}

void move(char *from,char *to)
{
    char *to_tmp;
    int i = 0,j;
    for(;i<512,to[i]!='\0';i++) ;
    if(i>=512)
        to_tmp = (char*)malloc(514);
    else 
        to_tmp = (char*)malloc(513);
    strcpy(to_tmp,to);
    if(to[i-1]=='/')
        strcat(to_tmp,"/");
    else 
        strcat(to_tmp,"//");
    // so we can get the last folder's inode later with this path.
    char f_tmp[121],d_tmp[121];
    int f_i_id = go_through_the_file_path(from,f_tmp);
    int d_i_id = go_through_the_dir_path(to_tmp,d_tmp);

    for(i=0;i<inode_table[f_i_id].size;i++)
    {
        fseek(fp,BLOCK_SIZE*inode_table[f_i_id].block_point[i],SEEK_SET);
        fread(block_buffer,BLOCK_SIZE,1,fp);
        for(j=0;j<8;j++)
        {
            if(!block_buffer[j].type)
            {
                if(!strcmp(block_buffer[j].name,f_tmp))
                {
                    int new_inode = block_buffer[j].inode_id;
                    if(deduplicate_name(f_tmp,d_i_id,_FILE_)<0)
                    {
                        printf("There is already a file named \"%s\" in this path !\n",f_tmp);
                        return ;
                    }
                    fseek(fp,BLOCK_SIZE*inode_table[f_i_id].block_point[i],SEEK_SET);
                    fread(block_buffer,BLOCK_SIZE,1,fp);
                    delete_a_dir_item(f_i_id,i,j);
                    fseek(fp,BLOCK_SIZE*inode_table[d_i_id].block_point[inode_table[d_i_id].size-1],SEEK_SET);
                    fread(block_buffer,BLOCK_SIZE,1,fp);
                    int l = 0;
                    for(;l<8;l++)
                    {
                        if(block_buffer[l].item_count)
                            break;
                    }
                    create_a_new_dir_item(l,d_i_id,new_inode,f_tmp,0);
                    free(to_tmp);
                    return ;
                }
                if(block_buffer[j].item_count)
                {
                    i = 100;
                    break;
                }
            }
        }
    }
    printf("Can't find a file named \"%s\" in this path\n",f_tmp);
    free(to_tmp);
}

void cd(char *path)
{
    int i = 0;
    char *path_tmp = (char*)malloc(514);
    while(path[i]!='\0')
        i++;
    strcpy(path_tmp,path);
    if(path[i-1]=='/')
        strcat(path_tmp,"/");
    else 
        strcat(path_tmp,"//");
    char global_name_tmp[512];
    int global_tmp = go_through_the_dir_path_to_end(path_tmp,global_name_tmp);
    if(global_tmp<0)
        return ;
    if(global_tmp)
    {
        // if(path_tmp[i+1]=='\0')
        //     path_tmp[i-1]='\0';
        // else 
        //     path_tmp[i]='\0';
        // if(path_tmp[0]!='/')
        // {
        //     strcat(global_name,"/");
        //     strcat(global_name,path_tmp);
        // }
        // else 
        //     strcpy(global_name,path_tmp);
        strcpy(global_name,global_name_tmp);
    }
    else 
        global_name[0] = '\0';
    
    global_inode = global_tmp;
    free(path_tmp);
}

void copy(char *ori, char *dest)
{
    char ori_name[121];
    int i;
    int tmp_id = go_through_the_file_path(ori,ori_name);
    if(tmp_id<0 || ori_name[0]=='\0' || check_text(ori_name))
        return ;
    
    if((i = deduplicate_name(ori_name,tmp_id,_FILE_))>=0)
    {
        printf("%s does not exist.\n",ori);
        return ;
    }
    
    int ori_id = -i;
    int size = inode_table[ori_id].size;

    int j=0;
    while(dest[j]!='\0')
        j ++;
    char dest_name[514];
    strcpy(dest_name,dest);
    if(dest[j-1]=='/')
        strcat(dest_name,"/");
    else 
        strcat(dest_name,"//");
    char fd_name[121];
    int dir_id = go_through_the_dir_path_to_end(dest_name,fd_name);
    if(dir_id<0)
        return;
    if(dest_name[j-1]=='/')
        dest_name[j] = '\0';
    else 
        dest_name[j+1] = '\0';

    strcat(dest_name,ori_name);
    create_file(dest_name,size);
    // copy content codes.
}