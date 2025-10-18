<h1>Incomplete Documentation</h1>

<p>read() and STDIN_FILENO come from <unistd.h>. We are asking read() to read 1 byte from the standard input into the variable c, and to keep doing it until there are no more bytes to read. read() returns the number of bytes that it read, and will return 0 when it reaches the end of a file.

Terminal attributes can be read into a termios struct by tcgetattr(). After modifying them, you can then apply them to the terminal using tcsetattr(). The TCSAFLUSH argument specifies when to apply the change: in this case, it waits for all pending output to be written to the terminal, and also discards any input that hasn’t been read.

The c_lflag field is for “local flags”.
</p>

Error handling

First, we’ll add a die() function that prints an error message and exits the program.



