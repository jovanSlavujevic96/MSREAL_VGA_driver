#include "Line.h"

int setLine(struct Line* line, const char(* commands)[BUFF_SIZE] )
{
	int ret;
	line->pt1.x = strToInt(commands[1]);
	line->pt1.y = strToInt(commands[2]);
	line->pt2.x = strToInt(commands[3]);
	line->pt2.y = strToInt(commands[4]);
	ret = kstrtoull((unsigned char*)commands[5],0,&line->line_color);
	
	return 0;
}

void LineOnScreen(const struct Line* line)
{
	int dx, dy, p, x=line->pt1.x, y=line->pt1.y,x_lim=line->pt2.x, incr=1;
	dx = (int)line->pt2.x-(int)line->pt1.x;
	if(dx < 0) 
		dx *= -1;
	dy = (int)line->pt2.y-(int)line->pt1.y;
	if(dy < 0) 
		dy *= -1;
	if(line->pt1.x > line->pt2.x)
	{
		x = line->pt2.x;
       		y = line->pt2.y;
		x_lim = line->pt1.x;
	}
	if(!(y==line->pt1.y && line->pt1.y<=line->pt2.y) && !(y==line->pt2.y && line->pt2.y<=line->pt1.y))
		incr = -1;

	p = 2*dy-dx;
	
	while(x<x_lim)
	{

		tx_vir_buffer[640*y+x]=(u32)line->line_color;
		if(p>=0)
			y+=incr, p=p+ 2*dy - 2*dx;
		else
			p=p + 2*dy;
		x++;
	}	
}
