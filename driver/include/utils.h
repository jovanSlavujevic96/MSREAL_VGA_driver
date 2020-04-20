#ifndef MSREAL_VGA_DRIVER_DRIVER_INCLUDE_UTILS_H_
#define MSREAL_VGA_DRIVER_DRIVER_INCLUDE_UTILS_H_

#define MAX_W 639
#define MAX_H 479

#define BUFF_SIZE 50

typedef int state_t;
enum {state_TEXT, state_LINE, state_RECT, state_CIRC, state_PIX, state_ERR};

u32* tx_vir_buffer;

static unsigned int strToInt(const char* string_num)
{
	int i,dec=1;
	unsigned int val=0;
	for(i=strlen(string_num)-1;i>=0;--i)
	{
		unsigned int tmp = (string_num[i]-48)*dec;
		dec *= 10;
		val += tmp;
	}
	return val;
}

static void parse_buffer(const char* buffer, char(* commands)[BUFF_SIZE])
{
	int i, incr=0, len=0;
	for(i=0;i<strlen(buffer);i++)
	{
		if(buffer[i] != ';' && buffer[i] != '\n')
			commands[incr][i-len] = buffer[i];
		else if(buffer[i] == ';')
		{
			len += strlen(commands[incr]) + 1;
			incr++;
		}
		else if(buffer[i] == '\n')
		{
			break;
		}
	}
}

static state_t getState(const char* command0)
{
	if(!strcmp(command0,"TEXT") || !strcmp(command0,"text") )
		return state_TEXT;
	else if(!strcmp(command0,"LINE") || !strcmp(command0,"line") )
		return state_LINE;
	else if(!strcmp(command0,"RECT") || !strcmp(command0,"rect") )
		return state_RECT;
	else if(!strcmp(command0,"CIRC") || !strcmp(command0,"circ") )
		return state_CIRC;
	else if(!strcmp(command0,"PIX")  || !strcmp(command0,"pix" ) )
		return state_PIX;
	printk(KERN_ERR "%s is not appropriate command\n",command0);
	return state_ERR;
}

#endif //MSREAL_VGA_DRIVER_DRIVER_INCLUDE_UTILS_H_
