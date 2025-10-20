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
#define KILO_VERSION "0.0.1"

//data
struct editorConfig
{
    int curX, curY;
    int screenRows;
    int screenCols;
    struct termios original_termios;
};

struct appendbuff
{
    char *b;
    int len;
};

enum editorKey {
    ARROW_UP = 1000, 
    ARROW_DOWN,
    ARROW_LEFT, 
    ARROW_RIGHT
};

//Terminals
void enableRawMode();
void disableRawMode();
void die(const char* s);
int editorReadKey();
int getWindowSize(int *rows, int *cols);
int getCursorPosition(int *rows, int *cols);

//Append buffer
void abAppend(struct appendbuff *ab, const char *s, int len);
void abFree(struct appendbuff *ab);

//Input
void editorProcessKeypress();
void editorMoveCursor(int key);

//Output
void editorRefreshScreen();
void editorDrawRows();

#endif 