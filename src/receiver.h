#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#define CURL_STATICLIB
extern "C" {
#include <curl/curl.h>
}
using namespace boost;
using namespace std;


static map<string, string> globalCacheMap;
static int globalDownloadIndex = 10;
static const double globalMinimumIdenticalProportion = 0.500001;
static const int globalStepDefault = 4000;
static int globalTimeOut = 10;
static int globalTimeOutDouble = globalTimeOut + globalTimeOut;
static const double globalWriteNextThreshold = 0.75;
static const double globalLessThanOne = 0.95;
static const double globalLessThanOneMinusThreshold = globalLessThanOne * (1.0 - globalWriteNextThreshold);


static size_t curlWriteFunction(void* buf, size_t size, size_t nmemb, void* userp);
string getCachedText(const string& fileName);
vector<string> getCoinAddressStrings(const string& dataDirectory, const string& fileName, int height, int step=globalStepDefault);
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
bool getIsSufficientAmount(vector<string> addressStrings, vector<int64> amounts, const string& dataDirectory, const string& fileName, int height, int64 share, int step=globalStepDefault);
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

// Get the cached text or read it from a file.
string getCachedText(const string& fileName)
{
	if (globalCacheMap.count(fileName) == 0)
		globalCacheMap[fileName] = getFileText(fileName);

	return globalCacheMap[fileName];
}

// Get the coin address strings for a height.
vector<string> getCoinAddressStrings(const string& dataDirectory, const string& fileName, int height, int step)
{
	vector<string> coinList;
	vector<vector<string> > coinLists;
	vector<string> textLines = getTextLines(getStepOutput(dataDirectory, fileName, height, step));
	bool isCoinSection = false;
	string oldToken = string();

	for (int lineIndex = 0; lineIndex < textLines.size(); lineIndex++)
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

	if ((int)coinLists.size() == 0)
	{
		printf("Warning, no coin lists were found for the file: %s\n", fileName.c_str());
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

// Get the words divided around the comma.
vector<string> getCommaDividedWords(const string& text)
{
	vector<string> commaDividedWords;
	int commaIndex = text.find(',');

	if (commaIndex == string::npos)
	{
		commaDividedWords.push_back(text);
		return commaDividedWords;
	}

	commaDividedWords.push_back(text.substr(0, commaIndex));
	commaDividedWords.push_back(text.substr(commaIndex + 1));
	return commaDividedWords;
}

// Get the common output according to the peers listed in a text.
string getCommonOutputByText(const string& fileText, const string& suffix)
{
	if (suffix == string("0") || suffix == string("1"))
	{
		string receiverFileName; 
		if(fTestNet == true)
		{
			receiverFileName = string("receiverTestNet_") + suffix + string(".csv");
		}
		else
		{
			receiverFileName = string("receiver_") + suffix + string(".csv");
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

	cout << endl << "Number of pages in getCommonOutputByText: " << pages.size() << endl;

	for (map<string,int>::iterator pageMapIterator = pageMap.begin(); pageMapIterator != pageMap.end(); pageMapIterator++)
		if ((*pageMapIterator).second >= minimumIdentical)
		{
			cout << "Number of identical pages in getCommonOutputByText: " << (*pageMapIterator).second << endl << endl;
			return (*pageMapIterator).first;
		}

	cout << "Insufficient identical pages in getCommonOutputByText." << endl;
	return string();
}

// Get the vector of directory names of the given directory.
vector<string> getDirectoryNames(const string& directoryName)
{
	vector<string> directoryNames;

	if (!getExists(directoryName))
	{
		printf("Warning, can not open directory: %s\n", directoryName.c_str());
		return directoryNames;
	}

	filesystem::directory_iterator endIterator;

	for (filesystem::directory_iterator directoryIterator(directoryName); directoryIterator != endIterator; ++directoryIterator)
	{
		if (!filesystem::is_directory(directoryIterator->status()))
			directoryNames.push_back(directoryIterator->path().string());
	}

	return directoryNames;
}

// Get the directory name of the given file.
string getDirectoryPath(const string& fileName)
{
	string directoryPath = (filesystem::path(fileName)).parent_path().string();
	if (directoryPath == string())
		return string(".");
	return directoryPath;
}

// Get a double precision float from a string.
double getDouble(const string& doubleString)
{
	double doublePrecision;
	istringstream doubleStream(doubleString);

	doubleStream >> doublePrecision;
	return doublePrecision;
}

// Determine if the file exists.
bool getExists(const string& fileName)
{
	return filesystem::exists(fileName);
}

// Get the random number from a file random_number in the same directory as the given file.
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

// Get the entire text of a file.
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

// Get the entire text of an https page.
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
//		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
//		curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
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

// Get an integer from a string.
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
		cout << "Could not get int for: " << integerString << endl;
		cout << "Exception: " << e.what() << endl;
		return 0;
	}
}

// Determine if the transactions add up to a share per address for each address.
bool getIsSufficientAmount(vector<string> addressStrings, vector<int64> amounts, const string& dataDirectory, const string& fileName, int height, int64 share, int step)
{
	vector<string> coinAddressStrings = getCoinAddressStrings(dataDirectory, fileName, height, step);
	map<string, int64> receiverMap;

	if (coinAddressStrings.size() == 0)
	{
		cout << "No coin addresses were found, there may be something wrong with the receiver_x.csv files." << endl;
		return false;
	}

	int64 sharePerAddress = share / (int64)coinAddressStrings.size();

	for (int i = 0; i < coinAddressStrings.size(); i++)
		receiverMap[coinAddressStrings[i]] = (int64)0;

	for (int i = 0; i < addressStrings.size(); i++)
	{
		string addressString = addressStrings[i];

		if (receiverMap.count(addressString) > 0)
			receiverMap[addressString] += amounts[i];
	}

	for (int i = 0; i < coinAddressStrings.size(); i++)
	{
		if (receiverMap[coinAddressStrings[i]] < sharePerAddress)
		{
			cout << endl << "In receiver.h, getIsSufficientAmount rejected the addresses or amounts." << endl;
			cout << "For the given:" << endl;
			cout << "Height: " << height << endl;
			cout << "Share: " << share << endl;
			cout << "Step: " << step << endl;
			cout << "The expected addresses are:" << endl;
			cout << "The receiverMap shows that they should get " << receiverMap[coinAddressStrings[i]] << " coins" << endl;
			cout << "The shares per address however are set to " << sharePerAddress << endl;
			for (int i = 0; i < coinAddressStrings.size(); i++)
				cout << coinAddressStrings[i] << endl;

			cout << endl << "The given addresses are:" << endl;

			for (int i = 0; i < addressStrings.size(); i++)
				cout << addressStrings[i] << endl;

			cout << endl << "The given amounts are:" << endl;

			for (int i = 0; i < amounts.size(); i++)
				cout << amounts[i] << endl;

			cout << endl;
			return false;
		}
	}

	return true;
}

// Get the directory path joined with the file name.
string getJoinedPath(const string& directoryPath, const string& fileName)
{
	filesystem::path completePath = filesystem::system_complete(filesystem::path(directoryPath));
	return (completePath / (filesystem::path(fileName))).string();
}

// Get the page by the address, be it a file name or hypertext address.
string getLocationText(const string& address)
{

  if (getStartsWith(address, string("https://")) || getStartsWith(address, string("http://")))
	return getHttpsText(address);

  return getFileText(address);
}

// Get the pages by the addresses, be they file names or hypertext addresses.
vector<string> getLocationTexts(vector<string> addresses)
{
	vector<string> locationTexts;

	for(int addressIndex = 0; addressIndex < addresses.size(); addressIndex++)
		locationTexts.push_back(getLocationText(addresses[addressIndex]));

	return locationTexts;
}

// Get the lowercase string.
string getLower(const string& text)
{
	int textLength = text.length();
	string lower = text.substr();

	for(int characterIndex = 0; characterIndex < textLength; characterIndex++)
	{
		lower[characterIndex] = tolower(text[characterIndex]);
	}

	return lower;
}

// Get the peer names from the text.
vector<string> getPeerNames(const string& text)
{
	bool isPeerSection = false;
	vector<string> peerNames;
	vector<string> textLines = getTextLines(text);

	for (int lineIndex = 0; lineIndex < textLines.size(); lineIndex++)
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

// Get the string with the search string replaced with the replace string.
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

// Determine if the first string starts with the second string.
bool getStartsWith(const string& firstString, const string& secondString)
{
	if (firstString.substr(0, secondString.size()) == secondString)
		return true;

	return false;
}

// Get the step file name by the file name.
string getStepFileName(const string& fileName, int height, int step)
{
	return getSuffixedFileName(fileName, getStringByInt(height / step));
}

// Get the step output according to the peers listed in a file.
string getStepOutput(const string& directoryPathInput, const string& fileName, int height, int step)
{

	string directoryPath = string();

	if (directoryPathInput != string())
		directoryPath = getJoinedPath(directoryPathInput, fileName.substr(0, fileName.rfind('.')));

	// To fix wrong receiver file problem and old receiver chain problem.
//	string stepFileName = getStepFileName(fileName, height, step);
//	if (stepFileName == string("receiver_1.csv"))
//	{
//		if (getExists(stepFileName))
//			writeFileTextByDirectory(directoryPath, stepFileName, getFileText(stepFileName));
//	}

//	if (stepFileName == string("receiver_2.csv"))
//	{
//		if (getExists(stepFileName))
//			writeFileTextByDirectory(directoryPath, stepFileName, getFileText(stepFileName));
//	}
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

// Get the step text by the file name.
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
//			return getFileText(directorySubName);
	
	string stepText = getFileText(stepFileName);

	if (stepText == string())
	{
		if (stepFileName == string("receiver_0.csv"))
		{
			cout << "Downloading " << stepFileName << " base file." << directorySubName << endl;
			string peerText = string("_beginpeers\n");
			peerText += string("http://galaxies.mygamesonline.org/receiver.csv\n");
			peerText += string("http://devcoinpool.btc-music.com/receiver/receiver.csv\n");
			peerText += string("http://devcoinblockexplorer.info/receiver/receiver.csv\n");
			peerText += string("http://devtome.com/files/receiver.csv\n");
			peerText += string("http://devcoin.darkgamex.ch/receiver.csv\n");
			peerText += string("_endpeers\n");
			stepText = getCommonOutputByText(peerText, string("0"));
			if (getStartsWith(stepText, string("Format,pluribusunum")))
                           writeFileText(directorySubName, stepText);
		}
		if (stepFileName == string("receiverTestNet_0.csv"))
		{
			cout << "Downloading " << stepFileName << " base file." << directorySubName << endl;
			string peerText = string("_beginpeers\n");
			peerText += string("https://raw.github.com/sidhujag/devcoin/master/src/node2/testnet3/receiverTestNet/receiverTestNet.csv\n");
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

// Get the step text recursively.
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
	}

	return previousText;
}

// Get the string from the boolean.
string getStringByBoolean(bool boolean)
{
	if (boolean)
		return string("true");
	return string("false");
}

// Get the string from the double precision float.
string getStringByDouble(double doublePrecision)
{
	ostringstream doubleStream;

	doubleStream << doublePrecision;

	return doubleStream.str();
}

// Get the string from the integer.
string getStringByInt(int integer)
{
	ostringstream integerStream;

	integerStream << integer;

	return integerStream.str();
}

// Get the file name with the suffix just before the extension.
string getSuffixedFileName(const string& fileName, const string& suffix)
{
	if (suffix == string())
		return fileName;

	int lastDotIndex = fileName.rfind(".");

	if (lastDotIndex == string::npos)
		return fileName + suffix;
	return fileName.substr(0, lastDotIndex) + string("_") + suffix + fileName.substr(lastDotIndex);
}

// Get the file names with the suffixes just before the extension.
vector<string> getSuffixedFileNames(vector<string> fileNames, const string& suffix)
{
	vector<string> suffixedFileNames;

	for(int fileNameIndex = 0; fileNameIndex < fileNames.size(); fileNameIndex++)
	{
		string fileName = fileNames[fileNameIndex];
		int doNotAddSuffixIndex = fileName.find("_do_not_add_suffix_");

		if (doNotAddSuffixIndex == string::npos)
			suffixedFileNames.push_back(getSuffixedFileName(fileName, suffix));
		else
			suffixedFileNames.push_back(fileName.substr(0, doNotAddSuffixIndex));
	}

	return suffixedFileNames;
}

// Get all the lines of text of a text.
vector<string> getTextLines(const string& text)
{
	return getTokens(getReplaced(getReplaced(text, string("\r"), string("\n")), string("\n\n"), string("\n")), string("\n"));
}

// Get the text without whitespace, joined with newlines in between.
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

// Get the tokens of the text split by the delimeters.
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

// Make a directory if it does not already exist.
void makeDirectory(const string& directoryPath)
{
	if (getReplaced(directoryPath) == string() || directoryPath == string("."))
		return;

	if (getExists(directoryPath))
		return;

	if (filesystem::create_directories(filesystem::path(directoryPath)))
		printf("The following directory was made: %s\n", directoryPath.c_str());
	else
		printf("Receiver.h can not make the directory %s so give it read/write permission for that directory.\n", directoryPath.c_str());
}

// Write a text to a file.
void writeFileText(const string& fileName, const string& fileText)
{
	if (fileText == string())
	{
		cout << "Warning, writeFileText in receiver.h won't write the file:\n" << fileName << "\nbecause the text is blank.\n";
		return;
	}

	makeDirectory(getDirectoryPath(fileName));
	ofstream fileStream(fileName.c_str());

	if (fileStream.is_open())
	{
	  fileStream << fileText;
	  fileStream.close();
	}
	else printf("The file %s can not be written to.\n", fileName.c_str());
}

// Write a text to a file joined to the directory path.
void writeFileTextByDirectory(const string& directoryPath, const string& fileName, const string& fileText)
{
	writeFileText(getJoinedPath(directoryPath, fileName), fileText);
}

// Write next step file if height is higher than the threshold.
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
