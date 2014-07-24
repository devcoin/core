// From the many, one
// From one, the source.
//
#ifndef WALLET_DB_ERRORS_H
#define WALLET_DB_ERRORS_H

/** Error statuses for the wallet database */
enum DBErrors
{
    DB_LOAD_OK,
    DB_CORRUPT,
    DB_NONCRITICAL_ERROR,
    DB_TOO_NEW,
    DB_LOAD_FAIL,
    DB_NEED_REWRITE
};

#endif
