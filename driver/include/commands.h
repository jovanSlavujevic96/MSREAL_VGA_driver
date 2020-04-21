#include "PrintWord.h"
#include "PrintLine.h"
#include "PrintRect.h"
#include "PrintCircle.h"

static int assign_params_from_commands(const state_t state, const char(* commands)[BUFF_SIZE])
{
	int ret=0;
	if(state == state_TEXT)
	{
		struct Word word;
		initWord(&word);
		ret = setWord(&word, commands);
		if(ret == -1)
			return ret;
        //printWord(&word);
		ret = WordOnScreen(&word);
	}
	else if(state == state_LINE)
	{
		struct Line line;
		ret = setLine(&line, commands);
		if(ret == -1)
			return ret;
		//printLine(&line);
		LineOnScreen(&line);
	}
	else if(state == state_RECT)
	{
		struct Rect rect;
		ret = setRect(&rect, commands);
		if(ret == -1)
			return ret;
		//printRect(&rect);
		RectOnScreen(&rect);
	}
	else if(state == state_CIRC)
	{
		struct Circle circle;
		setCircle(&circle, commands);
		CircleOnScreen(&circle);
	}
	else if(state == state_PIX)
	{
		unsigned long long pix_color;
		unsigned int x=strToInt(commands[1]),
		y = strToInt(commands[2]);
		ret = kstrtoull((unsigned char*)commands[3],0,&pix_color);
		tx_vir_buffer[640*y+x] = (u32)pix_color;
	}
	return ret;
}
