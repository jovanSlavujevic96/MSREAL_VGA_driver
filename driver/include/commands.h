#include "utils.h"
#include "Word.h"
#include "Line.h"
#include "Rect.h"
#include "Circle.h"

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
	return ret;
}