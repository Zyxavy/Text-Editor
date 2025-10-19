#ifndef KILO_H
#define KILO_H


//Headers
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

//Defines
#define CTRL_KEY(k) ((k) & 0x1f)

//data
struct termios original_termios;


//Terminals
void enableRawMode();
void disableRawMode();
void die(const char* s);
char editorReadKey();

//Input
void editorProcessKeypress();

//Output
void editorRefreshScreen();


#endif 