#ifndef UNSORTED_SHARED_CODE_H
#define UNSORTED_SHARED_CODE_H

#include <map>

#include "typedefs.h"
#include "uint256.h"

void ReadOrderPos(int64& nOrderPos, mapValue_t& mapValue);

void WriteOrderPos(const int64& nOrderPos, mapValue_t& mapValue);

#endif
