<h1>Incomplete Documentation</h1>

<p>read() and STDIN_FILENO come from <unistd.h>. We are asking read() to read 1 byte from the standard input into the variable c, and to keep doing it until there are no more bytes to read. read() returns the number of bytes that it read, and will return 0 when it reaches the end of a file.

Terminal attributes can be read into a termios struct by tcgetattr(). After modifying them, you can then apply them to the terminal using tcsetattr(). The TCSAFLUSH argument specifies when to apply the change: in this case, it waits for all pending output to be written to the terminal, and also discards any input that hasn’t been read.

The c_lflag field is for “local flags”.

The CTRL_KEY macro bitwise-ANDs a character with the value 00011111, in binary. 

editorReadKey()’s job is to wait for one keypress, and return it.

editorProcessKeypress() waits for a keypress, and then handles it. 

The 4 in our write() call means we are writing 4 bytes out to the terminal. The first byte is \x1b, which is the escape character, or 27 in decimal. The other three bytes are [2J.

"\x1[H" escape sequence is only 3 bytes long, and uses the H command (Cursor Position) to position the cursor.

editorDrawRows() will handle drawing each row of the buffer of text being edited.

ioctl(), TIOCGWINSZ, and struct winsize come from <sys/ioctl.h>.
ioctl() will place the number of columns wide and the number of rows high the terminal is into the given winsize struct. On failure, ioctl() returns -1.

initEditor()’s job will be to initialize all the fields in the E struct.

The C command (Cursor Forward) moves the cursor to the right, and the B command (Cursor Down) moves the cursor down. The argument says how much to move it right or down by. We use a very large value, 999, which should ensure that the cursor reaches the right and bottom edges of the screen.


</p>

editorDrawRows()  draws a '>' in each row,
created global editor struct
initialize all fields with initEditor()
Get the size of the window with getWindowSize()
