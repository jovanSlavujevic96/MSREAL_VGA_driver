#include "Point.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h> 

#define BUFF_SIZE 50

struct Point choose_point(const char* );
unsigned long long choose_color(const char* );
const char* choose_fill(const char* );

void print_character(void);
void print_line(void);
void print_rectangle(void);
void print_circle(const unsigned int );

void cases(const unsigned int , bool* );
void menu(unsigned int* );