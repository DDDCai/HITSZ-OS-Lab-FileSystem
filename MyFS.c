/*
 * @Author: Deng Cai 
 * @Date: 2019-09-09 15:58:51 
 * @Last Modified by: Deng Cai
 * @Last Modified time: 2020-12-05 17:02:03
 */
#include "fs_operation.h"

void main(int argc, char *argv[])
{
    int i,para_num;
    spBlock = malloc(sizeof(sp_block));
    fp = fopen(argv[1],"r+");
    if(!fp)
    {
        printf("fail to load the disk\n");
        return ;
    }
    fseek(fp,0,SEEK_SET);

    if(fread(spBlock,sizeof(sp_block),1,fp)!=1)
    {
        printf("Fail to read super block \n");
        free(spBlock);
        fclose(fp);
        return ;
    }

    fs_init(spBlock);
    fseek(fp,0,SEEK_SET);

    while(1)
    {
        if(global_inode==0)
            printf("\033[;34m~\033[0m");
        else 
            printf("\033[;34m%s\033[0m",global_name);
        printf("==> ");
        para_num = 0;
        char instruction[512];
        fgets(instruction,512,stdin);
        for(i=0;i<512;i++)
            if(instruction[i]=='\n')
                break;

        if(i==512)
        {
            printf("This instruction is too long (> 512 bytes).\n");
            continue;
        }
        char order[16];
        for(i=0;i<16;i++)
        {
            if(instruction[i]==' ' || instruction[i]=='\n')
            {
                order[i] = '\0';
                break;
            }
            order[i] = instruction[i];
        }
        if(i==16)
        {
            printf("Undefined instruction !\n");
            continue;
        }
        char para_1[512],para_2[512];
        while(instruction[i]==' ')
            i++;
        if(instruction[i]!='\n')
        {
            int j = 0;
            while(instruction[i]!=' ')
            {
                if(instruction[i]=='\n')
                {
                    para_1[j++] = '\0';
                    para_num = 1;
                    para_2[0] = '\0';
                    goto DO;
                }
                para_1[j++] = instruction[i];
                i++;
            }
            para_1[j] = '\0';
            para_num = 1;
            while(instruction[i]==' ')
                i++;
            if(instruction[i]!='\n')
            {
                j=0;
                while(instruction[i]!=' ')
                {
                    if(instruction[i]=='\n')
                    {
                        para_2[j++] = '\0';
                        para_num = 2;
                        goto DO;
                    }
                    para_2[j++] = instruction[i];
                    i++;
                }
                para_2[j] = '\0';
                para_num = 2;
                while(instruction[i]==' ')
                    i++;
                if(instruction[i]!='\n')
                {
                    printf("Unrecognized or too many parameters exit \n");
                    continue;
                }
            }
        }

    // So the original instruction is divided into three parts : 
    // order, para_1, and para_2.
    DO:
        if(!strcmp(order,"shutdown"))
            break;
        else if(!strcmp(order,"ls"))
        {
            if(para_num==0)
                ls(".");
            else if(para_num==2)
                printf("Unrecognized or too many parameters exit \n");
            else 
                ls(para_1);
        }
        else if(!strcmp(order,"create"))
        {
            if(para_num!=2)
            {
                printf("Some parameters are missing\n");
                continue;
            }
            if(!strcmp(para_1,"-d"))
                create_dir(para_2);
            else 
            {
                i = 0;
                while(para_1[i]!='\0')
                {
                    if(para_1[i]>='0' && para_1[i]<='9')
                        i++;
                    else 
                    {
                        printf("Unrecognized parameters (an integer or -d)\n");
                        i = -1;
                        break;
                    }
                }
                if(i>0)
                    create_file(para_2,atoi(para_1));
            }
        }
        else if(!strcmp(order,"mkdir"))
            create_dir(para_1);
        else if(!strcmp(order,"delete"))
        {
            if(para_num!=2)
                printf("Some parameters are missing\n");
            else if(!strcmp(para_1,"-f"))
                delete_file(para_2);
            else if(!strcmp(para_1,"-d"))
                delete_dir(para_2);
            else 
                printf("Unrecognized parameters (-f or -d)\n");
        }
        else if(!strcmp(order,"move"))
        {
            if(para_num!=2)
                printf("Some parameters are missing\n");
            else
                move(para_1,para_2);
        }
        else if(!strcmp(order,"copy") || !strcmp(order,"cp"))
        {
            if(para_num!=2)
                printf("Some parameters are missing\n");
            else 
                copy(para_1,para_2);
        }
        else if(!strcmp(order,"information"))
            print_information(0);
        else if(!strcmp(order,"cd"))
        {
            if(para_num==2)
                printf("Unrecognized or too many parameters exit \n");
            else if(para_num==1)
                cd(para_1);
        }
        else
            printf("Undefined instruction !\n");
        continue;
    }
    printf("#############################  GoodBye  #############################\n");

    fseek(fp,0,SEEK_SET);
    if(fwrite(spBlock,sizeof(sp_block),1,fp)!=1)
        printf("Fail to write super block \n");
    fseek(fp,BLOCK_SIZE,SEEK_SET);
    if(fwrite(inode_table,BLOCK_SIZE,32,fp)!=32)
        printf("Fail to write inode table \n");
    fclose(fp);
    free(spBlock);
}