#include "../include/app_utils.h"
#include "../include/app_menu.h"

#include <math.h>   //for pow() and sqrt()

#include <termios.h> //for termios structure

void KeyboardInit(void)
{
    static struct termios oldt, newt;
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);          
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
}

void reset_string(char* string)
{
    int i;
    for(i=0;i<strlen(string);++i)
        string[i] = 0;
}

void sendToDriver(const char* Command)
{
    printf("command sent to driver:\n%s",Command);
    FILE* fp = fopen("/dev/vga_dma","w");
    if(fp != NULL)
        fprintf(fp,"%s",Command);
    fclose(fp);
}


void drawCircle(int xc, int yc, int x, int y, unsigned long long color)
{
	char command[BUFF_SIZE] = {0};	
        sprintf(command,"pix;%d;%d;%#04llx\n",xc+x,yc+y,color);
        sendToDriver(command);
	reset_string(command);
	sprintf(command,"pix;%d;%d;%#04llx\n",xc-x,yc+y,color);
        sendToDriver(command);
	reset_string(command);
	sprintf(command,"pix;%d;%d;%#04llx\n",xc+x,yc-y,color);
        sendToDriver(command);
	reset_string(command);
	sprintf(command,"pix;%d;%d;%#04llx\n",xc-x,yc-y,color);
        sendToDriver(command);
	reset_string(command);
	sprintf(command,"pix;%d;%d;%#04llx\n",xc+y,yc+x,color);
        sendToDriver(command);
	reset_string(command);
	sprintf(command,"pix;%d;%d;%#04llx\n",xc-y,yc+x,color);
        sendToDriver(command);
	reset_string(command);
	sprintf(command,"pix;%d;%d;%#04llx\n",xc+y,yc-x,color);
        sendToDriver(command);
	reset_string(command);
	sprintf(command,"pix;%d;%d;%#04llx\n",xc-y,yc-x,color);
	sendToDriver(command);
}

void approximate_circle(const unsigned int Cx, const unsigned int Cy, const unsigned int r, const bool fill_circle, const unsigned long long color)
{
	int x = 0, y = r;
	int d = 3 - 2 * r;
	drawCircle(Cx,Cy,x,y,color);
	while(y >= x)
	{
		x++;
		if(d > 0)
		{
			y--;
			d = d + 4*(x-y) + 10;
		}
		else
			d = d + 4*x + 6;

		drawCircle(Cx,Cy,x,y,color);
	}
}
