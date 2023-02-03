#ifndef LCPEX_STRING_EXPANSION_H
#define LCPEX_STRING_EXPANSION_H

#include <wordexp.h>
#include <vector>
#include <cstring>
#include <iostream>


char ** expand_env(const std::string& var, int flags = 0);


#endif //LCPEX_STRING_EXPANSION_H
