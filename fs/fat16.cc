#include "stdint.h"
#include "fat16.h"
#include "printf.h"
#include "malloc.h"
#include "string.h"

uint FatGetTotalSectorCount(u8 *image)
{
    BiosParamBlock *bpb = (BiosParamBlock *)image;

    if (bpb->sectorCount)
    {
        return bpb->sectorCount;
    }
    else
    {
        return bpb->largeSectorCount;
    }
}

u8 *FatAllocImage(uint imageSize)
{
    u8 *image = (u8 *)malloc(imageSize);
    memset(image, ENTRY_ERASED, imageSize);
    return image;
}

uint FatGetImageSize(u8 *image)
{
    BiosParamBlock *bpb = (BiosParamBlock *)image;

    return FatGetTotalSectorCount(image) * bpb->bytesPerSector;
}

DirEntry *FatGetRootDirectory(u8 *image)
{
    BiosParamBlock *bpb = (BiosParamBlock *)image;

    uint offset = (bpb->reservedSectorCount + bpb->fatCount * bpb->sectorsPerFat) * bpb->bytesPerSector;
    uint dataSize = bpb->rootEntryCount * sizeof(DirEntry);

    printf("FAT16: offset + dataSize <= FatGetImageSize(image) = %d\n", offset + dataSize <= FatGetImageSize(image));

    return (DirEntry *)(image + offset);
}

DirEntry *FatFindFreeRootEntry(u8 *image)
{
    BiosParamBlock *bpb = (BiosParamBlock *)image;

    DirEntry *start = FatGetRootDirectory(image);
    DirEntry *end = start + bpb->rootEntryCount;

    for (DirEntry *entry = start; entry != end; ++entry)
    {
        u8 marker = entry->name[0];
        if (marker == ENTRY_AVAILABLE || marker == ENTRY_ERASED)
        {
            return entry;
        }
    }

    return 0;
}