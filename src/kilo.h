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
#include <time.h>
#include <stdarg.h>
#include <fcntl.h>

//Defines
#define CTRL_KEY(k) ((k) & 0x1f)
#define ABUF_INIT {NULL, 0}
#define KILO_VERSION "0.0.1"
#define KILO_TAB_STOP 8
#define KILO_QUIT_TIMES 3

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
    int dirty;
    char *fileName;
    char statusMsg[80];
    time_t statusMsgTime;
    struct termios original_termios;
    
};

struct appendbuff
{
    char *b;
    int len;
};

enum editorKey {
    BACKSPACE = 127,
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
char *editorPrompt(char *prompt);

//Output
void editorRefreshScreen();
void editorDrawRows(struct appendbuff *ab);
void editorScroll();
void editorDrawStatusBar(struct appendbuff * ab);
void editorStatusMessage(const char* fmt, ...);
void editorDrawMessageBar(struct appendbuff *ab);

//File i/o
void editorOpen(char *fileName);
char *editorRowsToString(int *bufferlen);
void editorSave();
void editorFind();

//Row Operations
void editorInsertRow(int at, char *s, size_t len);
void editorUpdateRow(erow *row);
int editorRowCurXToRenderX(erow *row, int curX);
int editorRowRenderXToCurX(erow *row, int rx);
void editorRowInsertChar(erow *row, int at, int c);
void editorRowDeleteChar(erow *row, int at);
void editorFreeRow(erow *row);
void editorDeleteRow(int at);
void editorRowAppendString(erow *row, char *s, size_t len);

//Editor Operations
void editorInsertChar(int c);
void editorDeleteChar();
void editorInsertNewLine();

//Prototypes
void editorSetStatusMessage(const char *fmt, ...); 
void editorRefreshScreen(); 


#endif 