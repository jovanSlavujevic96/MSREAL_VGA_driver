#ifndef MSREAL_VGA_DRIVER_DRIVER_INCLUDE_LINE_H_
#define MSREAL_VGA_DRIVER_DRIVER_INCLUDE_LINE_H_

#include "utils.h"
#include "Point.h"

struct Line
{
	struct Point pt1, pt2;
	unsigned long long line_color;
	bool horisontal;
};

//methods
void printLine(const struct Line* line);
int setLine(struct Line* line, const char(* commands)[BUFF_SIZE]);
void LineOnScreen(const struct Line* line);

#endif //MSREAL_VGA_DRIVER_DRIVER_INCLUDE_LINE_H_