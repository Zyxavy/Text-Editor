#include "kilo.h"

//  init
int main()
{
    enableRawMode();//Enable raw mode to disable echoing

    
    while (1)
    {
        char c = '\0';
        if(read(STDIN_FILENO, &c, 1) == 1 && errno != EAGAIN) die("read");

        if(iscntrl(c)) //Check if character is a control character
        {
            printf("%d\r\n", c);
        }
        else
        {
            printf("%d ('%c')\r\n", c, c);
        }
        if(c == 'q') break;
    } 
    return 0;
}


//  terminals
void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) die("tcgetattr"); //Get current terminal attributes
    atexit(disableRawMode); //Ensure raw mode is disabled on exit

    struct termios raw = original_termios; //Make a copy to modify
    
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
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1) die("tcsetattr"); //Restore original terminal attributes

}
void die(const char* s)
{
    perror(s);
    exit(1);
}


