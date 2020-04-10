#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/device.h>

#include <linux/io.h> //iowrite ioread
#include <linux/slab.h>//kmalloc kfree
#include <linux/platform_device.h>//platform driver
#include <linux/of.h>//of_match_table
#include <linux/ioport.h>//ioremap

#include <linux/dma-mapping.h>  //dma access
#include <linux/mm.h>  //dma access
#include <linux/interrupt.h>  //interrupt handlers

MODULE_AUTHOR ("FTN");
MODULE_DESCRIPTION("Test Driver for VGA controller IP.");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_ALIAS("custom:vga_dma controller");

#define BUFF_SIZE 100
#define DEVICE_NAME "vga_dma"
#define DRIVER_NAME "vga_dma_driver"
#define MAX_PKT_LEN 640*480*4
#define MAX_L 639
#define MAX_H 479

#include "letters.h"

//*******************FUNCTION PROTOTYPES************************************
static int vga_dma_probe(struct platform_device *pdev);
static int vga_dma_open(struct inode *i, struct file *f);
static int vga_dma_close(struct inode *i, struct file *f);
static ssize_t vga_dma_read(struct file *f, char __user *buf, size_t len, loff_t *off);
static ssize_t vga_dma_write(struct file *f, const char __user *buf, size_t length, loff_t *off);
static ssize_t vga_dma_mmap(struct file *f, struct vm_area_struct *vma_s);
static int __init vga_dma_init(void);
static void __exit vga_dma_exit(void);
static int vga_dma_remove(struct platform_device *pdev);

static irqreturn_t dma_isr(int irq,void*dev_id);
int dma_init(void __iomem *base_address);
u32 dma_simple_write(dma_addr_t TxBufferPtr, u32 max_pkt_len, void __iomem *base_address); 

//*********************GLOBAL VARIABLES*************************************
struct vga_dma_info {
  unsigned long mem_start;
  unsigned long mem_end;
  void __iomem *base_addr;
  int irq_num;
};

static struct cdev *my_cdev;
static dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct vga_dma_info *vp = NULL;

static struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = vga_dma_open,
	.release = vga_dma_close,
	.read = vga_dma_read,
	.write = vga_dma_write,
	.mmap = vga_dma_mmap
};

static struct of_device_id vga_dma_of_match[] = {
	{ .compatible = "xlnx,axi-dma-mm2s-channel", },
	{ .compatible = "vga_dma"},
	{ /* end of list */ },
};
MODULE_DEVICE_TABLE(of, vga_dma_of_match);

static struct platform_driver vga_dma_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table	= vga_dma_of_match,
	},
	.probe		= vga_dma_probe,
	.remove	= vga_dma_remove,
};


dma_addr_t tx_phy_buffer;
u32 *tx_vir_buffer;
static bool (*b_ptr)[7][5] = NULL;
static u32 small_letter[7][5] = {{0}};
static u32 big_letter[14][10] = {{0}};

//***************************************************************************
// PROBE AND REMOVE
static int vga_dma_probe(struct platform_device *pdev)
{
	struct resource *r_mem;
	int rc = 0;

	printk(KERN_INFO "vga_dma_probe: Probing\n");
	// Get phisical register adress space from device tree
	r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r_mem) {
		printk(KERN_ALERT "vga_dma_probe: Failed to get reg resource\n");
		return -ENODEV;
	}
	// Get memory for structure vga_dma_info
	vp = (struct vga_dma_info *) kmalloc(sizeof(struct vga_dma_info), GFP_KERNEL);
	if (!vp) {
		printk(KERN_ALERT "vga_dma_probe: Could not allocate memory for structure vga_dma_info\n");
		return -ENOMEM;
	}
	// Put phisical adresses in timer_info structure
	vp->mem_start = r_mem->start;
	vp->mem_end = r_mem->end;

	// Reserve that memory space for this driver
	if (!request_mem_region(vp->mem_start,vp->mem_end - vp->mem_start + 1, DRIVER_NAME))
	{
		printk(KERN_ALERT "vga_dma_probe: Could not lock memory region at %p\n",(void *)vp->mem_start);
		rc = -EBUSY;
		goto error1;
	}    
	// Remap phisical to virtual adresses

	vp->base_addr = ioremap(vp->mem_start, vp->mem_end - vp->mem_start + 1);
	if (!vp->base_addr) {
		printk(KERN_ALERT "vga_dma_probe: Could not allocate memory for remapping\n");
		rc = -EIO;
		goto error2;
	}

	// Get irq num 
	vp->irq_num = platform_get_irq(pdev, 0);
	if(!vp->irq_num)
	{
		printk(KERN_ERR "vga_dma_probe: Could not get IRQ resource\n");
		rc = -ENODEV;
		goto error2;
	}

	if (request_irq(vp->irq_num, dma_isr, 0, DEVICE_NAME, NULL)) {
		printk(KERN_ERR "vga_dma_probe: Could not register IRQ %d\n", vp->irq_num);
		return -EIO;
		goto error3;
	}
	else {
		printk(KERN_INFO "vga_dma_probe: Registered IRQ %d\n", vp->irq_num);
	}

	/* INIT DMA */
	dma_init(vp->base_addr);
	dma_simple_write(tx_phy_buffer, MAX_PKT_LEN, vp->base_addr); // helper function, defined later

	printk(KERN_NOTICE "vga_dma_probe: VGA platform driver registered\n");
	return 0;//ALL OK

error3:
	iounmap(vp->base_addr);
error2:
	release_mem_region(vp->mem_start, vp->mem_end - vp->mem_start + 1);
	kfree(vp);
error1:
	return rc;

}

static int vga_dma_remove(struct platform_device *pdev)
{
	u32 reset = 0x00000004;
	// writing to MM2S_DMACR register. Seting reset bit (3. bit)
	printk(KERN_INFO "vga_dma_probe: resseting");
	iowrite32(reset, vp->base_addr); 

	free_irq(vp->irq_num, NULL);
	iounmap(vp->base_addr);
	release_mem_region(vp->mem_start, vp->mem_end - vp->mem_start + 1);
	kfree(vp);
	printk(KERN_INFO "vga_dma_probe: VGA DMA removed");
	return 0;
}

//***************************************************
// IMPLEMENTATION OF FILE OPERATION FUNCTIONS
static int vga_dma_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO "vga_dma opened\n");
	return 0;
}

static int vga_dma_close(struct inode *i, struct file *f)
{
	printk(KERN_INFO "vga_dma closed\n");
	return 0;
}

static ssize_t vga_dma_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	//printk("vga_dma read\n");
	return 0;
}

static int choose_letter(const char letter)
{
    if((letter >= 'A' && letter <= 'Z') || (letter >= 'a' && letter <= 'z') || 
        letter != ' ' || letter != '!' || letter != ',' || letter != '?' || letter != '.')
    {   
        return -1;
    }
    if(letter == 'A')
        b_ptr = &A;

    return 0;
}

static void doubleSizeMat(void)
{
	int i,j,c,d;
	for(i=0;i<7;i++)
		for(j=0;j<5;j++)
			for(c=0;c<2;c++)
				for(d=0;d<2;d++)
					big_letter[2*i+c][2*j+d] = small_letter[i][j];
}

static void assign(bool set_big_letter, const bool (*letter)[5], const u32 col_let, const u32 col_bckg)
{
	int i,j;
	for(i=0;i<7;i++)
		for(j=0;j<5;j++)
			small_letter[i][j] = (letter[i][j] == 1) ? col_bckg : col_let; 

	if(set_big_letter)
    {
		doubleSizeMat();
    }
}

static unsigned int strToInt(const char* str)
{
    int i;
    int dec=1;
    unsigned int val=0;
    for(i=strlen(str)-1; i>=0; --i)
    {
        int a = (str[i]-48)*dec;
        dec *= 10;
        val += a;
    }
    return val;
}

static void assign_commands(const char* buff, char (*commands)[BUFF_SIZE])
{
    int i;
	int incr=0;
	int len = 0;
	for(i=0;i<strlen(buff);++i)
	{
		if(buff[i] != ',')
			commands[incr][i-len] = buff[i];
		else if(buff[i] == ',')
		{
			len += strlen(commands[incr]) + 1;
			incr++;
		}
	}
}

static int print_letter(const bool big_font, const unsigned int x_startPos, const unsigned int y_startPos)
{
	int step_x, step_y;
	int x,y,i,j;
	if(big_font)
	{
		step_x = 10;
		step_y = 14;
	}
	else
	{
		step_x = 5;
		step_y = 7;
	}
	if ((x_startPos + step_x > MAX_L) || (y_startPos + step_y > MAX_H) )
	{
		printk(KERN_ERR "VGA_write:\nX_axis position can be: [0,639]\nY_axis position can be: [0,479]\n");
		return -1;
	}
	for(y=y_startPos,i=0;y<y_startPos+step_y;++i,++y)
		for(x=x_startPos,j=0;x<x_startPos+step_x;++j,++x)
		{
			u32 rgb = (big_font == true) ? big_letter[i][j] : small_letter[i][j];
			tx_vir_buffer[640*y + x] = rgb;
		}
	return 0;
	
}

static int commands(const char (*commands)[BUFF_SIZE] )
{
	if(strlen(commands[0]) != 4)
		return -EINVAL;
	if(!strcmp(commands[0],"CHAR") || !strcmp(commands[0],"TEXT") )
	{
		bool big_font;
		unsigned int x_startPos, y_startPos; 
		unsigned long long rgb_text, rgb_bckg;

		big_font = (!strcmp(commands[2],"big")) ? true : false;
		if(!big_font && !strcmp(commands[2],"small") )
			return -EINVAL;

		x_startPos=strToInt(commands[3]);
		y_startPos=strToInt(commands[4]);
		
		kstrtoull(commands[5], 0, &rgb_text);
		kstrtoull(commands[6], 0, &rgb_bckg);

		if( choose_letter(commands[1]) == -1 )
			return -EINVAL;
		
		assign(big_font, *b_ptr, rgb_text, rgb_bckg);
		print_letter(big_font, (u32)x_startPos, (u32)y_startPos);

	}
	return 0;

}

static ssize_t vga_dma_write(struct file *f, const char __user *buf, size_t length, loff_t *off)
{	
	char buff[BUFF_SIZE];
	int ret = 0;
	
	ret = copy_from_user(buff, buf, length);  
	if(ret){
		printk("copy from user failed \n");
		return -EFAULT;
	}  
	buff[length] = 0;

	char commands_[7][BUFF_SIZE] = {{0}};
	assign_commands(buff, commands_);
	ret = commands(commands_);

	if(ret == -EINVAL)
		return -EINVAL;      
	return length;
}

static ssize_t vga_dma_mmap(struct file *f, struct vm_area_struct *vma_s)
{
	int ret = 0;
	long length = vma_s->vm_end - vma_s->vm_start;

	//printk(KERN_INFO "DMA TX Buffer is being memory mapped\n");

	if(length > MAX_PKT_LEN)
	{
		return -EIO;
		printk(KERN_ERR "Trying to mmap more space than it's allocated\n");
	}

	ret = dma_mmap_coherent(NULL, vma_s, tx_vir_buffer, tx_phy_buffer, length);
	if(ret<0)
	{
		printk(KERN_ERR "memory map failed\n");
		return ret;
	}
	return 0;
}

/****************************************************/
// IMPLEMENTATION OF DMA related functions

static irqreturn_t dma_isr(int irq,void*dev_id)
{
	u32 IrqStatus;  
	/* Read pending interrupts */
	IrqStatus = ioread32(vp->base_addr + 4);//read irq status from MM2S_DMASR register
	iowrite32(IrqStatus | 0x00007000, vp->base_addr + 4);//clear irq status in MM2S_DMASR register
	//(clearing is done by writing 1 on 13. bit in MM2S_DMASR (IOC_Irq)

	/*Send a transaction*/
	dma_simple_write(tx_phy_buffer, MAX_PKT_LEN, vp->base_addr); //My function that starts a DMA transaction
	return IRQ_HANDLED;;
}

int dma_init(void __iomem *base_address)
{
	u32 reset = 0x00000004;
	u32 IOC_IRQ_EN; 
	u32 ERR_IRQ_EN;
	u32 MM2S_DMACR_reg;
	u32 en_interrupt;

	IOC_IRQ_EN = 1 << 12; // this is IOC_IrqEn bit in MM2S_DMACR register
	ERR_IRQ_EN = 1 << 14; // this is Err_IrqEn bit in MM2S_DMACR register

	iowrite32(reset, base_address); // writing to MM2S_DMACR register. Seting reset bit (3. bit)

	MM2S_DMACR_reg = ioread32(base_address); // Reading from MM2S_DMACR register inside DMA
	en_interrupt = MM2S_DMACR_reg | IOC_IRQ_EN | ERR_IRQ_EN;// seting 13. and 15.th bit in MM2S_DMACR
	iowrite32(en_interrupt, base_address); // writing to MM2S_DMACR register  
	return 0;
}

u32 dma_simple_write(dma_addr_t TxBufferPtr, u32 max_pkt_len, void __iomem *base_address) {
	u32 MM2S_DMACR_reg;

	MM2S_DMACR_reg = ioread32(base_address); // READ from MM2S_DMACR register

	iowrite32(0x1 |  MM2S_DMACR_reg, base_address); // set RS bit in MM2S_DMACR register (this bit starts the DMA)

	iowrite32((u32)TxBufferPtr, base_address + 24); // Write into MM2S_SA register the value of TxBufferPtr.
	// With this, the DMA knows from where to start.

	iowrite32(max_pkt_len, base_address + 40); // Write into MM2S_LENGTH register. This is the length of a tranaction.
	// In our case this is the size of the image (640*480*4)
	return 0;
}



//***************************************************
// INIT AND EXIT FUNCTIONS OF THE DRIVER

static int __init vga_dma_init(void)
{

	int ret = 0;
	int i = 0;

	printk(KERN_INFO "vga_dma_init: Initialize Module \"%s\"\n", DEVICE_NAME);
	ret = alloc_chrdev_region(&my_dev_id, 0, 1, "VGA_region");
	if (ret)
	{
		printk(KERN_ALERT "vga_dma_init: Failed CHRDEV!\n");
		return -1;
	}
	printk(KERN_INFO "vga_dma_init: Successful CHRDEV!\n");
	my_class = class_create(THIS_MODULE, "VGA_drv");
	if (my_class == NULL)
	{
		printk(KERN_ALERT "vga_dma_init: Failed class create!\n");
		goto fail_0;
	}
	printk(KERN_INFO "vga_dma_init: Successful class chardev1 create!\n");
	my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id),0), NULL, "vga_dma");
	if (my_device == NULL)
	{
		goto fail_1;
	}

	printk(KERN_INFO "vga_dma_init: Device created\n");

	my_cdev = cdev_alloc();	
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 1);
	if (ret)
	{
		printk(KERN_ERR "vga_dma_init: Failed to add cdev\n");
		goto fail_2;
	}
	printk(KERN_INFO "vga_dma_init: Module init done\n");

	tx_vir_buffer = dma_alloc_coherent(NULL, MAX_PKT_LEN, &tx_phy_buffer, GFP_DMA | GFP_KERNEL);
	if(!tx_vir_buffer){
		printk(KERN_ALERT "vga_dma_init: Could not allocate dma_alloc_coherent for img");
		goto fail_3;
	}
	else
		printk("vga_dma_init: Successfully allocated memory for dma transaction buffer\n");
	for (i = 0; i < MAX_PKT_LEN/4;i++)
		tx_vir_buffer[i] = 0x00000000;
	printk(KERN_INFO "vga_dma_init: DMA memory reset.\n");
	return platform_driver_register(&vga_dma_driver);

fail_3:
	cdev_del(my_cdev);
fail_2:
	device_destroy(my_class, MKDEV(MAJOR(my_dev_id),0));
fail_1:
	class_destroy(my_class);
fail_0:
	unregister_chrdev_region(my_dev_id, 1);
	return -1;

} 

static void __exit vga_dma_exit(void)  		
{
	//Reset DMA memory
	int i =0;
	for (i = 0; i < MAX_PKT_LEN/4; i++) 
		tx_vir_buffer[i] = 0x00000000;
	printk(KERN_INFO "vga_dma_exit: DMA memory reset\n");

	// Exit Device Module
	platform_driver_unregister(&vga_dma_driver);
	cdev_del(my_cdev);
	device_destroy(my_class, MKDEV(MAJOR(my_dev_id),0));
	class_destroy(my_class);
	unregister_chrdev_region(my_dev_id, 1);
	dma_free_coherent(NULL, MAX_PKT_LEN, tx_vir_buffer, tx_phy_buffer);
	printk(KERN_INFO "vga_dma_exit: Exit device module finished\"%s\".\n", DEVICE_NAME);
}

module_init(vga_dma_init);
module_exit(vga_dma_exit);

