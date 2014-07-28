// From the many, one
// From one, the source
//
#ifndef DEVCOIN_UTIL_H
#define DEVCOIN_UTIL_H

#include "util.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <curl/curl.h>

using namespace boost;
using namespace std;

static const int step = 4000;
const std::string receiverCSV = string("receiver.csv");
const std::string receiverCSVTestNet = string("receiverTestNet.csv");
static const int64_t initialSubsidy = 50000 * COIN;
static const int64_t share = initialSubsidy * 9 / 10;
static const int64_t fallbackReduction = (initialSubsidy + share) / 2;
static map<string, string> globalCacheMap;
static const double globalMinimumIdenticalProportion = 0.500001;
static const int globalStepDefault = 4000;
static const double globalWriteNextThreshold = 0.75;
static const double globalLessThanOne = 0.95;
static const double globalLessThanOneMinusThreshold = globalLessThanOne * (1.0 - globalWriteNextThreshold);

string getCachedText(const string& fileName);
vector<string> getCommaDividedWords(const string& text);
string getCommonOutputByText(const string& fileName, const string& suffix=string(""));
vector<string> getDirectoryNames(const string& directoryName);
string getDirectoryPath(const string& fileName);
double getDouble(const string& doubleString);
bool getExists(const string& fileName);
double getFileRandomNumber(const string& dataDirectory, const string& fileName);
string getFileText(const string& fileName);
string getHttpsText(const string& address);
int getInt(const string& integerString);
string getInternetText(const string& address);
string getJoinedPath(const string& directoryPath, const string& fileName);
string getLocationText(const string& address);
vector<string> getLocationTexts(vector<string> addresses);
string getLower(const string& text);
vector<string> getPeerNames(const string& text);
string getReplaced(const string& text, const string& searchString=string(" "), const string& replaceString=string());
bool getStartsWith(const string& firstString, const string& secondString);
string getStepFileName(const string& fileName, int height, int step);
string getStepOutput(const string& directoryPathInput, const string& fileName, int height, int step);
string getStepText(const string& dataDirectory, const string& fileName, int height, int step);
string getStepTextRecursively(const string& directoryPath, const string& fileName, int height, const string& previousTextInput, int step, int valueDown);
string getStringByBoolean(bool boolean);
string getStringByDouble(double doublePrecision);
string getStringByInt(int integer);
string getSuffixedFileName(const string& fileName, const string& suffix=string());
vector<string> getSuffixedFileNames(vector<string> fileNames, const string& suffix=string());
vector<string> getTextLines(const string& text);
string getTextWithoutWhitespaceByLines(vector<string> lines);
vector<string> getTokens(const string& text=string(), const string& delimiters=string(" "));
void makeDirectory(const string& directoryPath);
void writeFileText(const string& fileName, const string& fileText);
void writeFileTextByDirectory(const string& directoryPath, const string& fileName, const string& fileText);
void writeNextIfValueHigher(const string& directoryPath, const string& fileName, int height, int step, const string& stepText);


// Callback function writes data to a std::ostream.
static size_t curlWriteFunction(void* buf, size_t size, size_t nmemb, void* userp)
{
    if(userp)
    {
        std::ostream& os = *static_cast<std::ostream*>(userp);
        std::streamsize len = size * nmemb;

        if(os.write(static_cast<char*>(buf), len))
            return len;
    }

    return 0;
}

// Get the share address for this round
//
vector<string> getCoinAddressStrings(
    const string& dataDirectory,
    const string& fileName,
    int height,
    int step=globalStepDefault
);

// Determine if the transactions add up to a share per address for each address.
//
bool getIsSufficientAmount(
    vector<string> addressStrings,
    vector<int64_t> amounts,
    const string& dataDirectory,
    const string& fileName,
    int height,
    int64_t share,
    int step
);

#endif
