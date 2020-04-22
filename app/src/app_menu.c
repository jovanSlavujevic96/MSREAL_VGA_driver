#include <stdio.h>
#include <string.h>

#include <unistd.h> //for usleep

#include "../include/app_menu.h"
#include "../include/colors.h"
#include "../include/app_utils.h"
#include "../include/Point.h"

#define new_line printf("\n")
#define delay2sec usleep(2000000)

static const char color_str[9][9] = {"Black","White","Red","Blue","Green","Yellow","Gray","Brown","Purple"};
static const unsigned long color_val[9] = {black, white, red, blue, green, yellow, gray, brown, purple};

static inline struct Point choose_point(const char* of_what)
{
    struct Point pt;
    printf("Enter the coordinates of %s with comma between then press <enter>: ", of_what);
    scanf("%d,%d",&pt.x,&pt.y);
    
    printf("x:%d,y:%d\n",pt.x,pt.y);
    return pt;
}

static inline unsigned long long choose_color(const char* of_what)
{
    int tmp;
    printf("Choose color of %s by typing option number:\n\
        [1]Black  [2]White  [3]Red  [4]Blue  [5]Green  [6]Yellow  [7]Gray  [8]Brown  [9]Purple\n", of_what);
    while(!(tmp >= 1 && tmp <= 9))
        tmp = getchar()-48, new_line;
    printf("You chose: %s\n",color_str[tmp-1]);
    return color_val[tmp-1];
}

static inline const char* choose_fill(const char* what)
{
    printf("Do you want to fill the %s?\nPress <Y> or <N>: ", what);
    char tmp=0;
    while(tmp != 'n' && tmp != 'N' && tmp != 'y' && tmp != 'Y')
        tmp = getchar(), new_line;
    return (tmp == 'n' || tmp == 'N') ? "no" : "fill";
}

static inline void print_character(void)
{
    char string[BUFF_SIZE]={0};
    unsigned long long char_color=choose_color("character/s"), bckg_color; 
    printf("\nEnter character/s then press <enter>: ");
    while(true)
    {
        int i;
        bool continue_loop=false;
        scanf("%s",string);
        for(i=0;i<strlen(string);++i)
            if( !(string[i] >= 'a' && string[i] <= 'z') && !(string[i] >= 'A' && string[i] <= 'Z') && string[i] != '!' &&
                string[i] != ',' && string[i] != ' ' && string[i] != '.' && string[i] != '?')
            {
                printf("\nInappropriate character in string!\nTry again: ");
                continue_loop=true;
                reset_string(string);
                break;
            }
        if(!continue_loop)
            break;
    }   
    printf("Do you want to print big(14x10) or small(7x5) font?\nPress <B> or <S>: ");
    char tmp=0;
    while(tmp != 'S' && tmp != 's' && tmp != 'b' && tmp != 'B')
        tmp = getchar(), new_line;

    const char* font = (tmp == 'S' || tmp == 's') ? "small" : "big";
    struct Point pt = choose_point("character/s");
    bckg_color=choose_color("background");

    char command[BUFF_SIZE*2]={0};
    sprintf(command,"text;%s;%s;%d;%d;%#04llx;%#04llx\n",string,font,pt.x,pt.y,char_color,bckg_color);
    sendToDriver(command);
    delay2sec;
}

static inline void print_line(void)
{
    const struct Point pt1 = choose_point("start of line"), pt2 = choose_point("end of line");
    const unsigned long long line_color = choose_color("line");
    char command[BUFF_SIZE*2] = {0};
    sprintf(command,"line;%d;%d;%d;%d;%#04llx\n",pt1.x,pt1.y,pt2.x,pt2.y,line_color);
    sendToDriver(command);
    delay2sec;
}

static inline void print_rectangle(void)
{
    const struct Point pt1 = choose_point("start of rectangle"), pt2 = choose_point("end of rectangle");
    const unsigned long long rectangle_color = choose_color("rectangle");
    printf("Do you want to fill the rectangle?\nPress <Y> or <N>: ");
    const char* fill_rectangle = choose_fill("rectangle");
    char command[BUFF_SIZE*2] = {0};
    sprintf(command,"rect;%d;%d;%d;%d;%#04llx;%s\n",pt1.x,pt1.y,pt2.x,pt2.y,rectangle_color,fill_rectangle);
    sendToDriver(command);
    delay2sec;
}

static inline void print_circle(const unsigned int solution)
{
    const struct Point pt1 = choose_point("center of circle");
    unsigned int r;
    printf("Enter the radius of circle: ");
    scanf("%d",&r);
    printf("r: %d\n",r);
    const unsigned long long circle_color = choose_color("circle");
    const char* fill_circle = choose_fill("rectangle");
    char command[BUFF_SIZE*2] = {0};
    sprintf(command,"circ;%d;%d;%d;%#04llx;%s\n",pt1.x,pt1.y,r,circle_color,fill_circle);
    sendToDriver(command);
    delay2sec;
}

void cases(const unsigned int Case, bool* Out)
{
    switch(Case)
    {
        case 1:
            print_character();
        break;

        case 2:
            print_line();
        break;

        case 3:
            print_rectangle();
        break;

        case 4:
            print_circle(0);
        break;

        case 5:
            *Out = true;
        break;
    }
}

void menu(unsigned int* TypedChar)
{
    printf("\nMenu:\n\
        [1] Print character/s on VGA screen.\n\
        [2] Draw line on VGA screen.\n\
        [3] Draw rectangle on VGA screen.\n\
        [4] Draw circle on VGA screen.\n\
        [5] Exit application.\n\
        Select option by typing option number.\n");
    *TypedChar = getchar()-48;
    new_line;
}
