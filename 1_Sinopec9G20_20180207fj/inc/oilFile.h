#ifndef _OIL_FILE_H_
#define _OIL_FILE_H_

/*
在文件"yaffs22\yaffsfs.h"中的参数定义
#undef NAME_MAX
#undef O_RDONLY
#undef O_WRONLY
#undef O_RDWR
#undef O_CREAT		
#undef O_EXCL
#undef O_TRUNC
#undef O_APPEND
#undef SEEK_SET
#undef SEEK_CUR
#undef SEEK_END
#undef EBUSY
#undef ENODEV
#undef EINVAL
#undef EBADF
#undef EACCESS
#undef EXDEV	
#undef ENOENT
#undef ENOSPC
#undef ENOTEMPTY
#undef ENOMEM
#undef EEXIST
#undef ENOTDIR
#undef EISDIR
#undef S_IFMT
#undef S_IFLNK
#undef S_IFDIR
#undef S_IFREG
#undef S_IREAD 
#undef S_IWRITE
#undef S_IFSOCK
#undef S_IFIFO
#undef S_IFCHR
#undef S_IFBLK
*/


#define fileOpenDir	yaffs_opendir
#define fileReadDir	yaffs_readdir
#define fileCloseDir 	yaffs_closedir

//#define ERROR (-1)
//#define ok    (0)

/*函数外部声明*/
extern int fileWriteForPath(const char *path, int oflag, int mode, off_t offset, void *buffer, int nbytes);
extern int fileReadForPath(const char *path, int oflag, int mode, off_t offset, void *buffer, int maxbytes);
extern int fileTruncateForPath(const char *path, int oflag, int mode, off_t length);
extern int fileMoveForPath(const char *src, const char *dst);
extern int fileCopyForPath(const char *src, const char *dst);
extern int fileDeleteForPath(const char *path);

extern int fileOpen(const char *path, int oflag, int mode);
extern int fileClose(int fd);
extern int fileWrite(int fd, off_t offset, void *buffer, int nbytes);
extern int fileRead(int fd, off_t offset, void *buffer, int maxbytes);
extern int fileTruncate(int fd, off_t newSize);
extern int fileInit(void);

extern int fileFlashOpen(const char *path, int oflag, int mode);
extern int fileFlashClose(int fd);



extern void fileTest(void);
extern void fileTestExit(void);
extern void ttTest(void);

#endif

