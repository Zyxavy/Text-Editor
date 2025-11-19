#include "kilo.h"

//  #===Input===#
void editorProcessKeypress()
{
    static int quitTimes = KILO_QUIT_TIMES;

    int c = editorReadKey();
    switch (c)
    {
        case '\r':
            editorInsertNewLine();
            break;

        case CTRL_KEY('q'): //Quit on Ctrl-Q
            if (E.dirty && quitTimes > 0)
            {
                editorStatusMessage("WARNING! File has unsaved changes. " "Press Ctrl-Q %d more times to quit.", quitTimes);
                quitTimes--;
                return;
            }

            write(STDOUT_FILENO, "\x1b[2J", 4); 
            write(STDOUT_FILENO, "\x1[H", 3); 
            exit(0); 
            break;
        
        case CTRL_KEY('s'): editorSave(); break;

        case HOME_KEY: //Go to beginning of line
            E.curX = 0;
            break;
        case END_KEY: //Go to end of line
            if(E.curY < E.numRows) E.curX = E.row[E.curY].size++;
            break;

        case CTRL_KEY('f'): //Find
            editorFind();
            break;
        
            //delete or backspaces
        case BACKSPACE:
        case CTRL_KEY('h'):
        case DELETE_KEY:
            if(c == DELETE_KEY) editorMoveCursor(ARROW_RIGHT);
            editorDeleteChar();
            break;
        
            //navigate up and down
        case PAGE_UP: 
        case PAGE_DOWN:
            {
                if(c == PAGE_UP)
                {
                    E.curY = E.rowOffset;
                }
                else if(c == PAGE_DOWN)
                {
                    E.curY = E.rowOffset + E.screenRows - 1;
                    if(E.curY > E.numRows) E.curY = E.numRows;
                }

                int times = E.screenRows;
                while(times--) editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN); //Move cursor up or down by screen rows
            }
            break;

        case ARROW_LEFT: 
        case ARROW_RIGHT: 
        case ARROW_UP: 
        case ARROW_DOWN:
            editorMoveCursor(c); break;
        case CTRL_KEY('l'):
        case '\x1b':
            break;
        default:
            editorInsertChar(c); break;
    }
    quitTimes = KILO_QUIT_TIMES;
}

void editorMoveCursor(int key)
{
    erow *row = (E.curY >= E.numRows) ? NULL : &E.row[E.curY]; //Get current row

    switch (key)
    {
    case ARROW_LEFT: 
        if(E.curX != 0)
        {
            E.curX--; 
        }
        else if(E.curY > 0)
        {
            E.curY--;
            E.curX = E.row[E.curY].size; //Move to end of previous line
        }
        break;
    case ARROW_RIGHT: 
        if(row && E.curX < row->size)
        {
        E.curX++;
        } 
        else if(row && E.curX == row->size) //Move to beginning of next line
        {
            E.curY++;
            E.curX = 0;
        }
        break;
    case ARROW_UP: 
        if(E.curY !=  0)
        {
            E.curY--; 
        }
        break;
    case ARROW_DOWN: 
        if(E.curY < E.numRows)
        {
            E.curY++; 
        }
        break;
    }

    row = (E.curY >= E.numRows) ? NULL : &E.row[E.curY]; //Get current row after moving
    int rowLen = row ? row->size : 0;
    if(E.curX > rowLen)
    {
        E.curX = rowLen;
    }
}

char *editorPrompt(char *prompt, void(*callback)(char*,int))
{
    //Dynamic buffer for user input
    size_t bufferSize = 128;
    char *buffer = malloc(bufferSize);

    size_t bufferLen = 0;
    buffer[0] = '\0';

    while(1)
    {
        editorStatusMessage(prompt, buffer);
        editorRefreshScreen();

        int c = editorReadKey();

        if(c == DELETE_KEY || c == CTRL_KEY('h') || c == BACKSPACE) //Handle backspace
        {
            if(bufferLen != 0) buffer[--bufferLen] = '\0';
        }
        else if(c == '\x1b') //If escape key, cancel prompt
        {
            editorStatusMessage("");
            if(callback) callback(buffer, c); //Call callback with escape key
            free(buffer);
            return NULL;
        }
        else if(c == '\r') //if enter key
        {
            if(bufferLen != 0) //If buffer is not empty, return it
            {
                editorStatusMessage("");
                if(callback) callback(buffer, c);//Call callback with enter key
                return buffer;
            }
        }
        else if(!iscntrl(c) && c < 128) //If printable character
        {
            if(bufferLen == bufferSize - 1) //Resize buffer if needed
            {
                bufferSize *= 2;
                buffer = realloc(buffer, bufferSize);
            }
            buffer[bufferLen++] = c;
            buffer[bufferLen] = '\0';
        }
        if(callback) callback(buffer, c); //Call callback with current buffer and keypress
    }
}
