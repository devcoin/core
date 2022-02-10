// Copyright (c) 2009-2022 The Bitcoin Core and Devcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DEVCOIN_RECEIVER_H
#define DEVCOIN_RECEIVER_H

#include <boost/asio.hpp>
#include <devcoin.h>

using namespace boost;
using namespace std;

static map<string, string> globalCacheMap;
static const double globalMinimumIdenticalProportion = 0.500001;
static int globalTimeOut = 10;
static int globalTimeOutDouble = globalTimeOut + globalTimeOut;
static const double globalWriteNextThreshold = 0.75;
static const double globalLessThanOne = 0.95;
static const double globalLessThanOneMinusThreshold = globalLessThanOne * (1.0 - globalWriteNextThreshold);

/**
 * Callback function writes data to a std::ostream.
 */
size_t curlWriteFunction(void* buf, size_t size, size_t nmemb, void* userp);

/**
 * /Get the cached text or read it from a file.
 */
string getCachedText(const string& fileName);

/**
 * Get the coin address strings for a height.
 */
vector<string> getCoinAddressStrings(const string& dataDirectory, const string& fileName, int height, int step=devcoinStep);

/**
 * Get the words divided around the comma.
 */
vector<string> getCommaDividedWords(const string& text);

/**
 * Get the common output according to the peers listed in a text.
 */
string getCommonOutputByText(const string& fileName, const string& suffix=string(""));

/**
 * Get the vector of directory names of the given directory.
 */
vector<string> getDirectoryNames(const string& directoryName);

/**
 * Get the directory name of the given file.
 */
string getDirectoryPath(const string& fileName);

/**
 * Get a double precision float from a string.
 */
double getDouble(const string& doubleString);

/**
 * Determine if the file exists.
 */
bool getExists(const string& fileName);

/**
 * Get the random number from a file random_number in the same directory as the given file.
 */
double getFileRandomNumber(const string& dataDirectory, const string& fileName);

/**
 * Get the entire text of a file.
 */
string getFileText(const string& fileName);

/**
 *  Get the entire text of an https page.
 */
string getHttpsText(const string& address);

/**
 * Get an integer from a string.
 */
int getInt(const string& integerString);

/**
 * Determine if the transactions add up to a Devcoin share per address for each address.
 */
bool getIsSufficientAmount(vector<string> addressStrings, vector<int64_t> amounts, const string& dataDirectory, const string& fileName, int height, int64_t share, int step=devcoinStep);

/**
 * Get the directory path joined with the file name.
 */
string getJoinedPath(const string& directoryPath, const string& fileName);

/**
 * Get the page by the address, be it a file name or hypertext address.
 */
string getLocationText(const string& address);

/**
 * Get the pages by the addresses, be they file names or hypertext addresses.
 */
vector<string> getLocationTexts(vector<string> addresses);

/**
 * Get the lowercase string.
 */
string getLower(const string& text);

/**
 * Get the peer names from the text.
 */
vector<string> getPeerNames(const string& text);

/**
 * Get the string with the search string replaced with the replace string.
 */
string getReplaced(const string& text, const string& searchString=string(" "), const string& replaceString=string());

/**
 * Determine if the first string starts with the second string.
 */
bool getStartsWith(const string& firstString, const string& secondString);

/**
 * Get the step file name by the file name.
 */
string getStepFileName(const string& fileName, int height, int step);

/**
 * Get the step output according to the peers listed in a file.
 */
string getStepOutput(const string& directoryPathInput, const string& fileName, int height, int step);

/**
 * Get the step text by the file name.
 */
string getStepText(const string& dataDirectory, const string& fileName, int height, int step);

/**
 * Get the step text recursively.
 */
string getStepTextRecursively(const string& directoryPath, const string& fileName, int height, const string& previousTextInput, int step, int valueDown);

/**
 * Get the string from the boolean.
 */
string getStringByBoolean(bool boolean);

/**
 * Get the string from the double precision float.
 */
string getStringByDouble(double doublePrecision);

/**
 * Get the string from the integer.
 */
string getStringByInt(int integer);

/**
 * Get the file name with the suffix just before the extension.
 */
string getSuffixedFileName(const string& fileName, const string& suffix=string());

/**
 * Get the file names with the suffixes just before the extension.
 */
vector<string> getSuffixedFileNames(vector<string> fileNames, const string& suffix=string());

/**
 * Get all the lines of text of a text.
 */
vector<string> getTextLines(const string& text);

/**
 * Get the text without whitespace, joined with newlines in between.
 */
string getTextWithoutWhitespaceByLines(vector<string> lines);

/**
 * Get the tokens of the text split by the delimeters.
 */
vector<string> getTokens(const string& text=string(), const string& delimiters=string(" "));

/**
 * Make a directory if it does not already exist.
 */
void makeDirectory(const string& directoryPath);

/**
 * Write a text to a file.
 */
void writeFileText(const string& fileName, const string& fileText);

/**
 * Write a text to a file joined to the directory path.
 */
void writeFileTextByDirectory(const string& directoryPath, const string& fileName, const string& fileText);

/**
 * Write next step file if height is higher than the threshold.
 */
void writeNextIfValueHigher(const string& directoryPath, const string& fileName, int height, int step, const string& stepText);

#endif // DEVCOIN_RECEIVER_H
