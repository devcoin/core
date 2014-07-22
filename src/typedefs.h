#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <map>
#include <string>

#include "uint256.h"

class CWalletTx;

typedef std::map<std::string, std::string> mapValue_t;
typedef std::map<uint256, CWalletTx> transaction_map_t;

#endif
