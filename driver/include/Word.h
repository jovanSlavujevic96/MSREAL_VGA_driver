#ifndef MSREAL_VGA_DRIVER_DRIVER_INCLUDE_WORD_H_
#define MSREAL_VGA_DRIVER_DRIVER_INCLUDE_WORD_H_

#include "Point.h"
#include "utils.h"

#define BIG_FONT_W 10
#define BIG_FONT_H 14
#define SMALL_FONT_W 5
#define SMALL_FONT_H 7

static struct Word
{
	char chars[BUFF_SIZE];
	bool big_font;
    	struct Point pt;
	unsigned long long char_color, bckg_color;
};

#endif //MSREAL_VGA_DRIVER_DRIVER_INCLUDE_WORD_H_
