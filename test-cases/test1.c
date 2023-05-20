#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <crypter.h>

int main()
{
  DEV_HANDLE cdev;
  char *msg = "Hello CS730!";
  char op_text[16];
  KEY_COMP a=30, b=17;
  uint64_t size = strlen(msg);
  strcpy(op_text, msg);
  cdev = create_handle();

  if(cdev == ERROR)
  {
    printf("Unable to create handle for device\n");
    exit(0);
  }

  if(set_key(cdev, a, b) == ERROR){
    printf("Unable to set key\n");
    exit(0);
  }
// //sleep(2);
  //if(set_config(cdev, DMA, SET) == ERROR){
    //printf("Unable to set key\n");
    //exit(0);
  //}

  // if(set_config(cdev, INTERRUPT, SET) == ERROR){
  //   printf("Unable to set key\n");
  //   exit(0);
  // }

  printf("Original Text: %s\n", msg);

  encrypt(cdev, op_text, size, 0);
  printf("Encrypted Text: %s\n", op_text);

  // sleep(2);
  // if(set_config(cdev, DMA, UNSET) == ERROR){
  //   printf("Unable to set key\n");
  //   exit(0);
  // }

  decrypt(cdev, op_text, size, 0);
  printf("Decrypted Text: %s\n", op_text);

// sleep(2);
//   if(set_config(cdev, DMA, SET) == ERROR){
//     printf("Unable to set key\n");
//     exit(0);
//   }

//   if(set_config(cdev, INTERRUPT, SET) == ERROR){
//     printf("Unable to set key\n");
//     exit(0);
//   }

//   if(set_key(cdev, 20, 7) == ERROR){
//     printf("Unable to set key\n");
//     exit(0);
//   }

//   printf("Original Text: %s\n", msg);

//   encrypt(cdev, op_text, size, 0);
//   printf("Encrypted Text: %s\n", op_text);


//   if(set_config(cdev, DMA, UNSET) == ERROR){
//     printf("Unable to set key\n");
//     exit(0);
//   }
// sleep(3);
//   decrypt(cdev, op_text, size, 0);
//   printf("Decrypted Text: %s\n", op_text);

//   if(set_config(cdev, DMA, SET) == ERROR){
//     printf("Unable to set key\n");
//     exit(0);
//   }

//   if(set_config(cdev, INTERRUPT, UNSET) == ERROR){
//     printf("Unable to set key\n");
//     exit(0);
//   }

//   if(set_key(cdev, 50, 34) == ERROR){
//     printf("Unable to set key\n");
//     exit(0);
//   }

//   printf("Original Text: %s\n", msg);

//   encrypt(cdev, op_text, size, 0);
//   printf("Encrypted Text: %s\n", op_text);

// sleep(3);
//   if(set_config(cdev, DMA, UNSET) == ERROR){
//     printf("Unable to set key\n");
//     exit(0);
//   }

//   decrypt(cdev, op_text, size, 0);
//   printf("Decrypted Text: %s\n", op_text);

  close_handle(cdev);
  return 0;
}

// #include <string.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <crypter.h>

// int main()
// {

// DEV_HANDLE cdev = create_handle();
// char *msg = "Hello CS614!\n";
// KEY_COMP a=30, b=17;
// uint64_t size = strlen(msg);
// set_config(cdev, DMA, 0);   // MMAP only valid in MMIO mode
// set_config(cdev, INTERRUPT, 0); // This test case is w/o interrupts 
// set_key(cdev, a, b);
// //printf("");
// printf("passed\n");
// //return 0;
// char *actual_buff = map_card(cdev, size);   // Return a pointer mapped to the device memory

// //printf("after map_card");
// printf("Original message %s",msg);
// strncpy(actual_buff, msg, size);

// encrypt(cdev, actual_buff, size, 1);   // Last argument is 1 ==> it is mapped
// printf("Encrypted message %s",actual_buff);

// decrypt(cdev, actual_buff, size, 1);   // Last argument is 1 ==> it is mapped
// printf("Decrypted message %s",actual_buff);
// //At this point, "actual_buf" contains the encrypted message
// unmap_card(cdev, actual_buff);
// close_handle(cdev);

// }
