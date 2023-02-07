#include <printf2.h>
#include <device.h>
#include <ata.h>
#include <stdint.h>

#define EXT2_SIGNATURE 0xEF53

struct superblock {
	uint32_t inodes;
	uint32_t blocks;
	uint32_t reserved_for_root;
	uint32_t unallocatedblocks;
	uint32_t unallocatedinodes;
	uint32_t superblock_id;
	uint32_t blocksize_hint; // shift by 1024 to the left
	uint32_t fragmentsize_hint; // shift by 1024 to left
	uint32_t blocks_in_blockgroup;
	uint32_t frags_in_blockgroup;
	uint32_t inodes_in_blockgroup;
	uint32_t last_mount;
	uint32_t last_write;
	uint16_t mounts_since_last_check;
	uint16_t max_mounts_since_last_check;
	uint16_t ext2_sig; // 0xEF53
	uint16_t state;
	uint16_t op_on_err;
	uint16_t minor_version;
	uint32_t last_check;
	uint32_t max_time_in_checks;
	uint32_t os_id;
	uint32_t major_version;
	uint16_t uuid;
	uint16_t gid;
	uint8_t unused[940];
} __attribute__((packed));

typedef struct superblock superblock_t;

superblock_t *cur_ext2;

void ext2_scan_dev(device_t *dev)
{
    printf("EXT2: sizeof(superblock_t) = %d\n", sizeof(superblock_t));
    char buf[sizeof(superblock_t)];
    ata_read(buf, 2, 2, dev);
    superblock_t *block = (superblock_t *) buf;
    if (block->ext2_sig == EXT2_SIGNATURE){
        printf("EXT2: Drive signature valid!\n");
        cur_ext2 = block;
    } else {
        printf("EXT2: Drive signature not valid. Signature: 0x%x Excepted: 0x%x\n", block->ext2_sig, EXT2_SIGNATURE);
    }
}