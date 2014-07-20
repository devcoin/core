// From the many, one
// From one, the source.
//
#ifndef DEVCOIN_EXCEPTIONS_H
#define DEVCOIN_EXCEPTIONS_H

// #include <stdlib.h>


class WalletCouldNotFindCoinsForTransaction: public std::exception
{
    virtual const char* what() const throw()
    {
      return "The wallet was unable to find coins to fulfill the transation.";
    }
};


#endif
