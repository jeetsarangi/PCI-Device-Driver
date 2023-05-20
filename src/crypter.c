#include<crypter.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include<stdio.h>

#define MAJOR_NUM 100
#define SIZE 1024
#define DMA_BUFFER 32768

#define IOCTL_ENCRYPT _IOWR(MAJOR_NUM, 0, char*)
#define IOCTL_DECRYPT _IOWR(MAJOR_NUM, 1, char*)
#define IOCTL_SETKEYS _IOWR(MAJOR_NUM, 2, char*)
#define IOCTL_SETCONFIG _IOWR(MAJOR_NUM, 3, char*)

// static unsigned long usr_size;

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

struct encrypt_data data_details ;
struct key_data key_d ;
struct config_data config_d ;





/*Function template to create handle for the CryptoCard device.
On success it returns the device handle as an integer*/
DEV_HANDLE create_handle()
{ 

  printf("Hii Will be Creating an handle for Crypto Device !!\n");
  int fd = open("/dev/CryptoCardChardev",O_RDWR);
  printf("Device Handle %i\n",fd);

  if(fd < 0)
    return ERROR;

  return fd;
}

/*Function template to close device handle.
Takes an already opened device handle as an arguments*/
void close_handle(DEV_HANDLE cdev)
{
  close(cdev);
  return;
}

/*Function template to encrypt a message using MMIO/DMA/Memory-mapped.
Takes four arguments
  cdev: opened device handle
  addr: data address on which encryption has to be performed
  length: size of data to be encrypt
  isMapped: TRUE if addr is memory-mapped address otherwise FALSE
*/
int encrypt(DEV_HANDLE cdev, ADDR_PTR addr, uint64_t length, uint8_t isMapped)
{

  unsigned long no_of_chunks,msg_len,temp_len;
  char* temp_addr = (char*)addr;
  temp_len = length;

  if((length % DMA_BUFFER) != 0){
    no_of_chunks = 1;  
  }
  no_of_chunks += (length / DMA_BUFFER);

  for(int i = 0;i < no_of_chunks;i++){

      if(temp_len > DMA_BUFFER)
        msg_len = DMA_BUFFER;
      else
        msg_len = temp_len;
      
      
      data_details.addr = temp_addr;
      data_details.length = msg_len;
      data_details.isMapped = isMapped;

      int ret = ioctl(cdev, IOCTL_ENCRYPT, &data_details);
      if(ret)
      {
          printf("Encryption failed ret : %d\n",ret);
          return ERROR;
      }

      temp_addr = temp_addr+DMA_BUFFER;
      temp_len = temp_len-DMA_BUFFER;


  }


  
  return 0;
  
}

/*Function template to decrypt a message using MMIO/DMA/Memory-mapped.
Takes four arguments
  cdev: opened device handle
  addr: data address on which decryption has to be performed
  length: size of data to be decrypt
  isMapped: TRUE if addr is memory-mapped address otherwise FALSE
*/
int decrypt(DEV_HANDLE cdev, ADDR_PTR addr, uint64_t length, uint8_t isMapped)
{

  unsigned long no_of_chunks,msg_len,temp_len;
  char* temp_addr = (char*)addr;
  temp_len = length;

  if((length % DMA_BUFFER) != 0){
    no_of_chunks = 1;  
  }
  no_of_chunks += (length / DMA_BUFFER);

  for(int i = 0;i < no_of_chunks;i++){

      //msg_len = min(temp_len, DMA_BUFFER);

      if(temp_len > DMA_BUFFER)
        msg_len = DMA_BUFFER;
      else
        msg_len = temp_len;
      
      
      data_details.addr = temp_addr;
      data_details.length = msg_len;
      data_details.isMapped = isMapped;

      int ret = ioctl(cdev, IOCTL_DECRYPT, &data_details);
      if(ret)
      {
          printf("Decryption failed ret : %d\n",ret);
          return ERROR;
      }

      temp_addr = temp_addr+DMA_BUFFER;
      temp_len = temp_len-DMA_BUFFER;


  }
  return 0;
  //return ERROR;
}

/*Function template to set the key pair.
Takes three arguments
  cdev: opened device handle
  a: value of key component a
  b: value of key component b
Return 0 in case of key is set successfully*/
int set_key(DEV_HANDLE cdev, KEY_COMP a, KEY_COMP b)
{

  key_d.a = a;
  key_d.b = b;
  // printf("fskjhfasj \n");
  int ret = ioctl(cdev, IOCTL_SETKEYS,&key_d); 
  //printf("SET KEYS");
  if(ret)
  {
      printf("SET KEY failed ret : %d\n",ret);
      return ERROR;
  }
  // printf("fskjhfasj \n");
  return 0;
  // return ERROR;
}

/*Function template to set configuration of the device to operate.
Takes three arguments
  cdev: opened device handle
  type: type of configuration, i.e. set/unset DMA operation, interrupt
  value: SET/UNSET to enable or disable configuration as described in type
Return 0 in case of key is set successfully*/
int set_config(DEV_HANDLE cdev, config_t type, uint8_t value)
{
  printf("In set config\n");
  config_d.type = type;
  config_d.value = value;
  int ret = ioctl(cdev, IOCTL_SETCONFIG ,&config_d); 
  //printf("SET Config");
  if(ret)
  {
      printf("Config failed ret : %d\n",ret);
      return ERROR;
  }
  return 0;
  //return ERROR;
}

/*Function template to device input/output memory into user space.
Takes three arguments
  cdev: opened device handle
  size: amount of memory-mapped into user-space (not more than 1MB strict check)
Return virtual address of the mapped memory*/
ADDR_PTR map_card(DEV_HANDLE cdev, uint64_t size)
{   
  unsigned long memsize;
    if(size>(SIZE*SIZE)){
         printf("user map can't done\n");
         return NULL;
     }
     memsize = size + 0xa8;
     if(memsize<(1024*1024))
	  memsize = 1024*1024;

     char *ubuf = mmap(NULL, memsize, PROT_WRITE|PROT_READ, MAP_SHARED, cdev, 0);
     if(ubuf == MAP_FAILED){
         printf("mmap failed\n");
         return NULL;
     }

     ubuf += 0xa8;
     return (void*)ubuf;
}

/*Function template to device input/output memory into user space.
Takes three arguments
  cdev: opened device handle
  addr: memory-mapped address to unmap from user-space*/
void unmap_card(DEV_HANDLE cdev, ADDR_PTR addr)
{
  munmap(addr-0xa8, SIZE*SIZE);
}
