#include "kilo.h"

// #===Find===#
void editorFind()
{
    //save positions
    int savedCurX = E.curX;
    int savedCurY = E.curY;
    int savedColOffset = E.colOffset;
    int savedRowOffset = E.rowOffset;

    char *query = editorPrompt("Search: %s (Use ESC/Arrows/Enter)", editorFindCallback); //Prompt for search query with callback
    
    if(query)
    {
        free(query);
    }
    else //If search was cancelled, restore cursor position
    {
        E.curX = savedCurX;
        E.curY = savedCurY;
        E.colOffset = savedColOffset;
        E.rowOffset = savedRowOffset;
    }
}

void editorFindCallback(char *query, int key)
{
    static int lastMatch = -1;
    static int direction = 1;

    static int savedHLLine;
    static char *savedHL = NULL;

    if(savedHL)
    {
        erow *row = &E.row[savedHLLine];

        if (row->highlight) memcpy(row->highlight, savedHL, row->rSize); //Restore previous highlight

        free(savedHL);
        savedHL = NULL;
    }

    if(key == '\r' || key == '\x1b')//Enter or Escape key
    {
        lastMatch = -1;
        direction = 1;
        return;
    }
    else if(key == ARROW_RIGHT || key == ARROW_DOWN) //Next match
    {
        direction = 1;
    }
    else if(key == ARROW_LEFT || key == ARROW_UP) //Previous match
    {
        direction = -1;
    } 
    else //New search, reset state
    {
        lastMatch = -1;
        direction = 1;
    }
    
    if(lastMatch == -1) direction = 1;
    int current = lastMatch;

    for (int i = 0; i < E.numRows; i++)
    {
        current += direction;
        if(current == -1) current = E.numRows - 1; //Wrap around to last row
        else if(current == E.numRows) current = 0; //Wrap around to first row

        erow *row = &E.row[current]; //Get current row
        char *match = strstr(row->render, query); //Search for query in rendered row
        
        if(match) //If match found
        {
            lastMatch = current;
            E.curY = current;
            E.curX = editorRowRenderXToCurX(row, match - row->render);
            E.rowOffset = E.numRows;

            if (row->highlight) //Save current highlight state
            {
                savedHLLine = current;
                savedHL = malloc(row->rSize);
                memcpy(savedHL, row->highlight, row->rSize);
            }

            int start = match - row->render;
            int qlen = strlen(query);

            if (start + qlen > row->rSize) qlen = row->rSize - start;  

            memset(&row->highlight[start], HL_MATCH, qlen);//highlight match
            break;
        }
    }

}
