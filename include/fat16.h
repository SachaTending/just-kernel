#ifndef _FAT16_H
#define _FAT16_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned int uint;

#include <stdbool.h>

#define PACKED __attribute__((__packed__))

#define SECTOR_SIZE 512

// ------------------------------------------------------------------------------------------------
typedef struct BiosParamBlock
{
    u8 jump[3];
    u8 oem[8];
    u16 bytesPerSector;
    u8 sectorsPerCluster;
    u16 reservedSectorCount;
    u8 fatCount;
    u16 rootEntryCount;
    u16 sectorCount;
    u8 mediaType;
    u16 sectorsPerFat;
    u16 sectorsPerTrack;
    u16 headCount;
    u32 hiddenSectorCount;
    u32 largeSectorCount;

    // Extended block
    u8 driveNumber;
    u8 flags;
    u8 signature;
    u32 volumeId;
    u8 volumeLabel[11];
    u8 fileSystem[8];
} PACKED BiosParamBlock;

typedef struct DirEntry
{
    // Following conventions of DOS 7.0
    u8 name[8];
    u8 ext[3];
    u8 attribs;
    u8 reserved;
    u8 createTimeMs;
    u16 createTime;
    u16 createDate;
    u16 accessDate;
    u16 extendedAttribsIndex;
    u16 mTime;
    u16 mDate;
    u16 clusterIndex;
    u32 fileSize;
} PACKED DirEntry;

uint FatGetTotalSectorCount(u8 *image);
u8 *FatAllocImage(uint imageSize);
DirEntry *FatGetRootDirectory(u8 *image);

#define ENTRY_AVAILABLE 0x00
#define ENTRY_ERASED 0xe5
#endif