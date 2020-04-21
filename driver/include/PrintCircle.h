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

struct _8points SetOfCirclePoints(int xc, int yc, int x, int y)
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

void fill8points(const struct _8points* pts, const bool fill, const unsigned long long color)
{
	int j,k;
	if(pts == NULL)
		return;
	for(j=0; j<4; ++j)
	{
		tx_vir_buffer[640*pts->pt[2*j].y + pts->pt[2*j].x] = (u32)color;
		tx_vir_buffer[640*pts->pt[2*j].y + pts->pt[2*j+1].x] = (u32)color;
		if(fill)	
			for(k=pts->pt[2*j+1].x+1;k<pts->pt[2*j].x;++k)
				tx_vir_buffer[640*pts->pt[2*j].y + k] = (u32)color;			
	}
}

void CircleOnScreen(const struct Circle* circle)
{
	int x = 0, y = circle->r;
	int d = 3 - 2 * circle->r;
	struct _8points tmp = SetOfCirclePoints(circle->pt.x,circle->pt.y,x,y);
	fill8points(&tmp,circle->fill_circle, circle->circle_color);
	while(y >= x)
	{
		++x;
		if(d > 0)
		{
			--y;
			d = d + 4*(x-y) +10;
		}
		else
			d = d + 4*x +6;
		tmp = SetOfCirclePoints(circle->pt.x,circle->pt.y,x,y);
		fill8points(&tmp,circle->fill_circle, circle->circle_color);
	}
}
