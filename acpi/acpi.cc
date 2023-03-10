#include "stdint.h"
#include "printf.h"
#include "string.h"
#include "module.h"
#include "port_io.h"

typedef struct AcpiHeader
{
    uint32_t signature;
    uint32_t length;
    uint8_t  revision;
    uint8_t  checksum;
    uint8_t  oem[6];
    uint8_t  oemTableId[8];
    uint32_t oemRevision;
    uint32_t creatorId;
    uint32_t creatorRevision;
} __attribute__((packed)) AcpiHeader;

typedef struct AcpiMadt
{
    AcpiHeader header;
    uint32_t localApicAddr;
    uint32_t flags;
} __attribute__((packed)) AcpiMadt;

static AcpiMadt *s_madt;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

extern u8 *g_localApicAddr;

typedef struct ApicHeader
{
    uint8_t type;
    uint8_t length;
} __attribute__((packed)) ApicHeader;

typedef struct ApicLocalApic
{
    ApicHeader header;
    u8 acpiProcessorId;
    u8 apicId;
    u32 flags;
} __attribute__((packed)) ApicLocalApic;

typedef struct ApicIoApic
{
    ApicHeader header;
    u8 ioApicId;
    u8 reserved;
    u32 ioApicAddress;
    u32 globalSystemInterruptBase;
} __attribute__((packed)) ApicIoApic;

typedef struct ApicInterruptOverride
{
    ApicHeader header;
    u8 bus;
    u8 source;
    u32 interrupt;
    u16 flags;
} __attribute__((packed)) ApicInterruptOverride;

typedef struct AcpiFadt
{
    AcpiHeader header;
    u32 firmwareControl;
    u32 dsdt;
    u8 reserved;
    u8 preferredPMProfile;
    u16 sciInterrupt;
    u32 smiCommandPort;
    u8 acpiEnable;
    u8 acpiDisable;
} __attribute__((packed)) AcpiFadt;

#define APIC_TYPE_LOCAL_APIC            0
#define APIC_TYPE_IO_APIC               1
#define APIC_TYPE_INTERRUPT_OVERRIDE    2

#define MAX_CPU_COUNT 16

extern u8 *g_localApicAddr;
extern u8 *g_ioApicAddr;

unsigned int g_acpiCpuCount;
u8 g_acpiCpuIds[MAX_CPU_COUNT];

static void AcpiParseFacp(AcpiFadt *facp)
{
    if (facp->smiCommandPort)
    {
        printf("ACPI: Enabling ACPI\n");
        outportb(facp->smiCommandPort, facp->acpiEnable);
    }
    else
    {
        printf("ACPI: ACPI already enabled\n");
    }
}

void LocalApicSendInit(unsigned int apic_id);
void LocalApicSendStartup(unsigned int apic_id, unsigned int vector);

extern "C" void CpuPayload()
{
    printf("Hello, im payload running from application cpu, this message indicates im booted up.");
}

extern "C" void ap_startup(int apicid)
{
    CpuPayload();
    while(1);
}

extern "C" void payload();

extern "C" uint8_t bspdone = 0;

static void AcpiParseApic(AcpiMadt *madt)
{
    printf("ACPI: ACPI Disabled.");
    return;
    s_madt = madt;

    printf("ACPI: Local APIC Address = 0x%x\n", madt->localApicAddr);
    g_localApicAddr = (u8 *)(uintptr_t)madt->localApicAddr;

    u8 *p = (u8 *)(madt + 1);
    u8 *end = (u8 *)madt + madt->header.length;

    while (p < end)
    {
        ApicHeader *header = (ApicHeader *)p;
        u8 type = header->type;
        u8 length = header->length;

        if (type == APIC_TYPE_LOCAL_APIC)
        {
            ApicLocalApic *s = (ApicLocalApic *)p;

            printf("ACPI: Found CPU: ID: %d APIC ID: %d FLAGS: %x\n", s->acpiProcessorId, s->apicId, s->flags);
            if (g_acpiCpuCount < MAX_CPU_COUNT)
            {
                g_acpiCpuIds[g_acpiCpuCount] = s->apicId;
                ++g_acpiCpuCount;
            }
            if (s->apicId != 0)
            {
                outportb(0xa1, 0xff);
                outportb(0x21, 0xff);
                asm volatile ("cli");
                printf("ACPI: Initializating CPU...\n");
                /*
                *((volatile uint32_t*)(g_localApicAddr + 0x280)) = 0;                                                                             // clear APIC errors
                *((volatile uint32_t*)(g_localApicAddr + 0x310)) = (*((volatile uint32_t*)(g_localApicAddr + 0x310)) & 0x00ffffff) | (s->apicId << 24);         // select AP
                *((volatile uint32_t*)(g_localApicAddr + 0x300)) = (*((volatile uint32_t*)(g_localApicAddr + 0x300)) & 0xfff00000) | 0x00C500;          // trigger INIT IPI
                do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile uint32_t*)(g_localApicAddr + 0x300)) & (1 << 12));         // wait for delivery
                *((volatile uint32_t*)(g_localApicAddr + 0x310)) = (*((volatile uint32_t*)(g_localApicAddr + 0x310)) & 0x00ffffff) | (s->apicId << 24);         // select AP
                *((volatile uint32_t*)(g_localApicAddr + 0x300)) = (*((volatile uint32_t*)(g_localApicAddr + 0x300)) & 0xfff00000) | 0x008500;          // deassert
                do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile uint32_t*)(g_localApicAddr + 0x300)) & (1 << 12));         // wait for delivery                                                                                                             // wait 10 msec
                //for (int i; i++; i<100) { asm volatile ("nop");}*/
                LocalApicSendInit(s->acpiProcessorId);

                printf("ACPI: Sending vector to CPU...\n");
                printf("ACPI: Payload address: 0x%x\n", 0x8000);
                /*
                for(int j = 0; j < 2; j++) {
                    *((volatile uint32_t*)(g_localApicAddr + 0x280)) = 0;                                                                     // clear APIC errors
                    *((volatile uint32_t*)(g_localApicAddr + 0x310)) = (*((volatile uint32_t*)(g_localApicAddr + 0x310)) & 0x00ffffff) | (s->acpiProcessorId << 24); // select AP
                    *((volatile uint32_t*)(g_localApicAddr + 0x300)) = (*((volatile uint32_t*)(g_localApicAddr + 0x300)) & 0xfff0f800) | 0x000608;  // trigger STARTUP IPI for 0800:0000                                                                                                      // wait 200 usec
                    do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile uint32_t*)(g_localApicAddr + 0x300)) & (1 << 12)); // wait for delivery
                */
                LocalApicSendStartup(s->acpiProcessorId, 0x8000);
                bspdone = 1;
                
            }
        }
        else if (type == APIC_TYPE_IO_APIC)
        {
            ApicIoApic *s = (ApicIoApic *)p;

            printf("ACPI: Found I/O APIC: ID: %d ADDR: 0x%x GLOBAL SYSTEM INTERRUPT BASE: %d\n", s->ioApicId, s->ioApicAddress, s->globalSystemInterruptBase);
            g_ioApicAddr = (u8 *)(uintptr_t)s->ioApicAddress;
        }
        else if (type == APIC_TYPE_INTERRUPT_OVERRIDE)
        {
            ApicInterruptOverride *s = (ApicInterruptOverride *)p;

            printf("ACPI: Found Interrupt Override: BUS: %d SOURCE: %d INTERRUPT: %d FLAGS: 0x%x\n", s->bus, s->source, s->interrupt, s->flags);
        }
        else
        {
            printf("ACPI: Unknown APIC structure %d\n", type);
        }

        p += length;
    }
}



static void AcpiParseDT(AcpiHeader *header)
{
    uint32_t signature = header->signature;

    char sigStr[5];
    memcpy(sigStr, &signature, 4);
    sigStr[4] = 0;
    printf("ACPI: %s 0x%x\n", sigStr, signature);

    if (signature == 0x50434146)
    {
        AcpiParseFacp((AcpiFadt *)header);
    }
    else if (signature == 0x43495041)
    {
        AcpiParseApic((AcpiMadt *)header);
    }
}

static void AcpiParseRsdt(AcpiHeader *rsdt)
{
    uint32_t *p = (uint32_t *)(rsdt + 1);
    uint32_t *end = (uint32_t *)((uint8_t*)rsdt + rsdt->length);

    while (p < end)
    {
        uint32_t address = *p++;
        AcpiParseDT((AcpiHeader *)(uintptr_t)address);
    }
}

static void AcpiParseXsdt(AcpiHeader *xsdt)
{
    uint64_t *p = (uint64_t *)(xsdt + 1);
    uint64_t *end = (uint64_t *)((uint8_t*)xsdt + xsdt->length);

    while (p < end)
    {
        uint64_t address = *p++;
        AcpiParseDT((AcpiHeader *)(uintptr_t)address);
    }
}

static bool AcpiParseRsdp(uint8_t *p) // From https://github.com/pdoane/osdev/blob/master/acpi/acpi.c
{
    // Parse Root System Description Pointer
    printf("ACPI: RSDP found\n");

    // Verify checksum
    uint8_t sum = 0;
    for (unsigned int i = 0; i < 20; ++i)
    {
        sum += p[i];
    }

    if (sum)
    {
        printf("ACPI: Checksum failed\n");
        return false;
    }

    // Print OEM
    char oem[7];
    memcpy(oem, p + 9, 6);
    oem[6] = '\0';
    printf("ACPI: OEM = %s\n", oem);

    // Check version
    uint8_t revision = p[15];
    if (revision == 0)
    {
        printf("ACPI: Version 1\n");

        uint32_t rsdtAddr = *(uint32_t *)(p + 16);
        AcpiParseRsdt((AcpiHeader *)(uintptr_t)rsdtAddr);
    }
    else if (revision == 2)
    {
        printf("ACPI: Version 2\n");

        uint32_t rsdtAddr = *(uint32_t *)(p + 16);
        uint64_t xsdtAddr = *(uint64_t *)(p + 24);

        if (xsdtAddr)
        {
            AcpiParseXsdt((AcpiHeader *)(uintptr_t)xsdtAddr);

        }
        else
        {
            AcpiParseRsdt((AcpiHeader *)(uintptr_t)rsdtAddr);
        }
    }
    else
    {
        printf("ACPI: Unsupported ACPI version %d\n", revision);
    }

    return true;
}

MODULE_START_CALL void AcpiInit() // From https://github.com/pdoane/osdev/blob/master/acpi/acpi.c
{
    // TODO - Search Extended BIOS Area

    // Search main BIOS area below 1MB
    uint8_t *p = (uint8_t *)0x000e0000;
    uint8_t *end = (uint8_t *)0x000fffff;

    while (p < end)
    {
        uint64_t signature = *(uint64_t *)p;

        if (signature == (unsigned long) 0x2052545020445352) // 'RSD PTR '
        {
            if (AcpiParseRsdp(p))
            {
                return;
                
            }
        }

        p += 16;
    }
}