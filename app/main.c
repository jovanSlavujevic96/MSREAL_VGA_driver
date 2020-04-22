#include "include/app_menu.h"
#include "include/app_utils.h"

#include <stdio.h>

int main(void)
{
    printf("Welcome to VGA driver test application!\n");
    KeyboardInit();
    
    bool out=false;
    while(!out)
    {
        unsigned int val;
        menu(&val);
        cases(val, &out);
    }
    printf("Succesfully exited program!\n");
    return 0;
}