#ifndef MSREAL_VGA_DRIVER_DRIVER_INCLUDE_RECT_H_
#define MSREAL_VGA_DRIVER_DRIVER_INCLUDE_RECT_H_

#include "utils.h"
#include "Point.h"

struct Rect
{
	struct Point pt1, pt2;
	unsigned long long rect_color;
	bool fill_rect;
};

//methods
void printRect(const struct Rect* rect);
int setRect(struct Rect* rect, const char(* commands)[BUFF_SIZE] );
void RectOnScreen(const struct Rect* rect);

#endif //MSREAL_VGA_DRIVER_DRIVER_INCLUDE_RECT_H_