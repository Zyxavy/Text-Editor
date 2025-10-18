#include "kilo.h"

int main()
{
    enableRawMode();//Enable raw mode to disable echoing

    
    while (1)
    {
        char c = '\0';
        read(STDIN_FILENO, &c, 1);

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

void enableRawMode()
{
    tcgetattr(STDIN_FILENO, &original_termios); //Get the current terminal attributes
    atexit(disableRawMode); //Ensure raw mode is disabled on exit

    struct termios raw = original_termios; //Make a copy to modify
    
    //Modify the terminal attributes to enable raw mode
    raw.c_iflag &= ~(ICRNL | IXON | BRKINT | INPCK | ISTRIP); 
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag &= ~(CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); 
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); //Set the new attributes
}

void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios); //Restore original attributes
}