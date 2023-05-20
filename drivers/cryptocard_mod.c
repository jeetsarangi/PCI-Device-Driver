#include<linux/module.h>
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
#include <asm/tlbflush.h>
#include<linux/uaccess.h>
#include<linux/device.h>
#include <linux/kthread.h> 
#include <linux/delay.h>
#include <linux/vmalloc.h>

#include "driver.h"




/////////////////////////PCI FUNCTIONS//////////////////////////////////////


MODULE_DEVICE_TABLE(pci, my_driver_id_table);

struct task_node{
        int tid ;
        int dma ;
        int intr ;
        int ismapped ;
        int decrypt ;
        KEY_COMP a;
        KEY_COMP b;
        struct  task_node *next;
};




static int my_driver_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
static int check_liveness(struct pci_dev *pdev,u8 __iomem *MEM);
void release_device(struct pci_dev *pdev);
static void my_driver_remove(struct pci_dev *pdev);
static irqreturn_t Interupt_handler(int irq, void *data);
void set_mmio_dataddr(void);
void mmio(struct task_node *cur,char * message);
void set_keys(uint8_t a, uint8_t b);
static void dma(struct task_node *cur,char * message);
void add_node(pid_t tid);
static struct task_node *search(pid_t tid);
void remove_node(pid_t tid);
static int device_mmap(struct file *filp, struct vm_area_struct *vma);

DECLARE_WAIT_QUEUE_HEAD(wq);


struct my_driver_priv {
    u8 __iomem *hwmem;
};

struct my_driver_priv *drv_priv;

static struct pci_driver my_driver = {
    .name = MY_DRIVER,
    .id_table = my_driver_id_table,
    .probe = my_driver_probe,
    .remove = my_driver_remove
};


unsigned long mmio_start,mmio_len;
struct task_node *head = NULL;

//////////////////////////PCI DECLARATIONS//////////////////////////////////

static int major;
atomic_t  device_opened;
static struct class *demo_class;
struct device *demo_device;


void remove_node(pid_t tid){

        
        struct task_node * iter, *temp;
        mutex_lock(&cs614_lock);
        iter = head;
        if(iter->tid == tid){
                temp = head;
                head = head->next;
        }else{
                while(iter){
                        if(iter->next->tid == tid){
                                temp = iter->next;
                                iter->next = iter->next->next;
                                break;
                        }
                }
                
        }
        kfree(temp);

        mutex_unlock(&cs614_lock);

}

static struct task_node *search(pid_t tid){

        // mutex_lock(&cs614_lock);

        struct task_node *iter = head;
        mutex_lock(&cs614_lock);
        while(iter){
                if(iter->tid == tid){
                        mutex_unlock(&cs614_lock);
                        return iter;
                }
                iter = iter->next;
        }

        mutex_unlock(&cs614_lock);

        return NULL;
        
}


void add_node(pid_t tid){

        struct task_node* new;
        mutex_lock(&cs614_lock);
        new = kmalloc(sizeof(struct task_node),GFP_KERNEL);
        new->tid = tid;
        new->dma = UNSET;
        new->intr = UNSET;
        new->decrypt = UNSET;
        new->ismapped = UNSET;
        new->next = head;
        head = new;
        mutex_unlock(&cs614_lock);
}




static int device_open(struct inode *inode, struct file *file)
{
        add_node(current->tgid);
        atomic_inc(&device_opened);
        try_module_get(THIS_MODULE);
        printk(KERN_INFO "Device opened successfully\n");
        return 0;
}

static int device_release(struct inode *inode, struct file *file)
{       
        remove_node(current->tgid);
        atomic_dec(&device_opened);
        module_put(THIS_MODULE);
        
        printk(KERN_INFO "Device closed successfully\n");

        return 0;
}


static ssize_t device_read(struct file *filp,
                           char *buffer,
                           size_t length,
                           loff_t * offset){
    printk(KERN_INFO "read called\n");
    return 0;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off){
    
    printk(KERN_INFO "write called\n");
    return 8;
}

static int device_mmap(struct file *filp, struct vm_area_struct *vma){

        unsigned long size;
        int ret;
	pr_info("Running device mmap");


        size = vma->vm_end - vma->vm_start;
        ret = io_remap_pfn_range(vma, vma->vm_start, mmio_start >> PAGE_SHIFT, size, vma->vm_page_prot);
        
        if(ret>0){
        printk(KERN_ERR"remap pfn range error\n");
        return -EAGAIN;
        }


	return 0;
}


long device_ioctl(struct file *file,	
		 unsigned int ioctl_num,
		 unsigned long ioctl_param)
{
	
	int ret = 0;
        struct encrypt_data *data_details = NULL;
        struct key_data *key_d = NULL;
        struct config_data *config_d = NULL;
        char * message = NULL;
        struct task_node* curr = search(current->tgid);
        //u64 message_length;
        //char *message_addr;
	/*
	 * Switch according to the ioctl called
	 */
	switch (ioctl_num) {
	
	case IOCTL_ENCRYPT:
                mutex_lock(&cs614_lock);
		data_details = (struct encrypt_data*)vmalloc(sizeof(struct encrypt_data));
		if(copy_from_user(data_details,(char*)ioctl_param,sizeof(struct encrypt_data))){
                pr_err("ENCRYPT address write error\n");
                return -1;
            }
                message = kzalloc(data_details->length, GFP_KERNEL);
                
                curr->decrypt = UNSET;
                curr->ismapped = data_details->isMapped;


                if(curr->ismapped == SET){
                        curr->dma = UNSET;
                }
                

                if(curr->ismapped == UNSET){
                if(copy_from_user(message,(char*)data_details->addr,data_details->length)){
                        pr_err("Copy message failed");
                        return -1;
                }}
                
                if(!(curr->dma))
                        mmio(curr,message);
                else
                        dma(curr,message);
                
                if(curr->ismapped == UNSET){
                        //printk("fsdf");
                if(copy_to_user((char*)data_details->addr,message,data_details->length)){
                        pr_info("copied encrypted back successfully!");
                }}


                

                pr_info("We are at Encryption");
		// pr_info("The Encryption Data address is = %p",(data_details->addr));
		// pr_info("The Encryption Data length is = %lu",(unsigned long)(data_details->length));
		// pr_info("The Encryption Data address is = %i",(int)(data_details->isMapped));
                vfree(data_details);
                mutex_unlock(&cs614_lock);
                return ret;
	case IOCTL_DECRYPT:
		
                mutex_lock(&cs614_lock);
		data_details = (struct encrypt_data*)vmalloc(sizeof(struct encrypt_data));
		if(copy_from_user(data_details,(char*)ioctl_param,sizeof(struct encrypt_data))){
                pr_err("ENCRYPT address write error\n");
                return -1;
            }
                message = kzalloc(data_details->length, GFP_KERNEL);
                
                curr->decrypt = SET;
                curr->ismapped = data_details->isMapped;


                if(curr->ismapped == SET){
                        curr->dma = UNSET;
                }

                
                if(curr->ismapped == UNSET){
                if(copy_from_user(message,(char*)data_details->addr,data_details->length)){
                        pr_err("Copy message failed");
                        return -1;
                }}
                
                if(!(curr->dma))
                        mmio(curr,message);
                else
                        dma(curr,message);
                
                if(curr->ismapped == UNSET){
                if(copy_to_user((char*)data_details->addr,message,data_details->length)){
                        pr_info("copied encrypted back successfully!");
                }}

                pr_info("We are at Decryption");
		// pr_info("The Decryption Data address is = %p",(data_details->addr));
		// pr_info("The Decryption Data length is = %lu",(unsigned long)(data_details->length));
		// pr_info("The Decryption Data address is = %i",(int)(data_details->isMapped));
                vfree(data_details);
                mutex_unlock(&cs614_lock);
                return ret;

	case IOCTL_SETKEYS:
		mutex_lock(&cs614_lock);
		key_d = (struct key_data*)vmalloc(sizeof(struct key_data));
		if(copy_from_user(key_d,(char*)ioctl_param,sizeof(struct key_data))){
                pr_err("SET KEY addresses write error\n");
                return -1;
            }

                curr->a = key_d->a;
                curr->b = key_d->b;
                // vfree(key_d);


                pr_info("We are at setkeys");
		pr_info("The SETKEY Data a is = %u",(key_d->a));
		pr_info("The SETKEY Data b is = %u",(key_d->b));

                vfree(key_d);
                mutex_unlock(&cs614_lock);
                pr_info("Returning from setkeys...");
                return ret;
	
	case IOCTL_SETCONFIG:

                mutex_lock(&cs614_lock);
		config_d = (struct config_data*)vmalloc(sizeof(struct config_data));
		if(copy_from_user(config_d,(char*)ioctl_param,sizeof(struct config_data))){
                pr_err("SET KEY addresses write error\n");
                return -1;
                }

                if(config_d->type == INTERRUPT){
                        curr->intr = config_d->value;
                }
                else{
                        curr->dma = config_d->value;
                }


                pr_info("We are at setconfig");
		pr_info("The Config Data type is = %i",(int)(config_d->type));
		pr_info("The Config Data value is = %i",(int)(config_d->value));
                vfree(config_d);
                mutex_unlock(&cs614_lock);
                return ret;
	
	    
	}

	return ret;
}

static void dma(struct task_node *cur,char * message){

        u64 temp = 0;
        int msg_len;
        u32 status_bit = 0x1;
        msg_len = sizeof(message);


        printk(KERN_INFO "DMA processing n");

        //setting the keys
        set_keys(cur->a,cur->b);

        //copy message to dma buff
        memcpy(dma_space,message,msg_len);

        //Seting dma message length
        writeq(msg_len,drv_priv->hwmem + OFFSET_MSGLENDMA);

        //dma handle setting
        writeq(dma_handle,drv_priv->hwmem+OFFSET_DMAADDR);

        //Command value finding
        if(cur->intr) status_bit|=0x4;
        if(cur->decrypt) status_bit|=0x2;
        

        //setting command reg

        writeq(status_bit,drv_priv->hwmem+OFFSET_DMACMDREG);

        if(!(cur->intr)){
                temp = readq(drv_priv->hwmem + OFFSET_DMACMDREG);
                while((temp & (u64)1)==1){
                        temp = readq(drv_priv->hwmem + OFFSET_DMACMDREG);
                        usleep_range(100,200);
                }
        }
        else{
                pr_info("Setting up Interupt DMA");
                wait_event_interruptible(wq,(ioread32(drv_priv->hwmem+OFFSET_DMACMDREG) & (u32)1)==0);
        }

        memcpy(message,dma_space,msg_len);

        return;



}




void set_keys(uint8_t a, uint8_t b){
    // printk("The values of a and b are: %u,%u\n",a,b);
    int val = (a << 8) | b;
    // g_a = a;
    // g_b = b;
    iowrite32(val,drv_priv->hwmem+OFFSET_SETKEYS);
    return;
}

void set_mmio_dataddr(void){
        writeq(OFFSET_LASTADDR,drv_priv->hwmem+OFFSET_DATAADDRMMIO);
        return;
}


void mmio(struct task_node *cur,char * message){


        u32 temp = 0;
        int msg_len;
        u32 status_bit = 0x0;
        //char *buf;
        msg_len = sizeof(message);

        pr_info("Processing MMio");
        pr_info("MSG len = %i",msg_len);

        set_keys(cur->a,cur->b);

        //setting message length
        iowrite32(msg_len,drv_priv->hwmem+OFFSET_LOMMMIO);


        //copy the message
        if(cur->ismapped == UNSET)
        memcpy(drv_priv->hwmem+OFFSET_LASTADDR,message,msg_len);


        if(cur->decrypt)
                status_bit |= (0x02);
        if(cur->intr) 
                status_bit |= 0x80;

        iowrite32(status_bit, drv_priv->hwmem + OFFSET_STATUSMMIO);

        //mmio without interupt
        set_mmio_dataddr();

        if(cur->intr == UNSET)
        {
        temp = ioread32(drv_priv->hwmem+OFFSET_STATUSMMIO);
        while((temp & 1)){
            temp = ioread32(drv_priv->hwmem+OFFSET_STATUSMMIO);
        }
        }
        else{
                pr_info("Setting up Interupt MMIO");
                wait_event_interruptible(wq,(ioread32(drv_priv->hwmem+OFFSET_STATUSMMIO) & (u32)1)==0);
        }

        //copy back message
        if(cur->ismapped == UNSET)
        memcpy(message,drv_priv->hwmem+(OFFSET_LASTADDR),msg_len);

        return;


}


static struct file_operations fops = {
        .read = device_read,
        .write = device_write,
	.unlocked_ioctl = device_ioctl,
        .open = device_open,
        .release = device_release,
        .mmap = device_mmap
};

static char *demo_devnode(struct device *dev, umode_t *mode)
{
        if (mode && dev->devt == MKDEV(major, 0))
                *mode = 0666;
        return NULL;
}


int init_module(void)
{
        int err;
	printk(KERN_INFO "Hello kernel\n");

        err = pci_register_driver(&my_driver);
        if(err < 0)
        {
                printk(KERN_ERR "PCI Driver resgister failed!!");
                return err;
        }

        major = register_chrdev(0, DEVNAME, &fops);
        err = major;
        if (err < 0) {      
             printk(KERN_ALERT "Registering char device failed with %d\n", major);   
             goto error_regdev;
        }                 
        
        demo_class = class_create(THIS_MODULE, DEVNAME);
        err = PTR_ERR(demo_class);
        if (IS_ERR(demo_class))
                goto error_class;

        demo_class->devnode = demo_devnode;

        demo_device = device_create(demo_class, NULL,
                                        MKDEV(major, 0),
                                        NULL, DEVNAME);
        err = PTR_ERR(demo_device);
        if (IS_ERR(demo_device))
                goto error_device;
 
        printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);                                                              
        atomic_set(&device_opened, 0);
        pr_info("The chardev is created !!");
	mutex_init(&cs614_lock);
	return 0;
   
error_device:
         class_destroy(demo_class);
error_class:
        unregister_chrdev(major, DEVNAME);
error_regdev:
        return  err;
}

static irqreturn_t Interupt_handler(int irq, void *data){


        u32 status;

        pr_info("In Interupt Handler \n");

        status = ioread32(drv_priv->hwmem + OFFSET_STATUSINT);
        if(status == MMIO_INTRUPT || status == DMA_INTRUPT){

                iowrite32(status, drv_priv->hwmem + OFFSET_INTACK);
                pr_info("The device raised the interupt with IRQ: %d ",irq);

                wake_up_interruptible(&wq);

                return IRQ_HANDLED;
                
        }

        return IRQ_NONE;





}



static int my_driver_probe(struct pci_dev *pdev, const struct pci_device_id *ent){


        int bar, err;
        u16 vendor, device;
        // unsigned long mmio_start,mmio_len;
        // struct my_driver_priv *drv_priv;

        pci_read_config_word(pdev, PCI_VENDOR_ID, &vendor);
        pci_read_config_word(pdev, PCI_DEVICE_ID, &device);

        printk(KERN_INFO "Device vid: 0x%X pid: 0x%X\n", vendor, device);


        bar = pci_select_bars(pdev, IORESOURCE_MEM);

        err = pci_enable_device_mem(pdev);

        if (err) {
                return err;
        }

        err = pci_request_region(pdev, bar, MY_DRIVER);

        if (err) {
                pci_disable_device(pdev);
                return err;
        }

        mmio_start = pci_resource_start(pdev, 0);
        mmio_len = pci_resource_len(pdev, 0);

        drv_priv = kzalloc(sizeof(struct my_driver_priv), GFP_KERNEL);

        if (!drv_priv) {
                release_device(pdev);
                return -ENOMEM;
        }

        drv_priv->hwmem = ioremap(mmio_start, mmio_len);
        // hwmem = drv_priv->hwmem;

        if (!drv_priv->hwmem) {
        release_device(pdev);
        return -EIO;
        }
        //buff_dma = dma_alloc_coherent(&(pdev->dev), DMA_BUFF_SIZE, &dma_handle, GFP_ATOMIC);

        printk(KERN_INFO "Identification = 0x%X",ioread32(drv_priv->hwmem));
        check_liveness(pdev,drv_priv->hwmem);


        dma_space = dma_alloc_coherent(&(pdev->dev), DMA_BUFF_SIZE, &dma_handle, GFP_ATOMIC);
        if(dma_space==NULL){
                printk(KERN_INFO "Failed to allocate dma space\n");
                return -EIO;
        }


        if(pdev->irq){
	 //int ret;
	 unsigned int req = pdev->irq;
	 printk(KERN_INFO"IRQ : %u\n",req);
	 //ret = devm_request_irq(&pdev->dev ,req, pci_irq_handler, IRQF_SHARED, DEVNAME, drv_priv);
	 if((devm_request_irq(&pdev->dev ,req, Interupt_handler, IRQF_SHARED, MY_DRIVER, drv_priv->hwmem))<0){
	    printk(KERN_ERR"Failed to register interrupt handler \n");
	}
        }

        printk(KERN_INFO "The IRQ init also been done for the device on above number if supported !!");


        printk(KERN_INFO "IN Probe done with all PCI Initiation!!");
        return 0;
}

void release_device(struct pci_dev *pdev)
{
    pci_release_region(pdev, pci_select_bars(pdev, IORESOURCE_MEM));
    pci_disable_device(pdev);
}

static void my_driver_remove(struct pci_dev *pdev)
{
   dma_free_coherent(&pdev->dev, DMA_BUFF_SIZE, dma_space, dma_handle);
    
    if(drv_priv->hwmem)
        iounmap(drv_priv->hwmem);

    release_device(pdev);
    pr_info("Remove done\n");
}

static int check_liveness(struct pci_dev *pdev,u8 __iomem *MEM)
{
    u32 num_device;
    u32 num = 0x65485BAF;
    iowrite32(num, MEM + OFFSET_LIVENESS);
    num_device = ioread32(MEM + OFFSET_LIVENESS);
    if (num_device != ~num)
    {
        pr_err("CRYPTOCARD_MOD ---> liveness check failed, num 0x%X ~num 0x%X device 0x%X\n", num, ~num, num_device);
        return -1;
    }
    pr_info("CRYPTOCARD_MOD ---> liveness check successful\n");
    return 0;
}

void cleanup_module(void)
{
        device_destroy(demo_class, MKDEV(major, 0));
        class_destroy(demo_class);
        unregister_chrdev(major, DEVNAME);
        pci_unregister_driver(&my_driver);
	printk(KERN_INFO "Goodbye kernel Getting Out of the Machine \n");
}

MODULE_AUTHOR("Jeet 21111032");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Assignment 3");
