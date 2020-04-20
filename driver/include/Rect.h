#ifndef MSREAL_VGA_DRIVER_DRIVER_INCLUDE_RECT_H_
#define MSREAL_VGA_DRIVER_DRIVER_INCLUDE_RECT_H_

#include "Point.h"

struct Rect
{
	struct Point pt1, pt2;
	unsigned long long rect_color;
	bool fill_rect;
};

#endif //MSREAL_VGA_DRIVER_DRIVER_INCLUDE_RECT_H_
