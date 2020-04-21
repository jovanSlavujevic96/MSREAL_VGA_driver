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
	int dx, dy, p, x, y;
	dx = (int)line->pt2.x-(int)line->pt1.x;
	dy = (int)line->pt2.y-(int)line->pt1.y;
	x = (int)line->pt1.x;
       	y = (int)line->pt1.y;
	p = 2*dy-dx;

	while(x<line->pt2.x)
	{

		tx_vir_buffer[640*y+x]=(u32)line->line_color;
		if(p>=0)
			++y, p=p+ 2*dy - 2*dx;
		else
			p=p + 2*dy;
		x++;
	}	
}
