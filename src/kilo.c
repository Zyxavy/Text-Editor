#include "kilo.h"

struct editorConfig E;

//  init
void initEditor()
{
    if(getWindowSize(&E.screenRows, &E.screenCols) == -1) die("getWindowSize");
}

int main()
{
    enableRawMode();//Enable raw mode to disable echoing
    initEditor();

    while (1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
    } 
    return 0;
}

//  terminals
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
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);


    perror(s);
    exit(1);
}

char editorReadKey()
{
    int nread;
    char c;
    
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if(nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}

int getWindowSize(int *rows, int* *cols)
{
    struct winsize ws;

   if(1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        if(write(STDOUT_FILENO, "x1b[999C\x1b[999B", 12) != 12) return -1;
        
        editorReadKey();
        return -1; 
    }
   else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }

}

//  Input
void editorProcessKeypress()
{
    char c = editorReadKey();
    switch (c)
    {
    case CTRL_KEY('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1[H", 3); 
        exit(0); 
        break;
    }
}

//  Output
void editorRefreshScreen()
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1[H", 3);

    editorDrawRows();
    write(STDOUT_FILENO, "\x1b[H", 3);
}

void editorDrawRows()
{
    int y;
    for(y = 0; y < E.screenRows; y++)
    {
        write(STDOUT_FILENO, ">\r\n", 3);
    }

}
