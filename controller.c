#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include "inode.h"

#define BLOCK_SIZE 4096;
#define MAX_FILES_COUNT 100;
#define INODE_AREA_SIZE MAX_FILES_COUNT * sizeof(struct inode_t);

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

static const char *fsfilename = "fsfile";

void WriteToFile()
{
	struct inode_t node;
	FILE *output;
	int i;
	int count=5, range=10; //диапазон
	if((output=fopen(fsfilename,"wb"))==0)
	{
		puts ("Sorry,Can't open output file.");
		exit(-1);
	}
	
	for(i=0; i<count; i++)
	{
		int n=rand()%range;
		fwrite(&n,sizeof(int),output);
	}
	fclose(output);
}

/*
*функция определяет метаинформацию о файле (путь к нему -*path
*метаинформация возвращается в виде структуры stat).
*указатель на функцию передадим модулю фьюз как поле getattr cтруктуры 
*fuse_operations
*/
 
static int fs_getattr(const char *path, struct stat *stbuf)
{
    int res = 0;
 
    memset(stbuf, 0, sizeof(struct stat));//инициализируем первые sizeof(struct stat) 0
/*если root dir*/
    if(strcmp(path, "/") == 0) {
//биты прав доступа к файлу, тип файла и спец биты.
        stbuf->st_mode = S_IFDIR | 0755;//каталог
        stbuf->st_nlink = 2;//ссыл.записей на задан. файл в директории
    }
//папка,использующаяся в качестве жесткого диска
    else if(strcmp(path, hello_path) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(hello_str); //размер файла
    }
    else
        res = -ENOENT;
 
    return res;
}

/*определяет порядок чтения данных из директории, указатель не нее отдадим в качестве поля readdir*/
/*т.е получаем содержимое каталога, используя указатель на функцию filler*/
static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;
 
    if(strcmp(path, "/") != 0)
        return -ENOENT;
 
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, hello_path + 1, NULL, 0);
 
    return 0;
}
 
//определяет имеет ли право пользователь открыть файл /hello, реализуется через анализ данных структуры типа fuse_file_info
static int fs_open(const char *path, struct fuse_file_info *fi)
{
    if(strcmp(path, hello_path) != 0)
        return -ENOENT;
 
    if((fi->flags & 3) != O_RDONLY)
        return -EACCES;
 
    return 0;
}
 /*определяет, как именно будет считываться информация из файла для передачи пользователю*/
static int fs_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
{
    size_t len;
    (void) fi;
    if(strcmp(path, hello_path) != 0)
        return -ENOENT;
 
    len = strlen(hello_str);
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, hello_str + offset, size);
    } else
        size = 0;
 
    return size;
}

static struct fuse_operations fs_oper; //необходимая для создания файловой системы переменная структуры с типом fuse_operations, будет необходимо передать ее в функцию fuse_main

/*структура fuse_operations передает указатель на функции, которые будут вызываться для выполнения соответ. действия*/

/*то есть создаем небходимые функции с логикой их выполнения, затем создаем переменную fuse_operations и отдаем ей соотв. функций,которые необходимо будет использовать*/
int main(int argc, char *argv[])
{
	 WriteToFile();
	fs_oper.getattr = hello_getattr; //передаем указатели на соотв.функции
	fs_oper.readdir = hello_readdir;
	fs_oper.open = hello_open;
	fs_oper.read = hello_read;

	return fuse_main(argc, argv, &hello_oper, 0);//отдаем переменную fuse_operations (инф ою операциях) модулю фьюз
}