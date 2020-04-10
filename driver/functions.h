#include "letters.h"

static int choose_letter(const char letter)
{
    if((letter >= 'A' && letter <= 'Z') || (letter >= 'a' && letter <= 'z') || 
        letter != ' ' || letter != '!' || letter != ',' || letter != '?' || letter != '.')
    {   
        return -1;
    }
    if(letter == 'A')
        b_ptr = &A;

    return 0;
}

static void doubleSizeMat(void)
{
	int i,j,c,d;
	for(i=0;i<7;i++)
		for(j=0;j<5;j++)
			for(c=0;c<2;c++)
				for(d=0;d<2;d++)
					big_letter[2*i+c][2*j+d] = small_letter[i][j];
}

static void assign(bool set_big_letter, const bool (*letter)[5], const u32 col_let, const u32 col_bckg)
{
	int i,j;
	for(i=0;i<7;i++)
		for(j=0;j<5;j++)
			small_letter[i][j] = (letter[i][j] == 1) ? col_bckg : col_let; 

	if(set_big_letter)
    {
		doubleSizeMat();
    }
}

static unsigned int strToInt(const char* str)
{
    int i;
    int dec=1;
    unsigned int val=0;
    for(i=strlen(str)-1; i>=0; --i)
    {
        int a = (str[i]-48)*dec;
        dec *= 10;
        val += a;
    }
    return val;
}

static void assign_commands(const char* buff, char (*commands)[BUFF_SIZE])
{
    int i;
	int incr=0;
	int len = 0;
	for(i=0;i<strlen(buff);++i)
	{
		if(buff[i] != ',')
			commands[incr][i-len] = buff[i];
		else if(buff[i] == ',')
		{
			len += strlen(commands[incr]) + 1;
			incr++;
		}
	}
}