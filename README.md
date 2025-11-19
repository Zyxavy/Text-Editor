# Kilo Text Editor

A lightweight, terminal-based text editor written in C, inspired by [antirez's kilo](https://github.com/antirez/kilo). This implementation follows the guide from [Build Your Own Text Editor](https://viewsourcecode.org/snaptoken/kilo/index.html).
This project serves as an exercise in understanding low-level terminal handling, system calls, and the C programming language.

## Requirements

- C compiler (GCC recommended)
- Unix-like system (Linux, macOS, BSD)
- Terminal with ANSI escape code support

## Compilation

To compile the editor, simply run the make command in the project directory. This will use the provided Makefile to create an executable file named kilo.

```bash
make
```

## Running the Editor

Once compiled, you can run the editor with the following command:

```bash
./kilo
```

## Credits

- This project is an implementation of the editor from the "Build Your Own Text Editor" tutorial.
- The original kilo editor was created by antirez.

## Keybind

## Key Bindings

Key: Ctrl-Q = Exit editor (requires multiple presses if file is modified)

Key: Ctrl-S = Save file

Key: Ctrl-F = Search (find) for text

Key: Arrow Keys = Move cursor

Key: Page Up / Down = Vertical scrolling by a screen's height

Key: Home Key = Jump to start of line

Key: End Key = Jump to end of line

Key: Backspace/Ctrl-H = Delete character before cursor


