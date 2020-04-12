#include "PrintCircle.h"

void setCircle(struct Circle* circle, const char(* commands)[BUFF_SIZE])
{
	int ret;
	circle->pt.x = strToInt(commands[1]);
	circle->pt.y = strToInt(commands[2]);
	circle->r = strToInt(commands[3]);
	ret = kstrtoull((unsigned char*)commands[4],0, &circle->circle_color);
	if(ret)
		return;
	circle->fill_circle = (!strcmp(commands[5],"fill") || !strcmp(commands[5],"FILL")) ? true : false;
}

void CircleOnScreen(const struct Circle* circle)
{
	int XX = circle->pt.x - circle->r, YY = circle->pt.y - circle->r;
	unsigned int xx=(unsigned int)XX, yy=(unsigned int)YY, x,y;
    if(XX < 0 || YY < 0)
	{
		printk(KERN_ERR "can fit whole on screen!\n");
		return;
	}

	for(x = xx; x <= (xx+2*circle->r); ++x)
		for(y = yy; y <= (yy+2*circle->r); ++y)
		{
			bool info;
            //float q = sqrt((float)((circle->pt.x-x)*(circle->pt.x-x)+(circle->pt.y-y)*(circle->pt.y-y) )) ;
			//info = (circle->fill_circle == true) ? ( (circle->r*0.9 > q) ? true : false ) : ((circle->r*0.9 > q && circle->r*0.8 < q) ? true : false);
			if(info)	
			{
				tx_vir_buffer[640*y + x] = (u32) circle->circle_color;
			}
		}
}