static const bool A7x5[7][5] =
{
	{1, 0, 0, 0, 1},
	{0, 0, 1, 0, 0},
	{0, 1, 1, 1, 0},
	{0, 0, 0, 0, 0},		    
	{0, 1, 1, 1, 0},		        
	{0, 1, 1, 1, 0},			    
	{0, 1, 1, 1, 0}
};

static int small_letter[7][5] = {{0}};
static int big_letter[14][10] = {{0}};

static void doubleSizeMat(void)
{
	int i,j,c,d;
	for(i=0;i<7;i++)
		for(j=0;j<5;j++)
			for(c=0;c<2;c++)
				for(d=0;d<2;d++)
					big_letter[2*i+c][2*j+d] = small_letter[i][j];
}

static void assign(bool set_big_letter, const bool (*letter)[5], int col_let, int col_bckg)
{
	int i,j;
	for(i=0;i<7;i++)
		for(j=0;j<5;j++)
			small_letter[i][j] = (letter[i][j] == 1) ? col_bckg : col_let; 

	if(set_big_letter)
		doubleSizeMat();
}

static void check_the_command(const char* buff)
{
	int i;
	char words[7][BUFF_SIZE] = {{0}};
	int incr=0;
	int len = 0;
	for(i=0;i<strlen(buff);++i)
	{
		if(buff[i] != ',')
			words[incr][i-len] = buff[i];
		else if(buff[i] == ',')
		{
			len += strlen(words[incr]) + 1;
			incr++;
		}
	}	
}
