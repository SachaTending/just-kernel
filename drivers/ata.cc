#include <port_io.h>
#include <printf.h>
#include <stdint.h>
#include <ata.h>
#include <module.h>



#define ATA_PRIMARY_IO 0x1F0
#define ATA_SECONDARY_IO 0x170

#define ATA_PRIMARY_DCR_AS 0x3F6
#define ATA_SECONDARY_DCR_AS 0x376

#define ATA_PRIMARY_IRQ 14
#define ATA_SECONDARY_IRQ 15

uint8_t ata_pm = 0; /* Primary master exists? */
uint8_t ata_ps = 0; /* Primary Slave exists? */
uint8_t ata_sm = 0; /* Secondary master exists? */
uint8_t ata_ss = 0; /* Secondary slave exists? */

uint8_t *ide_buf = 0;

void ide_select_drive(uint8_t bus, uint8_t i)
{
	if(bus == ATA_PRIMARY)
		if(i == ATA_MASTER)
			outportb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xA0);
		else outportb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xB0);
	else
		if(i == ATA_MASTER)
			outportb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xA0);
		else outportb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xB0);
}

uint8_t ide_identify(uint8_t bus, uint8_t drive)
{
	uint16_t io = 0;
	ide_select_drive(bus, drive);
	if(bus == ATA_PRIMARY) io = ATA_PRIMARY_IO;
	else io = ATA_SECONDARY_IO;
	/* ATA specs say these values must be zero before sending IDENTIFY */
	outportb(io + ATA_REG_SECCOUNT0, 0);
	outportb(io + ATA_REG_LBA0, 0);
	outportb(io + ATA_REG_LBA1, 0);
	outportb(io + ATA_REG_LBA2, 0);
	/* Now, send IDENTIFY */
	outportb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	printf("ATA: Sent IDENTIFY\n");
	/* Now, read status port */
	uint8_t status = inportb(io + ATA_REG_STATUS);
	if(status)
	{
		/* Now, poll untill BSY is clear. */
		while(inportb(io + ATA_REG_STATUS) & ATA_SR_BSY != 0) ;
pm_stat_read:		status = inportb(io + ATA_REG_STATUS);
		if(status & ATA_SR_ERR)
		{
			printf("ATA:%s%s has ERR set. Disabled.\n", bus==ATA_PRIMARY?"Primary":"Secondary", drive==ATA_PRIMARY?" master":" slave");
			return 0;
		}
		while(!(status & ATA_SR_DRQ)) goto pm_stat_read;
		printf("ATA: %s%s is online.\n", bus==ATA_PRIMARY?"Primary":"Secondary", drive==ATA_PRIMARY?" master":" slave");
		for(int i = 0; i<256; i++)
		{
			*(uint16_t *)(ide_buf + i*2) = inportw(io + ATA_REG_DATA);
		}
	}
}