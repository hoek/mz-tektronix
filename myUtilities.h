#ifndef _MY_UTILITIES_H
#define _MY_UTILITIES_H
// +++ include header files +++
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <cfloat>
#include <climits>
#include <limits>
#include <stdexcept>
#include <string>
#include <ctime>
#include <cctype>
#include <algorithm>
#include <cmath>

#define TOKEN_SIZE 10


std::vector<string> LineParser(string cUserLine, char cUserDelimiter=' ', Bool_t bVerboseMode=kFALSE);


#endif