#ifndef KILO_H
#define KILO_H


//Headers
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>

//Defines
#define CTRL_KEY(k) ((k) & 0x1f)

//data

struct editorConfig
{
    int screenRows;
    int screenCols;
    struct termios original_termios;
};

//Terminals
void enableRawMode();
void disableRawMode();
void die(const char* s);
char editorReadKey();
int getWindowSize(int *rows, int *cols);
int getCursorPosition(int *rows, int *cols);

//Input
void editorProcessKeypress();

//Output
void editorRefreshScreen();
void editorDrawRows();



#endif 