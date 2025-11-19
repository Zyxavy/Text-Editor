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

#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)
#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

//data
typedef struct erow
{
    int size;
    int rSize;
    char *chars;
    char *render;
    unsigned char *highlight;
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
    int dirty;

    erow *row;
    char *fileName;
    char statusMsg[80];

    time_t statusMsgTime;
    struct termios original_termios;
    struct editorSyntax *syntax;
    
};

struct appendbuff
{
    char *b;
    int len;
};

struct editorSyntax
{
    char *filetype;
    char **filematch;
    char **keywords;
    char *singleLineCommentStart;
    int flags;
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

enum editorHighlight
{
    HL_NORMAL = 0,
    HL_COMMENT,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_STRING,
    HL_NUMBER,
    HL_MATCH
};

//File Types

//for C/C++
char *C_HL_extension[] = {".c", ".h", ".cpp", ".hpp", NULL };
char *C_HL_keywords[] = {
  "switch", "if", "while", "for", "break", "continue", "return", "else",
  "struct", "union", "typedef", "static", "enum", "class", "case",
  "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
  "void|", NULL
};

struct editorSyntax HLDB[] = 
{
    {
        "c", C_HL_extension, C_HL_keywords, "//", HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
    },
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
char *editorPrompt(char *prompt, void(*callback)(char*,int));

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

//Find
void editorFind();
void editorFindCallback(char *query, int key);

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

//Syntax Highlighting
void editorUpdateSyntax(erow *row);
int editorSyntaxToColor(int highlight);
int isSeparator(int c);
void editorSelectSyntaxHighlight();

#endif 