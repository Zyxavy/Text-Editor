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
#include <string.h>

//Defines
#define CTRL_KEY(k) ((k) & 0x1f)
#define ABUF_INIT {NULL, 0}

//data
struct editorConfig
{
    int screenRows;
    int screenCols;
    struct termios original_termios;
};

struct appendbuff
{
    char *b;
    int len;
};


//Terminals
void enableRawMode();
void disableRawMode();
void die(const char* s);
char editorReadKey();
int getWindowSize(int *rows, int *cols);
int getCursorPosition(int *rows, int *cols);

//Append buffer
void abAppend(struct appendbuff *ab, const char *s, int len);
void abFree(struct appendbuff *ab);

//Input
void editorProcessKeypress();

//Output
void editorRefreshScreen();
void editorDrawRows();

#endif 