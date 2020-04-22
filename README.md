# MSREAL_VGA_driver

# driver user manual:
# 1.go to driver dir:                            $ cd driver/
# 2.for building code and turning on driver run: $ ./run_driver.sh  
# this script shall do make command, remove unnecessary output files and rmmod and insmod driver output file (.ko)
# 3.commands for checking driver:
#     3a. example of printing letter/s:          $ echo "text;STRING;big;5;5;0xff;0x00" >> /dev/vga_dma
#                                                @text/TEXT - indicator of printing charactes
#                                                @STRING - any letters and this characters: "." "," "!" "?" " "
#                                                @big/BIG/small/small - indicator of printing of big or small font
#                                                @5;5 - x and y coordinates of top left starting pixel of word/letter
#                                                @0xff - hex rgb val for color of characters
#                                                @0x00 - hex rgb val for color of background of characters

#     3b. example of drawing line:               $ echo "line;5;5;23;1;0xff" >> /dev/vga_dma
#                                                @line/LINE - indicator of drawing line
#                                                @5;5 - x and y coordinates of staring point of line
#                                                @23;1 - x and y coordinates of ending point of line
#                                                @0xff - hex rgb val for color of line

#     3c. example of drawing rectangle:          $ echo "rect;5;5;23;1;0xff;no" >> /dev/vga_dma 
#                                                @rect/RECT - indicator of drawing rectangle
#                                                @5;5 - x and y coordinates of top left point of rectangle
#                                                @23;1 - x and y coordinates of bottom right point of rectangle
#                                                @0xff - hex rgb val of color of rectangle
#                                                @fill/FILL/no/NO - indicator of filling or not filling rectangle with color

#     3d. example of drawing circle:             $ echo "circ;60;60;30;0xff;fill" >> /dev/vga_dma 
#                                                @circ/CIRC - indicator of drawing circle
#                                                @60;60 - x and y coordinates of center point of the circle
#                                                @30 - radius of the circle
#                                                @0xff - hex rgb val of color of circle
#                                                @fill/FILL - indicator of filling or not filling circle with color
