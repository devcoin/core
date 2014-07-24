// From the many, one
// From one, the source.
//
#include <list>
#include <utility> // Gives us std::pair

#include "script.h" // Gives us th typedef CTxDestination
#include "wallet.h"
#include "wallet_db.h"
#include "wallet_transaction.h"

using namespace std;

CWalletTx::CWalletTx()
{
    Init(NULL);
}

CWalletTx::CWalletTx(const CWallet* pwalletIn)
{
    Init(pwalletIn);
}

CWalletTx::CWalletTx(const CWallet* pwalletIn, const CMerkleTx& txIn) : CMerkleTx(txIn)
{
    Init(pwalletIn);
}

CWalletTx::CWalletTx(const CWallet* pwalletIn, const CTransaction& txIn) : CMerkleTx(txIn)
{
    Init(pwalletIn);
}

void CWalletTx::Init(const CWallet* pwalletIn)
{
    pwallet = pwalletIn;
    vtxPrev.clear();
    mapValue.clear();
    vOrderForm.clear();
    fTimeReceivedIsTxTime = false;
    nTimeReceived = 0;
    nTimeSmart = 0;
    fFromMe = false;
    strFromAccount.clear();
    vfSpent.clear();
    fDebitCached = false;
    fCreditCached = false;
    fImmatureCreditCached = false;
    fAvailableCreditCached = false;
    fChangeCached = false;
    nDebitCached = 0;
    nCreditCached = 0;
    nImmatureCreditCached = 0;
    nAvailableCreditCached = 0;
    nChangeCached = 0;
    nOrderPos = -1;
}


// marks certain txout's as spent
// returns true if any update took place
bool CWalletTx::UpdateSpent(const std::vector<char>& vfNewSpent)
{
    bool fReturn = false;
    for (unsigned int i = 0; i < vfNewSpent.size(); i++)
    {
        if (i == vfSpent.size())
            break;

        if (vfNewSpent[i] && !vfSpent[i])
        {
            vfSpent[i] = true;
            fReturn = true;
            fAvailableCreditCached = false;
        }
    }
    return fReturn;
}

// make sure balances are recalculated
void CWalletTx::MarkDirty()
{
    fCreditCached = false;
    fAvailableCreditCached = false;
    fDebitCached = false;
    fChangeCached = false;
}

void CWalletTx::BindWallet(CWallet *pwalletIn)
{
    pwallet = pwalletIn;
    MarkDirty();
}

void CWalletTx::MarkSpent(unsigned int nOut)
{
    if (nOut >= vout.size())
        throw std::runtime_error("CWalletTx::MarkSpent() : nOut out of range");
    vfSpent.resize(vout.size());
    if (!vfSpent[nOut])
    {
        vfSpent[nOut] = true;
        fAvailableCreditCached = false;
    }
}

bool CWalletTx::IsSpent(unsigned int nOut) const
{
    if (nOut >= vout.size())
        throw std::runtime_error("CWalletTx::IsSpent() : nOut out of range");
    if (nOut >= vfSpent.size())
        return false;
    return (!!vfSpent[nOut]);
}

int64 CWalletTx::GetDebit() const
{
    if (vin.empty())
        return 0;
    if (fDebitCached)
        return nDebitCached;
    nDebitCached = pwallet->GetDebit(*this);
    fDebitCached = true;
    return nDebitCached;
}

int64 CWalletTx::GetCredit(bool fUseCache) const
{
    // Must wait until coinbase is safely deep enough in the chain before valuing it
    if (IsCoinBase() && GetBlocksToMaturity() > 0)
        return 0;

    // GetBalance can assume transactions in mapWallet won't change
    if (fUseCache && fCreditCached)
        return nCreditCached;
    nCreditCached = pwallet->GetCredit(*this);
    fCreditCached = true;
    return nCreditCached;
}


int64 CWalletTx::GetImmatureCredit(bool fUseCache) const
{
    if (IsCoinBase() && GetBlocksToMaturity() > 0 && IsInMainChain())
    {
        if (fUseCache && fImmatureCreditCached)
            return nImmatureCreditCached;
        nImmatureCreditCached = pwallet->GetCredit(*this);
        fImmatureCreditCached = true;
        return nImmatureCreditCached;
    }

    return 0;
}


int64 CWalletTx::GetAvailableCredit(bool fUseCache) const
{
    // Must wait until coinbase is safely deep enough in the chain before valuing it
    if (IsCoinBase() && GetBlocksToMaturity() > 0)
        return 0;

    if (fUseCache && fAvailableCreditCached)
        return nAvailableCreditCached;

    int64 nCredit = 0;
    for (unsigned int i = 0; i < vout.size(); i++)
    {
        if (!IsSpent(i))
        {
            const CTxOut &txout = vout[i];
            nCredit += pwallet->GetCredit(txout);
            if (!MoneyRange(nCredit))
                throw std::runtime_error("CWalletTx::GetAvailableCredit() : value out of range");
        }
    }

    nAvailableCreditCached = nCredit;
    fAvailableCreditCached = true;
    return nCredit;
}


int64 CWalletTx::GetChange() const
{
    if (fChangeCached)
        return nChangeCached;
    nChangeCached = pwallet->GetChange(*this);
    fChangeCached = true;
    return nChangeCached;
}


int64 CWalletTx::GetTxTime() const
{
    int64 n = nTimeSmart;
    return n ? n : nTimeReceived;
}


int CWalletTx::GetRequestCount() const
{
    // Returns -1 if it wasn't being tracked
    int nRequests = -1;
    {
        LOCK(pwallet->cs_wallet);
        if (IsCoinBase())
        {
            // Generated block
            if (hashBlock != 0)
            {
                map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(hashBlock);
                if (mi != pwallet->mapRequestCount.end())
                    nRequests = (*mi).second;
            }
        }
        else
        {
            // Did anyone request this transaction?
            map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(GetHash());
            if (mi != pwallet->mapRequestCount.end())
            {
                nRequests = (*mi).second;

                // How about the block it's in?
                if (nRequests == 0 && hashBlock != 0)
                {
                    map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(hashBlock);
                    if (mi != pwallet->mapRequestCount.end())
                        nRequests = (*mi).second;
                    else
                        nRequests = 1; // If it's in someone else's block it must have got out
                }
            }
        }
    }
    return nRequests;
}

void CWalletTx::GetAmounts(list<pair<CTxDestination, int64> >& listReceived,
                           list<pair<CTxDestination, int64> >& listSent, int64& nFee, string& strSentAccount) const
{
    nFee = 0;
    listReceived.clear();
    listSent.clear();
    strSentAccount = strFromAccount;

    // Compute fee:
    int64 nDebit = GetDebit();
    if (nDebit > 0) // debit>0 means we signed/sent this transaction
    {
        int64 nValueOut = GetValueOut();
        nFee = nDebit - nValueOut;
    }

    // Sent/received.
    BOOST_FOREACH(const CTxOut& txout, vout)
    {
        CTxDestination address;
        vector<unsigned char> vchPubKey;
        if (!ExtractDestination(txout.scriptPubKey, address))
        {
            printf("CWalletTx::GetAmounts: Unknown transaction type found, txid %s\n",
                   this->GetHash().ToString().c_str());
        }

        // Don't report 'change' txouts
        if (nDebit > 0 && pwallet->IsChange(txout))
            continue;

        if (nDebit > 0)
            listSent.push_back(make_pair(address, txout.nValue));

        if (pwallet->IsMine(txout))
            listReceived.push_back(make_pair(address, txout.nValue));
    }

}

void CWalletTx::GetAccountAmounts(const string& strAccount, int64& nReceived,
                                  int64& nSent, int64& nFee) const
{
    nReceived = nSent = nFee = 0;

    int64 allFee;
    string strSentAccount;
    list<pair<CTxDestination, int64> > listReceived;
    list<pair<CTxDestination, int64> > listSent;
    GetAmounts(listReceived, listSent, allFee, strSentAccount);

    if (strAccount == strSentAccount)
    {
        BOOST_FOREACH(const PAIRTYPE(CTxDestination,int64)& s, listSent)
            nSent += s.second;
        nFee = allFee;
    }
    {
        LOCK(pwallet->cs_wallet);
        BOOST_FOREACH(const PAIRTYPE(CTxDestination,int64)& r, listReceived)
        {
            if (pwallet->mapAddressBook.count(r.first))
            {
                map<CTxDestination, string>::const_iterator mi = pwallet->mapAddressBook.find(r.first);
                if (mi != pwallet->mapAddressBook.end() && (*mi).second == strAccount)
                    nReceived += r.second;
            }
            else if (strAccount.empty())
            {
                nReceived += r.second;
            }
        }
    }
}

void CWalletTx::AddSupportingTransactions()
{
    vtxPrev.clear();

    const int COPY_DEPTH = 3;
    if (SetMerkleBranch() < COPY_DEPTH)
    {
        vector<uint256> vWorkQueue;
        BOOST_FOREACH(const CTxIn& txin, vin)
            vWorkQueue.push_back(txin.prevout.hash);

        {
            LOCK(pwallet->cs_wallet);
            map<uint256, const CMerkleTx*> mapWalletPrev;
            set<uint256> setAlreadyDone;
            for (unsigned int i = 0; i < vWorkQueue.size(); i++)
            {
                uint256 hash = vWorkQueue[i];
                if (setAlreadyDone.count(hash))
                    continue;
                setAlreadyDone.insert(hash);

                CMerkleTx tx;
                map<uint256, CWalletTx>::const_iterator mi = pwallet->mapWallet.find(hash);
                if (mi != pwallet->mapWallet.end())
                {
                    tx = (*mi).second;
                    BOOST_FOREACH(const CMerkleTx& txWalletPrev, (*mi).second.vtxPrev)
                        mapWalletPrev[txWalletPrev.GetHash()] = &txWalletPrev;
                }
                else if (mapWalletPrev.count(hash))
                {
                    tx = *mapWalletPrev[hash];
                }

                int nDepth = tx.SetMerkleBranch();
                vtxPrev.push_back(tx);

                if (nDepth < COPY_DEPTH)
                {
                    BOOST_FOREACH(const CTxIn& txin, tx.vin)
                        vWorkQueue.push_back(txin.prevout.hash);
                }
            }
        }
    }

    reverse(vtxPrev.begin(), vtxPrev.end());
}

bool CWalletTx::WriteToDisk()
{
    return CWalletDB(pwallet->strWalletFile).WriteTx(GetHash(), *this);
}

bool CWalletTx::IsFromMe() const
{
    return (GetDebit() > 0);
}

bool CWalletTx::IsConfirmed() const
{
    // Quick answer in most cases
    if (!IsFinal())
        return false;
    if (GetDepthInMainChain() >= 1)
        return true;
    if (!IsFromMe()) // using wtx's cached debit
        return false;

    // If no confirmations but it's from us, we can still
    // consider it confirmed if all dependencies are confirmed
    std::map<uint256, const CMerkleTx*> mapPrev;
    std::vector<const CMerkleTx*> vWorkQueue;
    vWorkQueue.reserve(vtxPrev.size()+1);
    vWorkQueue.push_back(this);
    for (unsigned int i = 0; i < vWorkQueue.size(); i++)
    {
        const CMerkleTx* ptx = vWorkQueue[i];

        if (!ptx->IsFinal())
            return false;
        if (ptx->GetDepthInMainChain() >= 1)
            continue;
        if (!pwallet->IsFromMe(*ptx))
            return false;

        if (mapPrev.empty())
        {
            BOOST_FOREACH(const CMerkleTx& tx, vtxPrev)
                mapPrev[tx.GetHash()] = &tx;
        }

        BOOST_FOREACH(const CTxIn& txin, ptx->vin)
        {
            if (!mapPrev.count(txin.prevout.hash))
                return false;
            vWorkQueue.push_back(mapPrev[txin.prevout.hash]);
        }
    }
    return true;
}
