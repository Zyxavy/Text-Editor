#ifndef KILO_H
#define KILO_H

//Feature test macros
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

//Headers
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>

//Defines
#define CTRL_KEY(k) ((k) & 0x1f)
#define ABUF_INIT {NULL, 0}
#define KILO_VERSION "0.0.1"
#define KILO_TAB_STOP 8

//data
typedef struct erow
{
    int size;
    int rSize;
    char *chars;
    char *render;
} erow;

struct editorConfig
{
    int curX, curY;
    int renderX;
    int rowOffset;
    int colOffset;
    int screenRows;
    int screenCols;
    int numRows;
    erow *row;
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
    ARROW_RIGHT,
    PAGE_UP,
    PAGE_DOWN,
    HOME_KEY,
    END_KEY,
    DELETE_KEY
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
void editorScroll();

//File i/o
void editorOpen(char *fileName);

//Row Operations
void editorAppendRow(char *s, size_t len);
void editorUpdateRow(erow *row);
int editorRowCurXToRenderX(erow *row, int curX);

#endif 