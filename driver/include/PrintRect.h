#include "Rect.h"
#include "Line.h"

void printRect(const struct Rect* rect)
{
	printk("Rect info:\n");
	printk("(%d,%d) <-> (%d,%d)\n",rect->pt1.x, rect->pt1.y, rect->pt2.x, rect->pt2.y);
	printk("rect color: %llu\n", rect->rect_color);
	printk("fill rect: %s\n", (rect->fill_rect == true) ? "true" : "false");
}

int setRect(struct Rect* rect, const char(* commands)[BUFF_SIZE] )
{
	int ret;
	rect->pt1.x = strToInt(commands[1]);
	rect->pt1.y = strToInt(commands[2]);
	rect->pt2.x = strToInt(commands[3]);
	rect->pt2.y = strToInt(commands[4]);
	ret = kstrtoull((unsigned char*)commands[5],0,&rect->rect_color);
	
	if(!strcmp(commands[6],"FILL") || !strcmp(commands[6],"fill"))
		rect->fill_rect = true;
	else if(!strcmp(commands[6],"NO") || !strcmp(commands[6],"no"))
		rect->fill_rect = false;
	else
	{
		printk(KERN_ERR "%s -> incorrect command!\n",commands[6]);
		return -1;
	}
	return 0;
}

void RectOnScreen(const struct Rect* rect)
{
	unsigned int startX=rect->pt2.x, endX=rect->pt1.x ,startY=rect->pt2.y, endY=rect->pt1.y, i,j;
	if(!rect->fill_rect)
	{
		struct Line lines[4] = 
		{ 
			{{rect->pt1.x, rect->pt1.y}, {rect->pt2.x, rect->pt1.y}, rect->rect_color, true},
			{{rect->pt1.x, rect->pt1.y}, {rect->pt1.x, rect->pt2.y}, rect->rect_color, false},
			{{rect->pt1.x, rect->pt2.y}, {rect->pt2.x, rect->pt2.y}, rect->rect_color, true},
			{{rect->pt2.x, rect->pt1.y}, {rect->pt2.x, rect->pt2.y}, rect->rect_color, false}
		};
		for(i=0;i<4;i++)
			LineOnScreen(&lines[i]);
		return;
	}
	if(rect->pt1.x < rect->pt2.x)
		startX = rect->pt1.x, endX = rect->pt2.x;
	if(rect->pt1.y < rect->pt2.y)
		startY = rect->pt1.y, endY = rect->pt2.y;
	for(i=startX; i<=endX; ++i)
		for(j=startY; j<=endY; ++j)
			tx_vir_buffer[640*j + i] = (u32)rect->rect_color;
}
