#include "kilo.h"

struct editorConfig E;

void editorSetStatusMessage(const char *fmt, ...)
{
    return;
} //prototype 


//  #===Init===#
void initEditor()
{
    //Initialize editor state to 0 or NULL
    E.curX = 0;
    E.curY = 0;
    E.renderX = 0;
    E.rowOffset = 0;
    E.colOffset = 0;
    E.numRows = 0;
    E.row = NULL;
    E.fileName = NULL;
    E.statusMsg[0] = '\0';
    E.statusMsgTime = 0;
    E.dirty = 0;

    if(getWindowSize(&E.screenRows, &E.screenCols) == -1) die("getWindowSize");//Get terminal size
    E.screenRows -= 2;
}

int main(int argc, char*argv[])
{
    enableRawMode();//Enable raw mode to disable echoing
    initEditor();
    if(argc >= 2)
    {
        editorOpen(argv[1]);
    }

    editorStatusMessage("HELP: Ctrl+S = Save | Ctrl+Q = Quit | Ctrl+F = Find");

    while (1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
    } 
    return 0;
}


//  #===Terminals===#
void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &E.original_termios) == -1) die("tcgetattr"); //Get current terminal attributes
    atexit(disableRawMode); //Ensure raw mode is disabled on exit

    struct termios raw = E.original_termios; //Make a copy to modify
    
    //Modify the terminal attributes to enable raw mode
    raw.c_iflag &= ~(ICRNL | IXON | BRKINT | INPCK | ISTRIP); 
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag &= ~(CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); 
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr"); //Set the new attributes
}

void disableRawMode()
{
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.original_termios) == -1) die("tcsetattr"); //Restore original terminal attributes

}

void die(const char* s)
{   
    write(STDOUT_FILENO, "\x1b[2J", 4);//Clear screen
    write(STDOUT_FILENO, "\x1b[H", 3);//Reposition cursor to top-left

    perror(s);
    exit(1);
}

int editorReadKey()
{
    int nread;
    char c;
    
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) //Read a single keypress
    {
        if(nread == -1 && errno != EAGAIN) die("read");//On error, die
    }
    
    if(c == '\x1b') //If an esape sequence,
    {
        char seqBuffer[3]; 

        if(read(STDIN_FILENO, &seqBuffer[0], 1) != 1) return '\x1b';
        if(read(STDIN_FILENO, &seqBuffer[1], 1) != 1) return '\x1b';

        if(seqBuffer[0] == '[') //if page up or down
        {
            if(seqBuffer[1] >= '0' && seqBuffer[1] <= '9')
            {
                if(read(STDIN_FILENO, &seqBuffer[2], 1) != 1) return '\x1b';
                if(seqBuffer[2] == '~')
                {
                    switch (seqBuffer[1])
                    {
                        case '1': return HOME_KEY;
                        case '3': return DELETE_KEY;
                        case '4': return END_KEY; 
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                }
            }
            else //else Arrow keys
            {
                switch (seqBuffer[1])
                {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                }
            }
        } 
        else if(seqBuffer[0] == '0')
        {
            switch (seqBuffer[1])
            {
                case 'H': return HOME_KEY;
                case 'f': return END_KEY;
            }
        }

        return '\x1b';
    } 
    else
    {
        return c;
    }
}

int getWindowSize(int *rows, int *cols)
{
    struct winsize ws;

   if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) //If ioctl fails
    {
        if(write(STDOUT_FILENO, "x1b[999C\x1b[999B", 12) != 12) return -1; //Move cursor to bottom-right

        return getCursorPosition(rows, cols); 
    }
   else
    {
        *cols = ws.ws_col; //Set columns and rows
        *rows = ws.ws_row;
        return 0;
    }

}

int getCursorPosition(int *rows, int *cols)
{
    char buffer[32];
    unsigned int i = 0;

    if(write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1; //Request cursor position

    while(i < sizeof(buffer) - 1) 
    {
        if(read(STDIN_FILENO, &buffer[i], 1) != 1) break; //On error, break
        if(buffer[i] == 'R') break;
        i++;
    }
    buffer[i] = '\0';

    if(buffer[0] != '\x1b' || buffer[1] != '[') return -1; //Invalid response
    if(sscanf(&buffer[2], "%d;%d", rows, cols) != 2) return -1;//Parse rows and cols

    return 0;
}


// #===Append Buffer===# 
void abAppend(struct appendbuff *ab, const char *s, int len)
{
    char *new = realloc(ab->b, ab->len + len); //Resize buffer

    if(new == NULL) return;
    memcpy(&new[ab->len], s, len); //Append new data
    ab->b = new;
    ab->len += len;
}

void abFree(struct appendbuff *ab)
{
    free(ab->b);
}


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
                editorSetStatusMessage("WARNING! File has unsaved changes. " "Press Ctrl-Q %d more times to quit.", quitTimes);
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
        editorSetStatusMessage(prompt, buffer);
        editorRefreshScreen();

        int c = editorReadKey();

        if(c == DELETE_KEY || c == CTRL_KEY('h') || c == BACKSPACE) //Handle backspace
        {
            if(bufferLen != 0) buffer[--bufferLen] = '\0';
        }
        else if(c == '\x1b') //If escape key, cancel prompt
        {
            editorSetStatusMessage("");
            if(callback) callback(buffer, c); //Call callback with escape key
            free(buffer);
            return NULL;
        }
        else if(c == '\r') //if enter key
        {
            if(bufferLen != 0) //If buffer is not empty, return it
            {
                editorSetStatusMessage("");
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
                if(highlight[j] == HL_NORMAL)
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

    //Create render position message
    int renderLen = snprintf(renderStatus, sizeof(renderStatus), "%d/%d", 
                            E.curY + 1, E.numRows);
    
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
    abAppend(ab, "x1b[K", 3); //Clear message bar line
    int msgLen = strlen(E.statusMsg); //Get length of status message
    
    if(msgLen > E.screenCols) msgLen = E.screenCols;
    if(msgLen && time(NULL) - E.statusMsgTime < 5) abAppend(ab, E.statusMsg, msgLen); //Display message if within 5 seconds
}


// #===File I/O===#
void editorOpen(char *fileName)
{
    free(E.fileName);
    E.fileName = strdup(fileName); //Store file name

    FILE *fp = fopen(fileName, "r"); //Open file for reading, if fails then call 'die'
    if(!fp) die("fopen");

    char *line = NULL;
    size_t lineCap = 0;
    ssize_t lineLen;
    while ((lineLen = getline(&line, &lineCap, fp)) != 1) //Read each line
    {
        while(lineLen > 0 && (line[lineLen - 1] == '\n' || line[lineLen - 1] == '\r')) //Trim newline characters
        {
            lineLen--; 
        }
        editorInsertRow(E.numRows, line, lineLen);//Append row to editor
    } 
    free(line);
    fclose(fp); //Close file
    E.dirty = 0;
}

char *editorRowsToString(int *bufferlen)
{
    int totalLen = 0;
    int i;
    for(i = 0; i < E.numRows; i++) totalLen += E.row[i].size + 1; //Calculate total length including newlines
    *bufferlen = totalLen;

    char *buffer = malloc(totalLen);
    char *p = buffer;

    for(i = 0; i < E.numRows; i++) //For each row
    {
        memcpy(p, E.row[i].chars, E.row[i].size); //Copy row characters
        p += E.row[i].size; //Move pointer forward
        *p = '\n'; //Add newline
        p++;
    }
    
    return buffer;
}

void editorSave()
{
    if(E.fileName == NULL)
    {
        E.fileName = editorPrompt("Save as: %s (ESC to cancel)", NULL);

        if(E.fileName == NULL)
        {
            editorSetStatusMessage("Save aborted!");
            return;
        }
    }

    int len;
    char *buffer = editorRowsToString(&len); //Convert rows to string

    int fd = open(E.fileName, O_RDWR | O_CREAT, 0644); //Open file for reading and writing, create if it doesn't exist
    if(fd != -1)
    {
        if(ftruncate(fd, len) != -1) //Truncate file to new length
        {
            if(write(fd, buffer, len) == len) //write buffer to file
            {
                close(fd);
                free(buffer);
                E.dirty = 0;
                editorStatusMessage("%d bytes written into disk", len);
                return;
            }
        }
        close(fd);
    }
    free(buffer);
    editorStatusMessage("Cant Save! I/O error: %s", strerror(errno));
}


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
        mempcpy(&E.row[savedHLLine].highlight, savedHL, E.row[savedHLLine].rSize); //Restore previous highlight
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

            savedHLLine = current;
            savedHL = malloc(row->rSize);
            memcpy(savedHL, row->highlight, row->rSize); //Save current highlight
            memset(&row->highlight[match - row->render], HL_MATCH, strlen(query)); //Highlight match
            break;
        }
    }

}

// #===Row Operations==#
void editorInsertRow(int at, char *s, size_t len)
{
    if(at < 0 || at > E.numRows) return;

    E.row = realloc(E.row, sizeof(erow) * (E.numRows + 1));//Resize row array to fit new row
    memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numRows - at)); //Shift rows down to make space

    E.row[at].size = len; //Set size of new row
    E.row[at].chars = malloc(len + 1); //Allocate memory for row characters
    memcpy(E.row[at].chars, s, len); //Copy characters into row
    E.row[at].chars[len] = '\0'; //Null-terminate the string

    E.row[at].rSize = 0;
    E.row[at].render = NULL;
    E.row[at].highlight = NULL;
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


// #===Syntax Highlight===#

void editorUpdateSyntax(erow *row)
{
    row->highlight = realloc(row->highlight, row->rSize); 
    memset(row->highlight, HL_NORMAL, row->rSize); //Default all to normal

    int prevSep = 1;

    int i = 0;
    while(i < row->rSize)
    {
        char c = row->render[i];
        unsigned char prevHL = (i > 0) ? row->highlight[i-1] : HL_NORMAL; //Get previous highlight

        if(isdigit(c) && (prevSep || prevHL == HL_NUMBER) || 
            (c == '.' && prevHL == HL_NUMBER)) //If digit or part of a number
        {
            row->highlight = HL_NUMBER; //Highlight numbers
            i++;
            prevSep = 0;
            continue;
        }
        prevSep = isSeparator(c);
        i++;
    }
}

int editorSyntaxToColor(int highlight)
{
    switch (highlight)
    {
        case HL_NUMBER: return 31;
        case HL_MATCH: return 34;
        default: return 37;
    }
}

int isSeparator(int c)
{
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}
