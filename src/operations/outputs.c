#include "kilo.h"

//  #===Output===#
void editorRefreshScreen()
{
    editorScroll();

    struct appendbuff ab = ABUF_INIT; //Initialize append buffer

    abAppend(&ab, "\x1b[?25l", 6); //Hide cursor
    abAppend(&ab, "\x1b[H", 3); //Reposition cursor to top-left

    editorDrawRows(&ab); //Draw rows
    editorDrawStatusBar(&ab); //Draw status bar
    editorDrawMessageBar(&ab); //Draw message bar

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "\x1b[%d;%dH", (E.curY - E.rowOffset) + 1, (E.renderX - E.colOffset) + 1); //Reposition cursor
    abAppend(&ab, buffer, strlen(buffer)); //Append cursor position command

    abAppend(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.b, ab.len); //Write buffer to stdout
    abFree(&ab);
}

void editorDrawRows(struct appendbuff *ab)
{
    int y;
    for(y = 0; y < E.screenRows; y++) //For each row
    {
        int fileRow = y + E.rowOffset;
        if(fileRow >= E.numRows)
        {
            if(E.numRows == 0 && y == E.screenRows / 3) //Display welcome message on one-third down the screen
            {
                char welcome[80];
                int welcomeLen = snprintf(welcome, sizeof(welcome), "Kilo editor -- version %s", KILO_VERSION);

                if(welcomeLen > E.screenCols) welcomeLen = E.screenCols; //Trim if too long
                int padding = (E.screenCols - welcomeLen) / 2; //Calculate left padding

                if(padding) //Add '>' before welcome message
                {
                    abAppend(ab, ">", 1); 
                    padding--;
                }
                while(padding--) abAppend(ab, " ", 1); //Add left padding

                abAppend(ab, welcome, welcomeLen);
            }
            else
            {
            abAppend(ab, ">", 1);
            }
        }
        else
        {
            int len = E.row[fileRow].rSize - E.colOffset; //Calculate length to render
            if(len < 0) len = 0;
            if(len > E.screenCols) len = E.screenCols;

            char *c = &E.row[fileRow].render[E.colOffset]; //Get pointer to start of render
            unsigned char *highlight = &E.row[fileRow].highlight[E.colOffset];//Get pointer to highlight
            int curColor = -1;

            for(int j = 0; j < len; j++)
            {
                if(iscntrl(c[j])) //If control character
                {
                    char symbol = (c[j] <= 26) ? '@' + c[j] : '?';
                    abAppend(ab, "\x1b[7m", 4);
                    abAppend(ab, &symbol, 1);
                    abAppend(ab, "\x1b[m", 3);

                    if(curColor != -1) //Restore color
                    {
                        char buffer[16];
                        int cLen = snprintf(buffer, sizeof(buffer), "\x1b[%dm", curColor);
                        abAppend(ab, buffer, cLen);
                    }
                }
                else if(highlight[j] == HL_NORMAL)
                {
                    if(curColor != -1) //If current color is not default
                    {
                        abAppend(ab, "\x1b[39m", 5); //Reset to default color
                        curColor = -1;
                    }
                    abAppend(ab, &c[j], 1);
                }
                else
                {
                    int color = editorSyntaxToColor(highlight[j]);

                    if(color != curColor) //If color has changed
                    {
                        curColor = color;
                        char buffer[16];
                        int charLen = snprintf(buffer, sizeof(buffer), "\x1b[%dm", color); //Set color
                        abAppend(ab, buffer, charLen);
                    }
                    abAppend(ab, &c[j], 1);
                }
            }
            abAppend(ab, "\x1b[39m", 5);
        }
        abAppend(ab, "\x1b[K", 3);//Clear line
        abAppend(ab, "\r\n", 2);
        
    }

}

void editorScroll()
{
    E.renderX = 0;
    if(E.curY < E.numRows) //If cursor is within the file rows
    {
        E.renderX = editorRowCurXToRenderX(&E.row[E.curY], E.curX); //Convert curX to renderX considering tabs
    }

    if(E.curY < E.rowOffset) //Scroll up
    {
        E.rowOffset = E.curY;
    }
    if(E.curY >= E.rowOffset + E.screenRows) //Scroll down
    {
        E.rowOffset = E.curY - E.screenRows + 1;
    }
    if(E.renderX < E.colOffset) //Scroll left
    {
        E.colOffset = E.renderX;
    }
    if(E.renderX >= E.colOffset + E.screenCols) //Scroll right
    {
        E.colOffset = E.renderX - E.screenCols + 1;
    }
}

void editorDrawStatusBar(struct appendbuff * ab)
{
    abAppend(ab, "\x1b[7m", 4); // Switch to inverted colors
    
    char status[80], renderStatus[80];
    
    //Create status message
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s", 
                      E.fileName ? E.fileName : "[No Name]", E.numRows, E.dirty ? "(Modified)" : " "); 

    //Create render message
    int renderLen = snprintf(renderStatus, sizeof(renderStatus), "%s | %d/%d", 
                            E.syntax ? E.syntax->filetype : "No file type",  E.curY + 1, E.numRows);
    
    if (len > E.screenCols) len = E.screenCols;
    

    int statusWidth = E.screenCols - renderLen;
    if (statusWidth < 0) statusWidth = 0;
    
    if (len > statusWidth) len = statusWidth;
    abAppend(ab, status, len);
    
    while (len < statusWidth) 
    {
        abAppend(ab, " ", 1);
        len++;
    }
    
    abAppend(ab, renderStatus, renderLen);
    
    abAppend(ab, "\x1b[m", 3); // Reset colors
    abAppend(ab, "\r\n", 2);
}

void editorStatusMessage(const char* fmt, ...)
{
    va_list ap; //Variable argument list
    va_start(ap, fmt); //Initialize argument list
    vsnprintf(E.statusMsg, sizeof(E.statusMsg), fmt, ap); //Format the message
    va_end(ap); //Clean up argument list
    E.statusMsgTime = time(NULL); //Record time message was set
}

void editorDrawMessageBar(struct appendbuff *ab)
{
    abAppend(ab, "\x1b[K", 3); //Clear message bar line
    int msgLen = strlen(E.statusMsg); //Get length of status message
    
    if(msgLen > E.screenCols) msgLen = E.screenCols;
    if(msgLen && time(NULL) - E.statusMsgTime < 5) abAppend(ab, E.statusMsg, msgLen); //Display message if within 5 seconds
}
