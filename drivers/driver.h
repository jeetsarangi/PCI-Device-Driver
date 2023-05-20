#ifndef CHARDEV_H
#define CHARDEV_H

#include<linux/kernel.h>
#include<linux/mm.h>
#include<linux/mm_types.h>
#include<linux/file.h>
#include<linux/fs.h>
#include<linux/path.h>
#include<linux/slab.h>
#include<linux/dcache.h>
#include<linux/sched.h>
#include<linux/uaccess.h>
#include<linux/fs_struct.h>
#include<linux/uaccess.h>
#include<linux/device.h>
#include<linux/ioctl.h>

#include<linux/pci.h>
#include<linux/mod_devicetable.h>
#include<linux/init.h>
#include<linux/module.h>
#include <linux/pci.h>
#include<linux/mutex.h>

#define SET 1
#define UNSET 0
#define TRUE 1
#define FALSE 0
#define ERROR -1

#define INTR_STATUS_OFFSET 0x24 //RO
#define MMIO_STATUS_OFFSET 0x20 //RO
#define MMIO_LEN_OFFSET 0x0c //RW
#define KEY_A_OFFSET 0x0a //WO
#define KEY_B_OFFSET 0x0b //WO
#define IDENTIFICATION_OFFSET 0x0 //RO
#define LIVENESS_OFFSET 0x04 //RW
#define INTR_RAISE_OFFSET //WO
#define INTR_ACK_OFFSET 0x64 //WO
#define MMIO_ADDR_OFFSET 0x80 //WO
#define DMA_ADDR_OFFSET 0x90 //RW
#define DMA_LEN_OFFSET 0x98 //RW
#define DMA_COMM_OFFSET 0xa0 //RW

#define MAJOR_NUM 100
#define DEVNAME "CryptoCardChardev"


/////////////PCI//////////////////

#define MY_DRIVER "Cryptocard"



#define CRYPTOVENDOR_ID 0x1234
#define CRYTODEVICE_ID 0xDEBA

#define OFFSET_IDENTIFICATION 0x00
#define OFFSET_LIVENESS 0x04
#define OFFSET_SETKEYS 0x08
#define OFFSET_LOMMMIO 0x0C
#define OFFSET_STATUSMMIO 0x20
#define OFFSET_STATUSINT 0x24
#define OFFSET_INTRAISE 0x60
#define OFFSET_INTACK 0x64
#define OFFSET_DATAADDRMMIO 0x80
#define OFFSET_DMAADDR 0x90
#define OFFSET_MSGLENDMA 0x98
#define OFFSET_DMACMDREG 0xa0
#define OFFSET_LASTADDR 0xa8

#define MMIO_INTRUPT 0x001
#define DMA_INTRUPT 0x100



static struct pci_device_id my_driver_id_table[] = {
    { PCI_DEVICE(CRYPTOVENDOR_ID, CRYTODEVICE_ID) },
    {0,}
};

struct mutex cs614_lock;

//////////////PCI//////////////////


#define IOCTL_ENCRYPT _IOWR(MAJOR_NUM, 0, char*)
#define IOCTL_DECRYPT _IOWR(MAJOR_NUM, 1, char*)
#define IOCTL_SETKEYS _IOWR(MAJOR_NUM, 2, char*)
#define IOCTL_SETCONFIG _IOWR(MAJOR_NUM, 3, char*)


#define DMA_BUFF_SIZE 32768

typedef enum {INTERRUPT, DMA} config_t;
typedef unsigned char KEY_COMP;
typedef void * ADDR_PTR;
dma_addr_t dma_handle;
char * dma_space;

struct encrypt_data{
    ADDR_PTR addr;
    uint64_t length;
    uint8_t isMapped;
};

struct key_data{
    KEY_COMP a;
    KEY_COMP b;
};

struct config_data{
    config_t type; 
    uint8_t value;
};

#endif
