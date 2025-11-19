#include "kilo.h"

// #===Editor Operations===#
void editorInsertChar(int c)
{
    if(E.curY == E.numRows) //If cursor is at the end, append a new row
    {
        editorInsertRow(E.numRows, "", 0);
    }

    editorRowInsertChar(&E.row[E.curY], E.curX, c); //Insert character at cursor position
    E.curX++;
}

void editorDeleteChar()
{
    if(E.curY == E.numRows) return; //If cursor is at the end, nothing to delete
    if(E.curX == 0 && E.curY == 0) return;

    erow *row = &E.row[E.curY]; //Get current row
    if(E.curX > 0) //If not at the beginning of the line, delete character before cursor
    {
        editorRowDeleteChar(row, E.curX - 1);
        E.curX--;
    }
    else //If at the beginning of the line, merge with previous line
    {
        E.curX = E.row[E.curY - 1].size;
        editorRowAppendString(&E.row[E.curY - 1], row->chars, row->size);
        editorDeleteRow(E.curY);
        E.curY--;
    }
}

void editorInsertNewLine()
{
    if(E.curX == 0) //If at the beginning of the line, insert a new empty row above
    {
        editorInsertRow(E.curY, "", 0);
    }
    else //Otherwise, split the current row
    {
        erow *row = &E.row[E.curY];
        editorInsertRow(E.curY + 1, &row->chars[E.curX], row->size - E.curX);
        row = &E.row[E.curY];
        row->size = E.curX;
        row->chars[row->size] = '\0';
        editorUpdateRow(row);
    }
    E.curY++;
    E.curX = 0;
}
