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

void approximate_circle(const unsigned int Cx, const unsigned int Cy, const unsigned int r, const bool fill_circle, const unsigned long long color)
{
    int x,y;
    int xx = Cx-r, yy = Cy-r;
    float q;
    char command[BUFF_SIZE] = {0};
    for(x=xx; x<=(xx+2*r); ++x)
        for(y=yy; y<=(yy+2*r); ++y)
        {
            q = sqrt( (float)(pow(Cx-x,2)+ pow(Cy-y,2)) );
            bool _fill = (r*0.9>q) ? true : false,
            _nofill = (r*0.9>q && r*0.8<q) ? true : false;
            bool info = (fill_circle == true) ? _fill : _nofill;
            if(info)
            {
                reset_string(command);
                sprintf(command,"pix;%d;%d;%#04llx\n",x,y,color);
                sendToDriver(command);
            }
        }
}
