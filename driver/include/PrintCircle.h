#include "Circle.h"

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

struct _8points SetOfCirclePoints(int xc, int yc, int x, int, y)
{
	struct _8points pts;
	pts.pt[0].x = xc+x;
	pts.pt[0].y = yc+y;

	pts.pt[1].x = xc-x;
	pts.pt[1].y = yc+y;

	pts.pt[2].x = xc+x;
	pts.pt[2].y = yc-y;

	pts.pt[3].x = xc-x;
	pts.pt[3].y = yc-y;

	pts.pt[4].x = xc+y;
	pts.pt[4].y = yc+x;

	pts.pt[5].x = xc-y;
	pts.pt[5].y = yc+x;

	pts.pt[6].x = xc+y;
	pts.pt[6].y = yc-x;

	pts.pt[7].x = xc-y;
	pts.pt[7].y = yc-x;

	return pts;
}

void CircleOnScreen(const struct Circle* circle)
{
	int x = 0, y = circle->r;
	int d = 3 - 2 * circle->r;
	unsigned int incr = 1, i,j,k;
	struct _8points base = SetOfCirclePoints(circle->pt.x,circle->pt.y,x,y);
	struct _8points *dynamic_array;
	while(y >= x)
	{
		++incr;
		++x;
		if(d > 0)
			--y;
	}
	dynamic_array = (struct _8points*)malloc(incr*sizeof(struct _8points));
	dynamic_array[0] = base;
	x = 0, y = circle->r;
	d = 3 - 2 * circle->r;
	i = 1;
	while(y >= x)
	{
		++x;
		if(d > 0)
		{
			--y;
			d = d + 4*(x-y) + 10;
		}
		else
			d = d + 4*x + 6;
		dynamic_array[incr] = SetOfCirclePoints(circle->pt.x,circle->pt.y,x,y);
		++incr
	}
	for(i=0; i<incr; ++i)
	{
		for(j=0; j<4; ++j)
			if(circle->fill_circle == false)
			{
				tx_vir_buffer[640*dynamic_array[i].pt[2*j].y + dynamic_array[i].pt[2*j].x] = (u32)circle->circle_color;
				tx_vir_buffer[640*dynamic_array[i].pt[2*j+1].y + dynamic_array[i].pt[2*j].x] = (u32)circle->circle_color;
			}
			else
				for(k=dynamic_array[i].pt[2*j+1].x; k<=dynamic_array[i].pt[2*j].x; ++k)
				{
					tx_vir_buffer[640*dynamic_array[i].pt[2*j].y + k] = (u32)circle->circle_color;
				}
	}

}
