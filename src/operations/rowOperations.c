#include "kilo.h"


// #===Row Operations==#
void editorInsertRow(int at, char *s, size_t len)
{
    if(at < 0 || at > E.numRows) return;

    E.row = realloc(E.row, sizeof(erow) * (E.numRows + 1));//Resize row array to fit new row
    memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numRows - at)); //Shift rows down to make space

    for(int j = at + 1; j <= E.numRows; j++) E.row[j].index++; //Update row indices

    E.row[at].index = at;

    E.row[at].size = len; //Set size of new row
    E.row[at].chars = malloc(len + 1); //Allocate memory for row characters
    memcpy(E.row[at].chars, s, len); //Copy characters into row
    E.row[at].chars[len] = '\0'; //Null-terminate the string

    E.row[at].rSize = 0;
    E.row[at].render = NULL;
    E.row[at].highlight = NULL;
    E.row[at].hlOpenComment = 0;
    editorUpdateRow(&E.row[at]);

    E.numRows++; //Increment number of rows
    E.dirty++;
}

void editorUpdateRow(erow *row)
{
    int tabs = 0;
    int j;
    for(j = 0; j < row->size; j++) //Count number of tabs
    {
        if(row->chars[j] == '\t') tabs++; //Each tab will expand to multiple spaces
    }

    free(row->render);
    row->render = malloc(row->size + tabs * (KILO_TAB_STOP - 1) + 1); //Allocate memory for rendered row
    
    int idx = 0;
    for(j = 0; j < row->size; j++) //For each character in the original row
    {
        if(row->chars[j] == '\t')
        {
            row->render[idx++] = ' ';
            while(idx % KILO_TAB_STOP != 0) row->render[idx++] = ' '; //Expand tab to spaces
        }
        else
        {
            row->render[idx++] = row->chars[j]; //Copy character
        }
    }

    row->render[idx] = '\0';
    row->rSize = idx; //Set rendered size
    editorUpdateSyntax(row);
}

int editorRowCurXToRenderX(erow *row, int curX)
{
    int renderX = 0;
    for(int j = 0; j < curX; j++)
    {
        if(row->chars[j] == '\t') renderX += (KILO_TAB_STOP - 1) - (renderX % KILO_TAB_STOP); //Account for tab expansion
        renderX++;
    }
    return renderX;
}

int editorRowRenderXToCurX(erow *row, int rx)
{
    int curRX = 0;
    
    int cx;
    for(cx = 0; cx < row->size; cx++)
    {
        if(row->chars[cx] == '\t')
        {
            curRX += (KILO_TAB_STOP - 1) - (curRX & KILO_TAB_STOP); //Account for tab expansion
        }
        curRX++;

        if(curRX > rx) return cx;
    }
    return cx;
}

void editorRowInsertChar(erow *row, int at, int c)
{
    if(at < 0 || at > row->size) at = row->size; //Clamp 'at' to valid range

    row->chars = realloc(row->chars, row->size + 2); //Resize character array to fit new character
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1); //Shift characters to the right
    row->size++;
    row->chars[at] = c;
    editorUpdateRow(row);
    E.dirty++;
}

void editorRowDeleteChar(erow *row, int at)
{
    if(at < 0 || at >= row->size) return; //Clamp 'at' to valid range

    memmove(&row->chars[at], &row->chars[at+1], row->size - at); //Shift characters to the left
    row->size--;
    editorUpdateRow(row);
    E.dirty++;
}

void editorFreeRow(erow *row)
{
    free(row->render);
    free(row->chars);
    free(row->highlight);
}

void editorDeleteRow(int at)
{
    if(at < 0 || at >= E.numRows) return; 

    editorFreeRow(&E.row[at]);
    memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numRows - at - 1)); //Shift rows up
    for (int j = at; j < E.numRows - 1; j++) E.row[j].index--;
    E.numRows--;
    E.dirty++;
}

void editorRowAppendString(erow *row, char *s, size_t len)
{
    row->chars = realloc(row->chars, row->size + len + 1); //Resize character array to fit new string
    mempcpy(&row->chars[row->size], s, len); //Append new string
    row->size += len;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
    E.dirty++;
}

