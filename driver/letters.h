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

#define BUFF_SIZE 100

static bool (*b_ptr)[7][5] = NULL;

static const bool A[7][5] =
{
	{1, 0, 0, 0, 1},
	{0, 0, 1, 0, 0},
	{0, 1, 1, 1, 0},
	{0, 0, 0, 0, 0},		    
	{0, 1, 1, 1, 0},		        
	{0, 1, 1, 1, 0},			    
	{0, 1, 1, 1, 0}
};

static u32 small_letter[7][5] = {{0}};
static u32 big_letter[14][10] = {{0}};


