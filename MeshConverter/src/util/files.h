#ifndef __UTIL_FILES_H_INCLUDED__
#define __UTIL_FILES_H_INCLUDED__

#include <stdio.h>
#include <string>

void ReadString(FILE *fp, std::string &buffer, int fixedLength = 0);

#endif
