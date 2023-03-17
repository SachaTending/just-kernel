#include "printf.h"
#include "string.h"
#include "pit.h"

/* POSIX ustar header format */
typedef struct {                /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char prefix[167];             /* 345 */
} __attribute__((packed)) tar_t;

int oct2bin(char *s, int n)
{
    int r=0;
    while(n-->0) {
        r<<=3;
        r+=*s++-'0';
    }
    return r;
}

int memcmp_l(void *s1, void *s2, int n)
{
    unsigned char *a=s1,*b=s2;
    while(n-->0){ if(*a!=*b) { return *a-*b; } a++; b++; }
    return 0;
}

int is_ustar(char *buf) {
    if (buf[0] == 'u') {
        if (buf[1] == 's') {
            if (buf[2] == 't') {
                if (buf[3] == 'a') {
                    if (buf[4] == 'r') {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int tar_lookup(char *archive, char *filename, char **out) {
    char *ptr = archive;
 
    while (!memcmp(ptr + 257, "ustar", 5)) {
        int filesize = oct2bin(ptr + 0x7c, 11);
        if (!memcmp(ptr, filename, strlen(filename) + 1)) {
            *out = ptr + 512;
            return filesize;
        }
        ptr += (((filesize + 511) / 512) + 1) * 512;
    }
    return 0;
}

void ustar_list(char *buf)
{
    char *types[]={"regular", "link  ", "symlnk", "chrdev", "blkdev", "dircty", "fifo  ", "???   "};
    //printf("%c %c %c %c %c\n", *buf+257, *buf+258, *buf+259, *buf+260, *buf+261);
    // iterate on archive's contents
    while(is_ustar(buf+257)) {
        // if it's an ustar archive
        tar_t *header=(tar_t*)buf;
        int fs=oct2bin(header->size,11);
        // print out meta information
        printf("USTAR: File: %s\n", header->name);
        printf("USTAR: Size: %d\n", fs);
        if (fs < 80) {
            printf("USTAR: Content: ");
            printf((char *)buf+sizeof(tar_t));
        }
        printf("\n");
        // jump to the next file
        buf+=(((fs+511)/512)+1)*512;
    }
}