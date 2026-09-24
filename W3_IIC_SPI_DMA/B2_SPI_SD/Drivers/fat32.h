#ifndef FAT32_H
#define FAT32_H


#define FAT32_OK       0
#define FAT32_ERROR    1


unsigned char FAT32_Mount(void);


unsigned long FAT32_ReadFile(
    char *filename,
    unsigned char *buffer,
    unsigned long max_size
);


unsigned char FAT32_WriteFile(
    char *filename,
    unsigned char *data,
    unsigned long size
);


#endif
