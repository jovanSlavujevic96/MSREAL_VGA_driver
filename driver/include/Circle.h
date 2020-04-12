#ifndef MSREAL_VGA_DRIVER_DRIVER_INCLUDE_CIRCLE_H_
#define MSREAL_VGA_DRIVER_DRIVER_INCLUDE_CIRCLE_H_

#include "utils.h"
#include "Point.h"

struct Circle
{
	struct Point pt;
	unsigned int r;
	unsigned long long circle_color;
	bool fill_circle;
};

//methods
void setCircle(struct Circle* circle, const char(* commands)[BUFF_SIZE]);
void CircleOnScreen(const struct Circle* circle);

#endif //MSREAL_VGA_DRIVER_DRIVER_INCLUDE_CIRCLE_H_

