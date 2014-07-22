// From the many, one
// From one, the source.
//
#ifndef WALLET_TRANSACTION_H
#define WALLET_TRANSACTION_H

#include <map>
#include <vector>

#include "serialize.h" // Gives us IMPLEMENT_SERIALIZE, which is dubious
#include "typedefs.h"
#include "uint256.h"
#include "unsorted_shared_code.h"

class CWallet;

/** A transaction with a bunch of additional info that only the owner cares about.
 * It includes any unrecorded transactions needed to link it back to the block chain.
 */
class CWalletTx : public CMerkleTx
{
private:
    const CWallet* pwallet;

public:
    std::vector<CMerkleTx> vtxPrev;
    mapValue_t mapValue;
    std::vector<std::pair<std::string, std::string> > vOrderForm;
    unsigned int fTimeReceivedIsTxTime;
    unsigned int nTimeReceived;  // time received by this node
    unsigned int nTimeSmart;
    char fFromMe;
    std::string strFromAccount;
    std::vector<char> vfSpent; // which outputs are already spent
    int64 nOrderPos;  // position in ordered transaction list

    // memory only
    mutable bool fDebitCached;
    mutable bool fCreditCached;
    mutable bool fImmatureCreditCached;
    mutable bool fAvailableCreditCached;
    mutable bool fChangeCached;
    mutable int64 nDebitCached;
    mutable int64 nCreditCached;
    mutable int64 nImmatureCreditCached;
    mutable int64 nAvailableCreditCached;
    mutable int64 nChangeCached;

    CWalletTx();
    CWalletTx(const CWallet* pwalletIn);
    CWalletTx(const CWallet* pwalletIn, const CMerkleTx& txIn);
    CWalletTx(const CWallet* pwalletIn, const CTransaction& txIn);

    void Init(const CWallet* pwalletIn);

    IMPLEMENT_SERIALIZE
    (
        CWalletTx* pthis = const_cast<CWalletTx*>(this);
        if (fRead)
            pthis->Init(NULL);
        char fSpent = false;

        if (!fRead)
        {
            pthis->mapValue["fromaccount"] = pthis->strFromAccount;

            std::string str;
            BOOST_FOREACH(char f, vfSpent)
            {
                str += (f ? '1' : '0');
                if (f)
                    fSpent = true;
            }
            pthis->mapValue["spent"] = str;

            WriteOrderPos(pthis->nOrderPos, pthis->mapValue);

            if (nTimeSmart)
                pthis->mapValue["timesmart"] = strprintf("%u", nTimeSmart);
        }

        nSerSize += SerReadWrite(s, *(CMerkleTx*)this, nType, nVersion,ser_action);
        READWRITE(vtxPrev);
        READWRITE(mapValue);
        READWRITE(vOrderForm);
        READWRITE(fTimeReceivedIsTxTime);
        READWRITE(nTimeReceived);
        READWRITE(fFromMe);
        READWRITE(fSpent);

        if (fRead)
        {
            pthis->strFromAccount = pthis->mapValue["fromaccount"];

            if (mapValue.count("spent"))
                BOOST_FOREACH(char c, pthis->mapValue["spent"])
                    pthis->vfSpent.push_back(c != '0');
            else
                pthis->vfSpent.assign(vout.size(), fSpent);

            ReadOrderPos(pthis->nOrderPos, pthis->mapValue);

            pthis->nTimeSmart = mapValue.count("timesmart") ? (unsigned int)atoi64(pthis->mapValue["timesmart"]) : 0;
        }

        pthis->mapValue.erase("fromaccount");
        pthis->mapValue.erase("version");
        pthis->mapValue.erase("spent");
        pthis->mapValue.erase("n");
        pthis->mapValue.erase("timesmart");
    )

    // marks certain txout's as spent
    // returns true if any update took place
    bool UpdateSpent(const std::vector<char>& vfNewSpent);

    // make sure balances are recalculated
    void MarkDirty();

    void BindWallet(CWallet *pwalletIn);

    void MarkSpent(unsigned int nOut);

    bool IsSpent(unsigned int nOut) const;

    int64 GetDebit() const;

    int64 GetCredit(bool fUseCache=true) const;

    int64 GetImmatureCredit(bool fUseCache=true) const;

    int64 GetAvailableCredit(bool fUseCache=true) const;

    int64 GetChange() const;

    void GetAmounts(
        std::list<std::pair<CTxDestination, int64> >& listReceived,
        std::list<std::pair<CTxDestination, int64> >& listSent,
        int64& nFee,
        std::string& strSentAccount
    )
    const;

    void GetAccountAmounts(
        const std::string& strAccount,
        int64& nReceived,
        int64& nSent,
        int64& nFee
    )
    const;

    bool IsFromMe() const;

    bool IsConfirmed() const;

    bool WriteToDisk();

    int64 GetTxTime() const;
    int GetRequestCount() const;

    void AddSupportingTransactions();
    bool AcceptWalletTransaction(bool fCheckInputs=true);
    void RelayWalletTransaction();
};

#endif