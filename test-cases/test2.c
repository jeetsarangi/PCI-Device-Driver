#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <crypter.h>

int main()
{

DEV_HANDLE cdev = create_handle();
char *msg = "Hello CS614!\n";
KEY_COMP a=30, b=17;
uint64_t size = strlen(msg);
set_config(cdev, DMA, 0);   // MMAP only valid in MMIO mode
set_config(cdev, INTERRUPT, 0); // This test case is w/o interrupts 
set_key(cdev, a, b);
char *actual_buff = map_card(cdev, size);   // Return a pointer mapped to the device memory
strncpy(actual_buff, msg, size);
encrypt(cdev, actual_buff, size, 1);   // Last argument is 1 ==> it is mapped
//At this point, "actual_buf" contains the encrypted message
unmap_card(cdev, actual_buff);
close_handle(cdev);

}