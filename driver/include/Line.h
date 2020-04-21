#ifndef MSREAL_VGA_DRIVER_DRIVER_INCLUDE_LINE_H_
#define MSREAL_VGA_DRIVER_DRIVER_INCLUDE_LINE_H_

#include "utils.h"
#include "Point.h"

struct Line
{
	struct Point pt1, pt2;
	unsigned long long line_color;
};

#endif //MSREAL_VGA_DRIVER_DRIVER_INCLUDE_LINE_H_
