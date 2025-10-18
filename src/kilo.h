#ifndef KILO_H
#define KILO_H


//Headers
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

//data
struct termios original_termios;


//Terminals
void enableRawMode();
void disableRawMode();
void die(const char* s);


#endif 