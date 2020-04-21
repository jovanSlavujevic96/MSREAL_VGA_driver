#include "characters1.h"
#include "characters2.h"

#include "Word.h"
#include "utils.h"

static const bool(* b_ptr)[7][5] = NULL;
static unsigned long long small_letter[7][5] = {{0}};
static unsigned long long big_letter[14][10] = {{0}};

static const bool(*const letter_forms[])[7][5] = 
{
	&A,&B,&C,&D,&E,&F,&G,&H,&I,&J,&K,&L,&M,&N,&O,&P,&Q,&R,&S,&T,&U,&V,&W,&X,&Y,&Z,
	&a,&b,&c,&d,&e,&f,&g,&h,&i,&j,&k,&l,&m,&n,&o,&p,&q,&r,&s,&t,&u,&v,&w,&x,&y,&z,
	&comma,&dot,&space,&questionnaire,&exclamation
};

static const char letters[] =
{
	'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
	'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
	',','.',' ','?','!'
};


static void initWord(struct Word* word)
{
	int i;
	for(i=0;i<BUFF_SIZE;++i)
		word->chars[i] = 0;
	word->big_font = false;
	word->pt.x = 0, word->pt.y=0;
	word->char_color=0, word->bckg_color=0;
}

static int setWord(struct Word* word, const char(* commands)[BUFF_SIZE])
{
	int i;
	for(i=0;i<strlen(commands[1]);++i)
		word->chars[i] = commands[1][i];
	
	if(!strcmp(commands[2],"big") || !strcmp(commands[2],"BIG") )
		word->big_font = true;
	else if(!strcmp(commands[2],"small") || !strcmp(commands[2],"SMALL") )
		word->big_font = false;
	else
	{
		printk(KERN_ERR "%s this is not appropriate command\n",commands[2]);
		return -1;
	}	
	word->pt.x = strToInt(commands[3]);
	word->pt.y = strToInt(commands[4]);
	i=kstrtoull((unsigned char*)commands[5],0,&word->char_color);
	i=kstrtoull((unsigned char*)commands[6],0,&word->bckg_color);
	return 0;
}

static int check_character(const char character)
{
	if( !(character >= 'A' && character <= 'Z') && !(character >= 'a' && character <= 'z') &&
		character != ' ' && character != '!' && character != ',' && character != '?' && character != '.')
	{
		return -1;
	}
	return 0;
}

static void choose_character(const char character, const bool(** ptr)[7][5])
{
	int i;
	for(i=0;i<57;++i)
		if(character == letters[i])
		{
			*ptr = letter_forms[i];
			return;
		}	
}

static void DoubleSizeMat(void)
{
	int i,j,c,d;
	for(i=0;i<7;++i)
		for(j=0;j<5;j++)
			for(c=0;c<2;c++)
				for(d=0;d<2;d++)
					big_letter[2*i+c][2*j+d]=small_letter[i][j];
}

static void AssignMatFromBool(const bool big_letter, const unsigned long long color_letter, const unsigned long long color_bckg)
{
	int i,j;
	for(i=0;i<7;++i)
		for(j=0;j<5;++j)
		{
			small_letter[i][j] = ((*b_ptr)[i][j] == 1) ? color_bckg : color_letter;
		}
	if(big_letter)
	{
		DoubleSizeMat();
	}
}

static void CharOnScreen(const bool big_font, const unsigned int x_StartPos, const unsigned int y_StartPos, const unsigned int x_Step, const unsigned int y_Step, const unsigned long long Col_Bckg)
{
	unsigned int x,y,i,j;
	for(y=y_StartPos, i=0; i<y_Step; ++i,++y)
		for(x=x_StartPos, j=0; j<x_Step; ++x,++j)
		{
			u32 rgb = (big_font == true) ? ((u32)big_letter[i][j]) : ((u32)small_letter[i][j]);
			tx_vir_buffer[640*y + x] = rgb;
		}
	x=x_StartPos+x_Step;
	for(y=y_StartPos,i=0; i<y_Step; ++i,++y)
	{
		tx_vir_buffer[640*y + x] = (u32)Col_Bckg;
	}
}

static int WordOnScreen(const struct Word* word)
{
	unsigned int i, Y = word->pt.y, X=word->pt.x, strLen = strlen(word->chars),
	x_step = (word->big_font == true) ? BIG_FONT_W : SMALL_FONT_W,
	y_step = (word->big_font == true) ? BIG_FONT_H : SMALL_FONT_H,
	checkX = X + x_step, checkY = Y + y_step;
	bool error=false;
	for(i=0; i<strLen; ++i)
	{
		if(check_character(word->chars[i]) == -1)
		{
			printk(KERN_ERR "VGA_DMA: %c cant be printed on screen, there's not this character on our library!\n",word->chars[i] );
			error = true;	
		}
	}

	if(checkX > MAX_W || checkY > MAX_H)
	{
		printk(KERN_ERR "VGA_DMA: %s cant whole fit into screen by one or both axis!\n",word->chars);
		error = true;
	}

	if(error)
		return -1;

	for(i=0;i<strLen;++i)
	{
		choose_character(word->chars[i], &b_ptr);
		AssignMatFromBool(word->big_font, word->char_color, word->bckg_color);
		CharOnScreen(word->big_font, X, Y, x_step, y_step, word->bckg_color);
		X += x_step+1;
		b_ptr = NULL;
		if(X+x_step > MAX_W && i < strLen-1)
		{
			printk(KERN_ERR "VGA_DMA: %c cant whole fit into screen by x axis!\n",word->chars[i]);
			break;
		}
	}
	return 0;
}
