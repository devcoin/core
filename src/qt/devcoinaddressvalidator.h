// Copyright (c) 2011-2020 The Bitcoin Core and Devcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DEVCOIN_QT_DEVCOINADDRESSVALIDATOR_H
#define DEVCOIN_QT_DEVCOINADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class DevcoinAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit DevcoinAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

/** Devcoin address widget validator, checks for a valid devcoin address.
 */
class DevcoinAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit DevcoinAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

#endif // DEVCOIN_QT_DEVCOINADDRESSVALIDATOR_H
