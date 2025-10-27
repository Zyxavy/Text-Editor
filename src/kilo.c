#include "kilo.h"

struct editorConfig E;

//  #===Init===#
void initEditor()
{
    E.curX = 0;
    E.curY = 0;
    E.rowOffset = 0;
    E.colOffset = 0;
    E.numRows = 0;
    E.row = NULL;

    if(getWindowSize(&E.screenRows, &E.screenCols) == -1) die("getWindowSize");//Get terminal size

}

int main(int argc, char*argv[])
{
    enableRawMode();//Enable raw mode to disable echoing
    initEditor();
    if(argc >= 2)
    {
        editorOpen(argv[1]);
    }

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
        *cols = ws.ws_col;
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
    int c = editorReadKey();
    switch (c)
    {
        case CTRL_KEY('q'): //Quit on Ctrl-Q
            write(STDOUT_FILENO, "\x1b[2J", 4); 
            write(STDOUT_FILENO, "\x1[H", 3); 
            exit(0); 
            break;

        case HOME_KEY:
            E.curX = 0;
            break;
        case END_KEY:
            E.curX = E.screenCols - 1;
            break;
        
        case PAGE_UP:
        case PAGE_DOWN:
            {
                int times = E.screenRows;
                while(times--) editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            break;

        case ARROW_LEFT: 
        case ARROW_RIGHT: 
        case ARROW_UP: 
        case ARROW_DOWN:
            editorMoveCursor(c);
            break;
    }
}

void editorMoveCursor(int key)
{
    switch (key)
    {
    case ARROW_LEFT: 
        if(E.curX != 0)
        {
            E.curX--; 
        }
        break;
    case ARROW_RIGHT: 
        E.curX++; 
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
}

//  #===Output===#
void editorRefreshScreen()
{
    editorScroll();

    struct appendbuff ab = ABUF_INIT; //Initialize append buffer

    abAppend(&ab, "\x1b[?25l", 6); //Hide cursor
    abAppend(&ab, "\x1b[H", 3); //Reposition cursor to top-left

    editorDrawRows(&ab); //Draw rows

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "\x1b[%d;%dH", (E.curY - E.rowOffset) + 1, (E.curX - E.colOffset) + 1); //Reposition cursor
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

                if(padding)
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
            int len = E.row[fileRow].size - E.colOffset;
            if(len < 0) len = 0;
            if(len > E.screenCols) len = E.screenCols;
            abAppend(ab, &E.row[fileRow].chars[E.colOffset], len);
        }
        abAppend(ab, "\x1b[K", 3);//Clear line
        if(y < E.screenRows - 1) //Avoid adding a new line on the last row
        {
            abAppend(ab, "\r\n", 2);
        }
    }

}

void editorScroll()
{
    if(E.curY < E.rowOffset)
    {
        E.rowOffset = E.curY;
    }
    if(E.curY >= E.rowOffset + E.screenRows)
    {
        E.rowOffset = E.curY - E.screenRows + 1;
    }
    if(E.curX < E.colOffset)
    {
        E.colOffset = E.curX;
    }
    if(E.curX >= E.colOffset + E.screenCols)
    {
        E.colOffset = E.curX - E.screenCols + 1;
    }
}

// #===File I/O===#
void editorOpen(char *fileName)
{
    FILE *fp = fopen(fileName, "r");
    if(!fp) die("fopen");

    char *line = NULL;
    size_t lineCap = 0;
    ssize_t lineLen;
    while ((lineLen = getline(&line, &lineCap, fp)) != 1)
    {
        while(lineLen > 0 && (line[lineLen - 1] == '\n' || line[lineLen - 1] == '\r')) //Trim newline characters
        {
            lineLen--;
        }
        editorAppendRow(line, lineLen);
    } 
    free(line);
    fclose(fp);
}

void editorAppendRow(char *s, size_t len)
{
    E.row = realloc(E.row, sizeof(erow) * (E.numRows + 1));

    int at =  E.numRows;
    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';
    E.numRows++;
}