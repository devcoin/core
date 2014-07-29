// From the many, one
// From one, the source
//
#ifndef DEVCOIN_UTIL_H
#define DEVCOIN_UTIL_H

#include <map>
#include <string>
#include <vector>

#include "util.h"


static const int step = 4000;
const std::string receiverCSV = std::string("receiver.csv");
const std::string receiverCSVTestNet = std::string("receiverTestNet.csv");
static const int64_t initialSubsidy = 50000 * COIN;
static const int64_t share = initialSubsidy * 9 / 10;
static const int64_t fallbackReduction = (initialSubsidy + share) / 2;
static std::map<std::string, std::string> globalCacheMap;
static const double globalMinimumIdenticalProportion = 0.500001;
static const int globalStepDefault = 4000;
static const double globalWriteNextThreshold = 0.75;
static const double globalLessThanOne = 0.95;
static const double globalLessThanOneMinusThreshold = globalLessThanOne * (1.0 - globalWriteNextThreshold);

std::string getCachedText(const std::string& fileName);
std::vector<std::string> getCommaDividedWords(const std::string& text);
std::string getCommonOutputByText(const std::string& fileName, const std::string& suffix=std::string(""));
std::vector<std::string> getDirectoryNames(const std::string& directoryName);
std::string getDirectoryPath(const std::string& fileName);
double getDouble(const std::string& doubleString);
bool getExists(const std::string& fileName);
double getFileRandomNumber(const std::string& dataDirectory, const std::string& fileName);
std::string getFileText(const std::string& fileName);
std::string getHttpsText(const std::string& address);
int getInt(const std::string& integerString);
std::string getInternetText(const std::string& address);
std::string getJoinedPath(const std::string& directoryPath, const std::string& fileName);
std::string getLocationText(const std::string& address);
std::vector<std::string> getLocationTexts(std::vector<std::string> addresses);
std::string getLower(const std::string& text);
std::vector<std::string> getPeerNames(const std::string& text);
std::string getReplaced(const std::string& text, const std::string& searchString=std::string(" "), const std::string& replaceString=std::string());
bool getStartsWith(const std::string& firstString, const std::string& secondString);
std::string getStepFileName(const std::string& fileName, int height, int step);
std::string getStepOutput(const std::string& directoryPathInput, const std::string& fileName, int height, int step);
std::string getStepText(const std::string& dataDirectory, const std::string& fileName, int height, int step);
std::string getStepTextRecursively(const std::string& directoryPath, const std::string& fileName, int height, const std::string& previousTextInput, int step, int valueDown);
std::string getStringByBoolean(bool boolean);
std::string getStringByDouble(double doublePrecision);
std::string getStringByInt(int integer);
std::string getSuffixedFileName(const std::string& fileName, const std::string& suffix=std::string());
std::vector<std::string> getSuffixedFileNames(std::vector<std::string> fileNames, const std::string& suffix=std::string());
std::vector<std::string> getTextLines(const std::string& text);
std::string getTextWithoutWhitespaceByLines(std::vector<std::string> lines);
std::vector<std::string> getTokens(const std::string& text=std::string(), const std::string& delimiters=std::string(" "));
void makeDirectory(const std::string& directoryPath);
void writeFileText(const std::string& fileName, const std::string& fileText);
void writeFileTextByDirectory(const std::string& directoryPath, const std::string& fileName, const std::string& fileText);
void writeNextIfValueHigher(const std::string& directoryPath, const std::string& fileName, int height, int step, const std::string& stepText);

// Get the share address for this round
//
std::vector<std::string> getCoinAddressStrings(
    const std::string& dataDirectory,
    const std::string& fileName,
    int height,
    int step
);

// Determine if the transactions add up to a share per address for each address.
//
bool getIsSufficientAmount(
    std::vector<std::string> addressStrings,
    std::vector<int64_t> amounts,
    const std::string& dataDirectory,
    const std::string& fileName,
    int height,
    int64_t share,
    int step
);

#endif
