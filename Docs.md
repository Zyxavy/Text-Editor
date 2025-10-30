# Incomplete Documentation

## Kilo Technical Documentation>

This document provides a detailed explanation of the internal workings of the Kilo text editor. It covers terminal manipulation, input/output handling, and screen rendering.

### 1. Terminal Handling (termios.h)

The editor needs to precisely control the terminal's behavior. This is primarily achieved using the <termios.h> library.

#### Raw Mode

By default, the terminal operates in "canonical mode" (or cooked mode), where keyboard input is processed line by line. To process each keypress as it happens, we must enable "raw mode".
```
enableRawMode():
```

1. Reads the current terminal attributes into a termios struct using tcgetattr(). A copy of these original attributes is saved.

2. Sets an atexit(disableRawMode) handler to ensure that even if the program crashes, the terminal settings are restored.

3. Modifies the termios struct to disable several flags:

- ECHO: Prevents keys from being printed to the terminal as they are typed.

- ICANON: Disables canonical mode, allowing us to read input byte-by-byte.

- ISIG: Disables signals like Ctrl-C (SIGINT) and Ctrl-Z (SIGTSTP).

- IXON: Disables software flow control (Ctrl-S and Ctrl-Q).

- IEXTEN: Disables Ctrl-V.

- ICRNL: Prevents the terminal from translating carriage returns (\r) into newlines (\n).

- OPOST: Disables all output processing, preventing translation of \n to \r\n.

4. Applies the modified attributes to the terminal using tcsetattr(). The TCSAFLUSH argument ensures the changes are applied after all pending output is written and any unread input is discarded.

- disableRawMode(): This function is called on exit to restore the terminal's original attributes, ensuring the user's shell works correctly after the editor closes.

### 2. Input Processing

#### Reading Keystrokes

- editorReadKey(): This function waits for a single keypress and returns it. It uses read() from <unistd.h> to read 1 byte from standard input. It loops until a byte is read, handling potential errors.

- editorProcessKeypress(): This function calls editorReadKey() and then uses a switch statement to handle the input. Currently, it handles Ctrl-Q to exit and w/a/s/d for cursor movement.

- CTRL_KEY(k) Macro: This macro simulates the Ctrl key modifier. It works by performing a bitwise-AND with 0x1f (binary 00011111), which sets the upper 3 bits of the character's byte to 0. This mimics how the Ctrl key was traditionally handled in terminals.

### 3. Screen Rendering and Output

To avoid screen flicker and perform efficient updates, the editor builds the entire screen content in memory before writing it to the terminal in a single operation.

#### The Append Buffer (struct appendbuff)

This dynamic string structure is used to buffer all the output.

- abAppend(ab, s, len): Appends a string s of length len to the buffer. It uses realloc() to resize the memory block holding the buffer's content.

- abFree(ab): Frees the memory allocated for the buffer.

#### VT100 Escape Sequences

The editor uses standard VT100 escape sequences to control the terminal's cursor and appearance. These are special character sequences that start with \x1b (the escape character).

- \x1b[2J: Clear the entire screen.

- \x1b[K: Clear the current line from the cursor to the end.

- \x1b[H: Position the cursor at the top-left corner (home position).

- \x1b[?25l: Hide the cursor.

- \x1b[?25h: Show the cursor.

- \x1b[<row>;<col>H: Position the cursor at a specific row and col.

#### The Rendering (editorRefreshScreen)

This is the main rendering function, called in the main loop.

1. Initializes an append buffer ab.

2. Hides the cursor (\x1b[?25l) to prevent it from flickering during the redraw.

3. Positions the cursor at the home position (\x1b[H).

4. Calls editorDrawRows() to draw the content of the screen into the buffer.

5. Calculates the new cursor position and generates the escape sequence to move it there (\x1b[<E.curY + 1>;<E.curX + 1>H).

6. Shows the cursor again (\x1b[?25h).

7. Writes the entire content of the append buffer to standard output with a single write() call.

8. Frees the append buffer.

### 4. Window Size and Cursor Position

The editor must know the dimensions of the terminal window to render correctly.

- getWindowSize():

1. The primary method is to use ioctl() with TIOCGWINSZ. This system call fills a winsize struct with the number of rows and columns.

2. If ioctl() fails, it uses a fallback method: it moves the cursor to the bottom-right of the screen (\x1b[999C\x1b[999B) and then queries the cursor's position.

- getCursorPosition(): This function is the fallback for getWindowSize().

1. It writes an escape sequence (\x1b[6n) that asks the terminal to report its cursor position.

2. It then reads the response from standard input, which is formatted as \x1b[<rows>;<cols>R.

3. It parses this response using sscanf() to extract the row and column numbers.

.....