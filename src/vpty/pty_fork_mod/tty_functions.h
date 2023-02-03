//
// Created by phanes on 2/3/23.
//

#ifndef LCPEX_TTY_FUNCTIONS_H
#define LCPEX_TTY_FUNCTIONS_H


#include <termios.h>

int ttySetCbreak(int fd, struct termios *prevTermios);

int ttySetRaw(int fd, struct termios *prevTermios);

#endif //LCPEX_TTY_FUNCTIONS_H
