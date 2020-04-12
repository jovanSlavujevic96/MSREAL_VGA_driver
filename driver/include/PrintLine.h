#include "Line.h"

void printLine(const struct Line* line)
{
	printk("Line info:\n");
	printk("(%d,%d) <-> (%d,%d)\n",line->pt1.x, line->pt1.y, line->pt2.x, line->pt2.y);
	printk("line color: %llu\n", line->line_color);
	printk("line: %s\n", (line->horisontal == true) ? "Horizontal" : "Vertical");
}

int setLine(struct Line* line, const char(* commands)[BUFF_SIZE] )
{
	int ret;
	line->pt1.x = strToInt(commands[1]);
	line->pt1.y = strToInt(commands[2]);
	line->pt2.x = strToInt(commands[3]);
	line->pt2.y = strToInt(commands[4]);
	ret = kstrtoull((unsigned char*)commands[5],0,&line->line_color);
	
	if(line->pt1.y == line->pt2.y)
		line->horisontal = true;
	else if(line->pt1.x == line->pt2.x)
		line->horisontal = false;
	else
	{
		printk(KERN_ERR "VGA_DMA: Line is nor horisontal nor vertical!\n");
		return -1;
	}
	return 0;
}

void LineOnScreen(const struct Line* line)
{
	int i,start,end;
	if(line->horisontal)
	{
		if(line->pt2.x > line->pt1.x)
			start = line->pt1.x,
			end = line->pt2.x;
		else
			start = line->pt2.x,
			end = line->pt1.x;

		for(i=start; i<=end; ++i)
			tx_vir_buffer[640*line->pt1.y + i] = (u32)line->line_color;
		return;
	}
	if(line->pt2.y > line->pt1.y)
		start = line->pt1.y,
		end = line->pt2.y;
	else
		start = line->pt2.y,
		end = line->pt1.y;
	
	for(i=start; i<=end; ++i)
		tx_vir_buffer[640*i + line->pt1.x] = (u32)line->line_color;
}