#include "../include/app_utils.h"
#include "../include/app_menu.h"

#include <termios.h> //for termios structure
#include <unistd.h> //for STDIN_FILENO macro

#include <stdio.h>	//for FILE, fprintf, printf
#include <string.h>

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
