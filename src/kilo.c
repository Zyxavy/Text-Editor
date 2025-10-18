#include "kilo.h"

int main()
{
    enableRawMode();//Enable raw mode to disable echoing

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q'); //Read input until 'q' is pressed
    return 0;
}

void enableRawMode()
{
    tcgetattr(STDIN_FILENO, &original_termios); //Get the current terminal attributes
    atexit(disableRawMode); //Ensure raw mode is disabled on exit

    struct termios raw = original_termios; //Make a copy to modify
    raw.c_lflag &= ~(ECHO | ICANON); //Disable echoing and canonical mode

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); //Set the new attributes
}

void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios); //Restore original attributes
}