#ifndef LCPEX_TTY_FUNCTIONS_H
#define LCPEX_TTY_FUNCTIONS_H


#include <termios.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>

int ttySetCbreak(int fd, struct termios * prevTermios);
int ttySetRaw(int fd, struct termios * prevTermios);
void ttyResetExit( struct termios * ttyOrig );



#endif //LCPEX_TTY_FUNCTIONS_H
