# Kilo Text Editor Technical Documentation
-----------------------------------

## Overview

This editor is a minimal terminal-based text editor modeled after the kilo editor by antirez. It supports:

- Moving the cursor with arrow keys
- Opening and saving files
- Rendering text with scroll support
- Rendering a status bar and message bar

The source code is broken into two main files:

- kilo.h — Header file that declares types, constants, and function prototypes
- kilo.c — Main source file that implements the editor functionalities

-----------------------------------

## Headers, Macros, and Constants

### Feature Test Macros

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

These macros enable certain POSIX, BSD, and GNU extensions.

### Included Headers

- Standard I/O: stdio.h
- Terminal control: termios.h, unistd.h, sys/ioctl.h
- Error handling: errno.h, string.h, stdarg.h
- Time management: time.h
- File handling: fcntl.h

### Defined Macros

Macro: CTRL_KEY(k)
Usage: Converts a character to its Ctrl-key value

Macro: KILO_VERSION
Usage: Application Version

Macro: KILO_TAB_STOP
Usage: Number of spaces represented by a tab (default: 8)

Macro: ABUF_INIT
Usage: Initializes an append buffer

-----------------------------------

## Data Structures

### struct editorConfig

Holds the current state of the editor:

struct editorConfig {
    int curX, curY;          // Cursor position
    int renderX;             // Rendered X position (for tabs)
    int rowOffset, colOffset;  // Scroll offsets
    int screenRows, screenCols; // Editor window dimensions
    int numRows;               // Number of rows
    erow *row;                 // Pointer to rows of text
    char *fileName;            // File being edited
    char statusMsg[80];       // Status message buffer
    time_t statusMsgTime;       // Timestamp for status message
    struct termios original_termios; // Original terminal state
};

### typedef struct erow

Represents a single line in the text buffer:

typedef struct erow {
    int size;     // Raw character count
    int rSize;    // Rendered character count (tabs expanded)
    char *chars;  // Raw character string
    char *render; // Rendered string (prepared for display)
} erow;

### struct appendbuff

Dynamic string buffer used for rendering:

struct appendbuff {
    char *b;  // Buffer
    int len;  // Length
};

-----------------------------------

## Editor Initialization

The editor is set up with default state values in initEditor():

void initEditor() {
    E.curX = E.curY = 0;
    E.rowOffset = E.colOffset = 0;
    E.numRows = 0;
    E.row = NULL;
    E.fileName = NULL;
    // Determine terminal window size...
}

It calls getWindowSize() to fetch the dimensions (in rows and columns) of the terminal.

-----------------------------------

## Terminal Handling

The editor manipulates the terminal mode to handle inputs rawly and render outputs precisely.

### Raw Mode

- enableRawMode(): Activates raw input mode by turning off flags like ECHO, ICANON, etc.
- disableRawMode(): Restores the original terminal state using the saved original_termios

### Flags disabled in raw mode

Flag: ECHO
Explanation: Prevents typed input from echoing

Flag: ICANON
Explanation: Disables line-based buffering (input available immediately)

Flag: ISIG
Explanation: Disables Ctrl-C, Ctrl-Z signals

Flag: IXON
Explanation: Disables Ctrl-S/Q for software flow control

Flag: ICRNL
Explanation: Prevents translating CR to NL

Flag: OPOST
Explanation: Disables output processing

-----------------------------------

## Input Processing

### editorReadKey()

Reads a single key from stdin using read(), including multibyte escape sequences for arrow keys and others.

### editorProcessKeypress()

Reflects keyboard events by updating the editor state:

Key: Ctrl-Q
Action: Quit editor

Key: Ctrl-S
Action: Save file

Key: Arrow Keys
Action: Move cursor

Key: Page Up / Down
Action: Scroll by screen

Key: Home / End
Action: Move cursor (begin/end of line)

-----------------------------------

## Rendering / Output

### Refresh Cycle

editorRefreshScreen() updates the visible portion of the editor:

1.  editorScroll() adjusts offsets
2.  Initializes append buffer
3.  Draws content (via editorDrawRows())
4.  Draws status bar (editorDrawStatusBar())
5.  Draws message bar (editorDrawMessageBar())
6.  Moves cursor to proper location
7.  Outputs everything in one write() call

This process ensures smooth non-flickering display.

-----------------------------------

## File I/O

### Opening Files

editorOpen() reads a file line by line and loads it into the internal row storage using editorAppendRow().

### Saving Files

editorSave() converts all rows to a single string using editorRowsToString() and writes it to disk.

-----------------------------------

## Row Operations

### Core Functions

Function: editorAppendRow()
Purpose: Adds a new row to the editor buffer

Function: editorUpdateRow()
Purpose: Prepares a row for rendering (expanding tabs)

Function: editorRowInsertChar()
Purpose: Inserts a character into a row

Function: editorRowCurXToRenderX()
Purpose: Converts raw index to rendered index

-----------------------------------

## Append Buffer

Used to collect output commands and content efficiently:

- abAppend(): Appends data to the buffer
- abFree(): Frees memory after use

-----------------------------------

## Key Bindings

Key: Ctrl-Q
Description: Exit editor

Key: Ctrl-S
Description: Save file

Key: Arrow Keys
Description: Move cursor

Key: Page Up / Down
Description: Vertical scrolling

Key: Home / End
Description: Jump to start/end of line

Key: Backspace/Delete
Description: (TODO) Delete characters

-----------------------------------

## This project is still incomplete

