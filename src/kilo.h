#ifndef KILO_H
#define KILO_H


//Headers
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

//Structs
struct termios original_termios;


//Function Declarations
void enableRawMode();
void disableRawMode();


#endif 