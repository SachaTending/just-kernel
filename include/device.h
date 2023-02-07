#ifndef _DEVICE_H
#define _DEVICE_H
#include <stdint.h>

typedef enum __device_type {
	DEVICE_UNKNOWN = 0,
	DEVICE_CHAR = 1,
	DEVICE_BLOCK = 2,
} device_type;


struct __device_t {
	char *name;
	uint32_t unique_id;
	device_type dev_type;
	struct __fs_t *fs;
	unsigned long pos;
	uint8_t (*read)(uint8_t* buffer, uint32_t offset , uint32_t len, void* dev);
	uint8_t (*write)(uint8_t *buffer, uint32_t offset, uint32_t len, void* dev);
	void *priv;
};

typedef struct __device_t device_t;

#endif