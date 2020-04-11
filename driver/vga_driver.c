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

#include "letters.h"

MODULE_AUTHOR ("FTN");
MODULE_DESCRIPTION("Test Driver for VGA controller IP.");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_ALIAS("custom:vga_dma controller");

#define DEVICE_NAME "vga_dma"
#define DRIVER_NAME "vga_dma_driver"
#define BUFF_SIZE 50
#define MAX_PKT_LEN 640*480*4
#define MAX_W 639
#define MAX_H 479

#define BIG_FONT_W 10
#define BIG_FONT_H 14
#define SMALL_FONT_W 5
#define SMALL_FONT_H 7

typedef int state_t;
enum {state_TEXT, state_LINE, state_RECT, state_CIRC, state_PIX, state_ERR};

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
static const bool((* b_ptr)[7][5]) = NULL;
static unsigned long long small_letter[7][5] = {{0}};
static unsigned long long big_letter[14][10] = {{0}};

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
	printk(KERN_ERR "vga_dma opened\n");
	return 0;
}

static int vga_dma_close(struct inode *i, struct file *f)
{
	printk(KERN_ERR "vga_dma closed\n");
	return 0;
}

static ssize_t vga_dma_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	printk(KERN_ERR "vga_dma read\n");
	return 0;
}

static void print_pix(const bool big_font, const unsigned int x_StartPos, const unsigned int y_StartPos, const unsigned int x_Step, const unsigned int y_Step)
{
	unsigned int x,y,i,j;
	
	for(y=y_StartPos, i=0; i<y_Step; ++i,++y)
		for(x=x_StartPos, j=0; j<x_Step; ++x,++j)
		{
			u32 rgb = (big_font == true) ? ((u32)big_letter[i][j]) : ((u32)small_letter[i][j]);
			tx_vir_buffer[640*y + x] = rgb;
		}
}

static void parse_buffer(const char* buffer, char(* commands)[BUFF_SIZE])
{
	int i, incr=0, len=0;
	for(i=0;i<strlen(buffer);i++)
	{
		if(buffer[i] != ',' && buffer[i] != '\n')
			commands[incr][i-len] = buffer[i];
		else if(buffer[i] == ',')
		{
			len += strlen(commands[incr]) + 1;
			incr++;
		}
		else if(buffer[i] == '\n')
		{
			break;
		}
	}
}

static unsigned int strToInt(const char* string_num)
{
	int i,dec=1;
	unsigned int val=0;
	for(i=strlen(string_num)-1;i>=0;--i)
	{
		unsigned int tmp = (string_num[i]-48)*dec;
		dec *= 10;
		val += tmp;
	}
	return val;
}

static state_t getState(const char* command0)
{
	if(!strcmp(command0,"TEXT"))
		return state_TEXT;
	else if(!strcmp(command0,"LINE"))
		return state_LINE;
	else if(!strcmp(command0,"RECT"))
		return state_RECT;
	else if(!strcmp(command0,"CIRC"))
		return state_CIRC;
	else if(!strcmp(command0,"PIX"))
		return state_PIX;
	return state_ERR;
}

struct Text
{
	char m_Letters[BUFF_SIZE];
	bool m_BigFont;
	unsigned int m_Xstart, m_Ystart;
	unsigned long long m_ColorLetter, m_ColorBckg;
};

void initTextStruct(struct Text* text)
{
	int i;
	for(i=0;i<BUFF_SIZE;++i)
		text->m_Letters[i] = 0;
	text->m_BigFont = false;
	text->m_Xstart = 0, text->m_Ystart=0;
	text->m_ColorLetter=0, text->m_ColorBckg=0;
}

struct Text getText(const char(* commands)[BUFF_SIZE])
{
	int i;
	struct Text text;
	initTextStruct(&text);
	for(i=0;i<strlen(commands[1]);++i)
		text.m_Letters[i] = commands[1][i];
	text.m_BigFont = (!strcmp(commands[2],"big")) ? true : false;
	text.m_Xstart = strToInt(commands[3]);
	text.m_Ystart = strToInt(commands[4]);
	i=kstrtoull((unsigned char*)commands[5],0,&text.m_ColorLetter);
	i=kstrtoull((unsigned char*)commands[6],0,&text.m_ColorBckg);
	return text;
}

static int check_character(const char letter)
{
	if( !(letter >= 'A' && letter <= 'Z') && !(letter >= 'a' && letter <= 'z') &&
		letter != ' ' && letter != '!' && letter != ',' && letter != '?' && letter != '.')
	{
		return -1;
	}
	return 0;
}

static void set_character(const char letter, const bool(** ptr)[7][5])
{
	if(letter == 'A')
		*ptr = &A;
	else if(letter == 'B')
		*ptr = &B;
	else if(letter == 'C')
		*ptr = &C;
	else if(letter == 'D')
		*ptr = &D;
	else if(letter == 'E')
		*ptr = &E;
	else if(letter == 'F')
		*ptr = &F;
	else if(letter == 'G')
		*ptr = &G;
	else if(letter == 'H')
		*ptr = &H;
	else if(letter == 'I')
		*ptr = &I;
	else if(letter == 'J')
		*ptr = &J;
	else if(letter == 'K')
		*ptr = &K;
	else if(letter == 'L')
		*ptr = &L;
	else if(letter == 'M')
		*ptr = &M;
	else if(letter == 'N')
		*ptr = &N;
	else if(letter == 'O')
		*ptr = &O;
	else if(letter == 'P')
		*ptr = &P;
	else if(letter == 'Q')
		*ptr = &Q;
	else if(letter == 'R')
		*ptr = &R;
	else if(letter == 'S')
		*ptr = &S;
	else if(letter == 'T')
		*ptr = &T;
}

static void DoubleSizeMat(void)
{
	int i,j,c,d;
	for(i=0;i<7;++i)
		for(j=0;j<5;j++)
			for(c=0;c<2;c++)
				for(d=0;d<2;d++)
					big_letter[2*i+c][2*j+d]=small_letter[i][j];
}

static void assignValToLetter(const bool big_letter, const unsigned long long color_letter, const unsigned long long color_bckg)
{
	int i,j;
	for(i=0;i<7;++i)
		for(j=0;j<5;++j)
		{
			small_letter[i][j] = ((*b_ptr)[i][j] == 1) ? color_bckg : color_letter;
		}
	if(big_letter)
	{
		DoubleSizeMat();
	}
}

static int printWord(const struct Text text)
{
	unsigned int i, Y = text.m_Ystart, X=text.m_Xstart, strLen = strlen(text.m_Letters),
	x_step = (text.m_BigFont == true) ? BIG_FONT_W : SMALL_FONT_W,
	y_step = (text.m_BigFont == true) ? BIG_FONT_H : SMALL_FONT_H,
	checkX = X + strLen * (x_step + 1) - 1,	checkY = Y + y_step;
	bool error=false;
	for(i=0; i<strLen; ++i)
	{
		if(check_character(text.m_Letters[i]) == -1)
		{
			printk(KERN_ERR "VGA_DMA: %c cant be printed on screen, there's not this character on our library!\n",text.m_Letters[i] );
			error = true;	
		}
	}

	if(checkX > MAX_W || checkY > MAX_H)
	{
		printk(KERN_ERR "VGA_DMA: %s cant whole fit into screen!\n",text.m_Letters);
		error = true;
	}

	if(error)
		return -1;

	for(i=0;i<strLen;++i)
	{
		set_character(text.m_Letters[i], &b_ptr);
		assignValToLetter(text.m_BigFont, text.m_ColorLetter, text.m_ColorBckg);
		print_pix(text.m_BigFont, X, Y, x_step, y_step);
		X += x_step + 1;
		b_ptr = NULL;
	}
	return 0;
}

static int assign_params_from_commands(const state_t state, const char(* commands)[BUFF_SIZE])
{
	int ret=0;
        if(state == state_TEXT)
	{
		printWord(getText(commands));
	}
	return ret;
}

static ssize_t vga_dma_write(struct file *f, const char __user *buf, size_t length, loff_t *off)
{	
	char buff[2*BUFF_SIZE];
	int ret = 0;
	char commands[7][BUFF_SIZE] = {{0}};
	state_t state;
	int i;
	
	ret = copy_from_user(buff, buf, length);  
	if(ret){
		printk("copy from user failed \n");
		return -EFAULT;
	}  
	buff[length] = '\0';
	
	parse_buffer(buff, commands);
	for(i=0; i<7;++i)
		printk("%d: %s\n", i, commands[i]);

	state = getState(commands[0]);
	ret = assign_params_from_commands(state, commands);

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

