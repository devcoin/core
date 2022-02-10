// Copyright (c) 2009-2022 The Bitcoin Core and Devcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <chainparamsbase.h>
#define CURL_STATICLIB
extern "C" {
#include <curl/curl.h>
}
#include <iostream>
#include <logging.h>
#include <receiver.h>
#include <shutdown.h>
#include <util/system.h>

using namespace boost;
using namespace std;

size_t curlWriteFunction(void* buf, size_t size, size_t nmemb, void* userp)
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

string getCachedText(const string& fileName)
{
    if (globalCacheMap.count(fileName) == 0)
        globalCacheMap[fileName] = getFileText(fileName);

    return globalCacheMap[fileName];
}

vector<string> getCoinAddressStrings(const string& dataDirectory, const string& fileName, int height, int step)
{
    vector<string> coinList;
    vector<vector<string> > coinLists;
    string stepOutput = getStepOutput(dataDirectory, fileName, height, step);
    vector<string> textLines = getTextLines(stepOutput);
    bool isCoinSection = false;
    string oldToken = string();

    for (unsigned int lineIndex = 0; lineIndex < textLines.size(); lineIndex++)
    {
        string firstLowerSpaceless = string();
        string line = textLines[lineIndex];
        vector<string> words = getCommaDividedWords(line);

        if (words.size() > 0)
            firstLowerSpaceless = getReplaced(getLower(words[0]));

        if (firstLowerSpaceless == string("coin"))
        {
            vector<string> coinList = getTokens(words[1], ",");
            coinLists.push_back(coinList);
        }

        if (firstLowerSpaceless == string("_endcoins") || firstLowerSpaceless == string("_endaddresses"))
            isCoinSection = false;

        if (isCoinSection)
        {
            vector<string> coinList = getTokens(line, ",");
            coinLists.push_back(coinList);
        }

        if (firstLowerSpaceless == string("_begincoins") || firstLowerSpaceless == string("_beginaddresses"))
            isCoinSection = true;
    }

    if ((unsigned int)coinLists.size() == 0)
    {
        LogPrintf("%s: Warning, no coin lists were found for the file: %s\n", __func__, fileName.c_str());
        return getTokens();
    }

    int remainder = height - step * (height / step);
    int modulo = remainder % (int)coinLists.size();

    vector<string> originalList = coinLists[modulo];
    for (vector<string>::iterator tokenIterator = originalList.begin(); tokenIterator != originalList.end(); tokenIterator++)
    {
        if (*tokenIterator != string("="))
            oldToken = tokenIterator->substr();

        coinList.push_back(oldToken);
    }
    return coinList;
}

vector<string> getCommaDividedWords(const string& text)
{
    vector<string> commaDividedWords;
    size_t commaIndex = text.find(',');

    if (commaIndex == string::npos)
    {
        commaDividedWords.push_back(text);
        return commaDividedWords;
    }

    commaDividedWords.push_back(text.substr(0, commaIndex));
    commaDividedWords.push_back(text.substr(commaIndex + 1));
    return commaDividedWords;
}

string getCommonOutputByText(const string& fileText, const string& suffix)
{
    if (suffix == string("0") || suffix == string("1"))
    {
        string receiverFileName;
        ArgsManager argsman;
        string chain = argsman.GetChainName();
        if (CBaseChainParams::MAIN == chain) {
            receiverFileName = string("receiver_") + suffix + string(".csv");
        }
        else if (CBaseChainParams::TESTNET == chain) {
            receiverFileName = string("receiverTestNet_") + suffix + string(".csv");
        }
        else if (CBaseChainParams::SIGNET == chain) {
            receiverFileName = string("receiverSigNet_") + suffix + string(".csv");
        }
        else if (CBaseChainParams::REGTEST == chain) {
            receiverFileName = string("receiverRegTest_") + suffix + string(".csv");
        }
        if (getExists(receiverFileName))
            return getFileText(receiverFileName);
    }

    vector<string> peerNames = getPeerNames(fileText);
    vector<string> pages = getLocationTexts(getSuffixedFileNames(peerNames, suffix));
    int minimumIdentical = (int)ceil(globalMinimumIdenticalProportion * (double)pages.size());
    map<string, int> pageMap;

    for (vector<string>::iterator pageIterator = pages.begin(); pageIterator != pages.end(); pageIterator++)
    {
        string firstLine = string();
        vector<string> lines = getTextLines(*pageIterator);
        string textWithoutWhitespace = getTextWithoutWhitespaceByLines(lines);

        if (lines.size() > 0)
            firstLine = getLower(lines[0]);

        if (getStartsWith(firstLine, string("format")) && (firstLine.find(string("pluribusunum")) != string::npos))
        {
            if (pageMap.count(textWithoutWhitespace))
                pageMap[textWithoutWhitespace] += 1;
            else
                pageMap[textWithoutWhitespace] = 1;
        }
    }

    LogPrintf("%s: Number of pages in getCommonOutputByText: %d\n", __func__, pages.size());

    for (map<string,int>::iterator pageMapIterator = pageMap.begin(); pageMapIterator != pageMap.end(); pageMapIterator++) {
        if ((*pageMapIterator).second >= minimumIdentical)
            return (*pageMapIterator).first;

        LogPrintf("%s: Number of identical pages in getCommonOutputByText: %d\n", __func__, (*pageMapIterator).second);
    }

    LogPrintf("%s: Insufficient identical pages in getCommonOutputByText.\n", __func__);

    return string();
}

vector<string> getDirectoryNames(const string& directoryName)
{
    vector<string> directoryNames;

    if (!getExists(directoryName))
    {
        LogPrintf("%s: Warning, can not open directory: %s\n", __func__, directoryName.c_str());
        return directoryNames;
    }

    boost::filesystem::directory_iterator endIterator;

    for (boost::filesystem::directory_iterator directoryIterator(directoryName); directoryIterator != endIterator; ++directoryIterator)
    {
        if (!boost::filesystem::is_directory(directoryIterator->status()))
            directoryNames.push_back(directoryIterator->path().string());
    }

    return directoryNames;
}

string getDirectoryPath(const string& fileName)
{
    string directoryPath = (boost::filesystem::path(fileName)).parent_path().string();
    if (directoryPath == string())
        return string(".");
    return directoryPath;
}

double getDouble(const string& doubleString)
{
    double doublePrecision;
    istringstream doubleStream(doubleString);

    doubleStream >> doublePrecision;
    return doublePrecision;
}

bool getExists(const string& fileName)
{
    return boost::filesystem::exists(fileName);
}

double getFileRandomNumber(const string& dataDirectory, const string& fileName)
{
    string directoryPath = dataDirectory.substr();
    if (dataDirectory == string())
        directoryPath = getDirectoryPath(fileName);
    string numberFilePath = getJoinedPath(directoryPath, string("random_number.txt"));
    string numberFileText = getFileText(numberFilePath);

    if (numberFileText == string())
    {
        srand(time(NULL));
        double randomDouble = ((double)(rand() % 10000) + 0.5) / 10000.0;
        numberFileText = getStringByDouble(randomDouble);
        writeFileText(numberFilePath, numberFileText);
    }

    return getDouble(numberFileText);
}

string getFileText(const string& fileName)
{
    ifstream fileStream(fileName.c_str());

    if (!fileStream.is_open())
        return string();

    string fileText;
    fileStream.seekg(0, ios::end);
    fileText.reserve(fileStream.tellg());
    fileStream.seekg(0, ios::beg);
    fileText.assign((istreambuf_iterator<char>(fileStream)), istreambuf_iterator<char>());
    fileStream.close();
    return fileText;
}

string getHttpsText(const string& address)
{

    CURL *curl;
    long http_code;
    string ret = string();

    curl = curl_easy_init();

    if(curl)
    {
        CURLcode code;
        std::ostringstream oss;
//        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
//        curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &oss);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &curlWriteFunction);
        curl_easy_setopt(curl, CURLOPT_URL, address.c_str());
        code = curl_easy_perform(curl);
//                printf("Curl-- Response: %d\n", code);

        if (code == CURLE_OK) {
            code = curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &http_code);
            // printf("Http Response: %l\n", http_code);
            if (http_code == 200) ret = oss.str();
            }
        }
    curl_easy_cleanup(curl);
    return ret;
}

int getInt(const string& integerString)
{
    try
    {
        int integer;
        istringstream intStream(integerString);

        intStream >> integer;
        return integer;
    }
    catch (std::exception& e)
    {
        LogPrintf("%s: Could not get int for: %s\n", __func__, integerString);
        LogPrintf("%s: Exception: %s\n", __func__, e.what());
        return 0;
    }
}

bool getIsSufficientAmount(vector<string> addressStrings, vector<int64_t> amounts, const string& dataDirectory, const string& fileName, int height, int64_t share, int step)
{
    vector<string> coinAddressStrings = getCoinAddressStrings(dataDirectory, fileName, height, step);
    map<string, int64_t> receiverMap;

    if (coinAddressStrings.size() == 0)
    {
        LogPrintf("%s: No coin addresses were found, there may be something wrong with the receiver_x.csv files.\n", __func__);
        return false;
    }

    int64_t sharePerAddress = share / (int64_t)coinAddressStrings.size();

    for (unsigned int i = 0; i < coinAddressStrings.size(); i++)
        receiverMap[coinAddressStrings[i]] = (int64_t)0;

    for (unsigned int i = 0; i < addressStrings.size(); i++)
    {
        string addressString = addressStrings[i];

        if (receiverMap.count(addressString) > 0)
            receiverMap[addressString] += amounts[i];
    }

    for (unsigned int i = 0; i < coinAddressStrings.size(); i++)
    {
        if (receiverMap[coinAddressStrings[i]] < sharePerAddress)
        {
            LogPrintf("%s: In receiver.h, getIsSufficientAmount rejected the addresses or amounts.\n", __func__);
            LogPrintf("For the given:\n");
            LogPrintf("- Height: %d\n", height);
            LogPrintf("- Share: %d\n", share);
            LogPrintf("- Step: %d\n", step);
            LogPrintf("The receiverMap shows that they should get %d coins\n", receiverMap[coinAddressStrings[i]]);
            LogPrintf("The shares per address however are set to %d\n", sharePerAddress);
            LogPrintf("The expected addresses are:\n");
            for (unsigned int i = 0; i < coinAddressStrings.size(); i++)
                LogPrintf("  %s\n", coinAddressStrings[i]);

            LogPrintf("The given addresses are:\n");
            for (unsigned int i = 0; i < addressStrings.size(); i++)
                LogPrintf("  %s\n", addressStrings[i]);

            LogPrintf("The given amounts are:\n");
            for (unsigned int i = 0; i < amounts.size(); i++)
                LogPrintf("  %d\n", amounts[i]);

            return false;
        }
    }

    return true;
}

string getJoinedPath(const string& directoryPath, const string& fileName)
{
    boost::filesystem::path completePath = boost::filesystem::system_complete(boost::filesystem::path(directoryPath));
    return (completePath / (boost::filesystem::path(fileName))).string();
}

string getLocationText(const string& address)
{

  if (getStartsWith(address, string("https://")) || getStartsWith(address, string("http://")))
    return getHttpsText(address);

  return getFileText(address);
}

vector<string> getLocationTexts(vector<string> addresses)
{
    vector<string> locationTexts;

    for(unsigned int addressIndex = 0; addressIndex < addresses.size(); addressIndex++) {
        LogPrintf("%s: Fetching %s ...\n", __func__, addresses[addressIndex]);
        const string locationText = getLocationText(addresses[addressIndex]);
        LogPrintf("%s: got %d bytes\n", __func__, locationText.length());
        locationTexts.push_back(locationText);
    }

    return locationTexts;
}

string getLower(const string& text)
{
    unsigned int textLength = text.length();
    string lower = text.substr();

    for(unsigned int characterIndex = 0; characterIndex < textLength; characterIndex++)
    {
        lower[characterIndex] = tolower(text[characterIndex]);
    }

    return lower;
}

vector<string> getPeerNames(const string& text)
{
    bool isPeerSection = false;
    vector<string> peerNames;
    vector<string> textLines = getTextLines(text);

    for (unsigned int lineIndex = 0; lineIndex < textLines.size(); lineIndex++)
    {
        string firstLowerSpaceless = string();
        string line = textLines[lineIndex];
        vector<string> words = getCommaDividedWords(line);

        if (words.size() > 0)
            firstLowerSpaceless = getReplaced(getLower(words[0]));

        if (firstLowerSpaceless == string("peer"))
            peerNames.push_back(getReplaced(words[1]));

        if (firstLowerSpaceless == string("_endpeers"))
            isPeerSection = false;

        if (isPeerSection)
            peerNames.push_back(getReplaced(words[0]));

        if (firstLowerSpaceless == string("_beginpeers"))
            isPeerSection = true;
    }

    return peerNames;
}

string getReplaced(const string& text, const string& searchString, const string& replaceString)
{
    string::size_type position = 1;
    string replaced = text.substr();

    // position - 1 is to delete a repeated searchString
    while ((position = replaced.find(searchString, position - 1)) != string::npos)
    {
        replaced.replace(position, searchString.size(), replaceString );
        position++;
    }

    return replaced;
}

bool getStartsWith(const string& firstString, const string& secondString)
{
    if (firstString.substr(0, secondString.size()) == secondString)
        return true;

    return false;
}

string getStepFileName(const string& fileName, int height, int step)
{
    return getSuffixedFileName(fileName, getStringByInt(height / step));
}

string getStepOutput(const string& directoryPathInput, const string& fileName, int height, int step)
{

    string directoryPath = string();

    if (directoryPathInput != string())
        directoryPath = getJoinedPath(directoryPathInput, fileName.substr(0, fileName.rfind('.')));

    // To fix wrong receiver file problem and old receiver chain problem.
//    string stepFileName = getStepFileName(fileName, height, step);
//    if (stepFileName == string("receiver_1.csv"))
//    {
//        if (getExists(stepFileName))
//            writeFileTextByDirectory(directoryPath, stepFileName, getFileText(stepFileName));
//    }

//    if (stepFileName == string("receiver_2.csv"))
//    {
//        if (getExists(stepFileName))
//            writeFileTextByDirectory(directoryPath, stepFileName, getFileText(stepFileName));
//    }
    string stepText = getStepText(directoryPath, fileName, height, step);

    if (stepText != string())
    {
        writeNextIfValueHigher(directoryPath, fileName, height, step, stepText);
        return stepText;
    }

    int valueDown = height - step;
    string previousText = string();
    while (valueDown >= 0)
    {
        previousText = getStepText(directoryPath, fileName, valueDown, step);

        if (previousText != string())
            return getStepTextRecursively(directoryPath, fileName, height, previousText, step, valueDown);

        valueDown -= step;
    }

    return string();
}

string getStepText(const string& dataDirectory, const string& fileName, int height, int step)
{

    string stepFileName = getStepFileName(fileName, height, step);
    if (dataDirectory == string())
        return getFileText(stepFileName);

    string directorySubName = getJoinedPath(dataDirectory, stepFileName);
    if (getExists(directorySubName))
    {
            return getCachedText(directorySubName);
    }
//            return getFileText(directorySubName);

    string stepText = getFileText(stepFileName);

    if (stepText == string())
    {
        LogPrintf("%s: Need to download %s to %s\n", __func__, stepFileName, directorySubName);
        if (stepFileName == string("receiver_0.csv")) {
            string peerText = string("_beginpeers\n");
            peerText += string("https://galaxies.mygamesonline.org/receiver.csv\n");
            peerText += string("https://receiver01.devcoin.org/receiver.csv\n");
            peerText += string("https://receiver02.devcoin.org/receiver/receiver.csv\n");
            peerText += string("https://receiver03.devcoin.org/receiver.csv\n");
            peerText += string("https://receiver04.devcoin.org/receiver.csv\n");
            peerText += string("_endpeers\n");
            stepText = getCommonOutputByText(peerText, string("0"));
            if (getStartsWith(stepText, string("Format,pluribusunum")))
                writeFileText(directorySubName, stepText);
        }
        else if (stepFileName == string("receiverTestNet_0.csv")) {
            string peerText = string("_beginpeers\n");
            peerText += string("https://testnet-receiver01.devcoin.org/receiverTestNet.csv\n");
            peerText += string("https://testnet-receiver02.devcoin.org/receiverTestNet.csv\n");
            peerText += string("_endpeers\n");
            stepText = getCommonOutputByText(peerText, string("0"));
            if (getStartsWith(stepText, string("Format,pluribusunum")))
                writeFileText(directorySubName, stepText);
        }
        else if (stepFileName == string("receiverSigNet_0.csv")) {
            string peerText = string("_beginpeers\n");
            peerText += string("https://signet-receiver01.devcoin.org/receiverSigNet.csv\n");
            peerText += string("https://signet-receiver02.devcoin.org/receiverSigNet.csv\n");
            peerText += string("_endpeers\n");
            stepText = getCommonOutputByText(peerText, string("0"));
            if (getStartsWith(stepText, string("Format,pluribusunum")))
                writeFileText(directorySubName, stepText);
        }
        else if (stepFileName == string("receiverRegTest_0.csv")) {
            string peerText = string("_beginpeers\n");
            peerText += string("https://signet-receiver01.devcoin.org/receiverRegTest.csv\n");
            peerText += string("https://signet-receiver02.devcoin.org/receiverRegTest.csv\n");
            peerText += string("_endpeers\n");
            stepText = getCommonOutputByText(peerText, string("0"));
            if (getStartsWith(stepText, string("Format,pluribusunum")))
                writeFileText(directorySubName, stepText);
        }
        else
            return string();
    }

    writeFileText(directorySubName, stepText);
    return stepText;
}

string getStepTextRecursively(const string& directoryPath, const string& fileName, int height, const string& previousTextInput, int step, int valueDown)
{
    string previousText = previousTextInput.substr();
    string stepFileName;

    for(int valueUp = valueDown; valueUp < height; valueUp += step)
    {
        int nextValue = valueUp + step;
        previousText = getCommonOutputByText(previousText, getStringByInt(nextValue / step));
        stepFileName = getStepFileName(fileName, nextValue, step);
        writeFileTextByDirectory(directoryPath, stepFileName, previousText);

        if (ShutdownRequested()) break;
    }

    return previousText;
}

string getStringByBoolean(bool boolean)
{
    if (boolean)
        return string("true");
    return string("false");
}

string getStringByDouble(double doublePrecision)
{
    ostringstream doubleStream;

    doubleStream << doublePrecision;

    return doubleStream.str();
}

string getStringByInt(int integer)
{
    ostringstream integerStream;

    integerStream << integer;

    return integerStream.str();
}

string getSuffixedFileName(const string& fileName, const string& suffix)
{
    if (suffix == string())
        return fileName;

    size_t lastDotIndex = fileName.rfind(".");

    if (lastDotIndex == string::npos)
        return fileName + suffix;
    return fileName.substr(0, lastDotIndex) + string("_") + suffix + fileName.substr(lastDotIndex);
}

vector<string> getSuffixedFileNames(vector<string> fileNames, const string& suffix)
{
    vector<string> suffixedFileNames;

    for (unsigned int fileNameIndex = 0; fileNameIndex < fileNames.size(); fileNameIndex++)
    {
        string fileName = fileNames[fileNameIndex];
        size_t doNotAddSuffixIndex = fileName.find("_do_not_add_suffix_");

        if (doNotAddSuffixIndex == string::npos)
            suffixedFileNames.push_back(getSuffixedFileName(fileName, suffix));
        else
            suffixedFileNames.push_back(fileName.substr(0, doNotAddSuffixIndex));
    }

    return suffixedFileNames;
}

vector<string> getTextLines(const string& text)
{
    return getTokens(getReplaced(getReplaced(text, string("\r"), string("\n")), string("\n\n"), string("\n")), string("\n"));
}

string getTextWithoutWhitespaceByLines(vector<string> lines)
{
    string textWithoutWhitespace = string();

    for (vector<string>::iterator lineIterator = lines.begin(); lineIterator != lines.end(); lineIterator++)
    {
        string line = getReplaced(*lineIterator);

        if (line.size() > 0)
            textWithoutWhitespace += line + string("\n");
    }

    return textWithoutWhitespace;
}

vector<string> getTokens(const string& text, const string& delimiters)
{
    vector<string> tokens;
    string::size_type lastPosition = text.find_first_not_of(delimiters, 0);
    string::size_type position = text.find_first_of(delimiters, lastPosition);

    while (string::npos != position || string::npos != lastPosition)
    {
        tokens.push_back(text.substr(lastPosition, position - lastPosition));
        lastPosition = text.find_first_not_of(delimiters, position);
        position = text.find_first_of(delimiters, lastPosition);
    }

    return tokens;
}

void makeDirectory(const string& directoryPath)
{
    if (getReplaced(directoryPath) == string() || directoryPath == string("."))
        return;

    if (getExists(directoryPath))
        return;

    if (boost::filesystem::create_directories(boost::filesystem::path(directoryPath)))
        LogPrintf("%s: The following directory was made: %s\n", __func__, directoryPath.c_str());
    else
        LogPrintf("%s: Receiver.h can not make the directory %s so give it read/write permission for that directory.\n", __func__, directoryPath.c_str());
}

void writeFileText(const string& fileName, const string& fileText)
{
    if (fileText == string())
    {
        LogPrintf("%s: Warning, writeFileText in receiver.h won't write the file:\n%s\nbecause the text is blank.\n", __func__, fileName);
        return;
    }

    makeDirectory(getDirectoryPath(fileName));
    ofstream fileStream(fileName.c_str());

    if (fileStream.is_open())
    {
      fileStream << fileText;
      fileStream.close();
    }
    else LogPrintf("%s: The file %s can not be written to.\n", __func__, fileName.c_str());
}

void writeFileTextByDirectory(const string& directoryPath, const string& fileName, const string& fileText)
{
    writeFileText(getJoinedPath(directoryPath, fileName), fileText);
}

void writeNextIfValueHigher(const string& directoryPath, const string& fileName, int height, int step, const string& stepText)
{
    int remainder = height - step * (height / step);
    double aboveThreshold = globalLessThanOneMinusThreshold * getFileRandomNumber(directoryPath, fileName);
    int remainderThreshold = (int)(double(step) * (globalWriteNextThreshold + aboveThreshold));
    string writeNextWhenFileName = getJoinedPath(directoryPath, string("write_next_when.txt"));

    if (remainder < remainderThreshold)
        return;

    if (getExists(writeNextWhenFileName))
    {
        if (remainder < getInt(getFileText(writeNextWhenFileName)))
            return;
        else
            remove(writeNextWhenFileName.c_str());
    }

    int nextValue = height + step;
    string nextFileName = getJoinedPath(directoryPath, getStepFileName(fileName, nextValue, step));

    if (!getExists(nextFileName))
    {
        string nextText = getCommonOutputByText(stepText, getStringByInt(nextValue / step));

        if (nextText == string())
        {
            int addition = 10;

            if (remainder > (int)(globalLessThanOne * (double)step))
                addition = 3;

            writeFileText(writeNextWhenFileName, getStringByInt(remainder + addition));
        }
        else
            writeFileText(nextFileName, nextText);
    }
}
