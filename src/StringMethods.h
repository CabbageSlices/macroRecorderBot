#ifndef _STRINGMETHODS_HPP_INCLUDED
#define _STRINGMETHODS_HPP_INCLUDED

#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using std::vector;
using std::fstream;
using std::string;
using std::stringstream;
using std::getline;

/**
 * @brief      Loads a file to string.
 *
 * @param[in]  fileName  The file name
 *
 * @return     a string containing the contents of the file, or an empty string if an error occured.
 */
inline string loadFileToString(const string &fileName) {

	fstream file(fileName);

	if(!file)
		return "";

	string fileAsString{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

	file.close();
	return fileAsString;
}

inline string extractLine(const string &targetString, const size_t &linePos) {

	size_t newlinePos = targetString.find('\n', linePos);
	return targetString.substr(linePos, newlinePos - linePos);

}

/**
 * @brief Starting from the given position, search the given string for a sentance wrapped in double quotes
 * The search will only continue until the end of line
 * returns the string inside of the quotation words without the surrounding quotes, or empty string if none exists
 * 
 * @param stringToSearch string that needs to be searched
 * @param linePos position in the string to begin the search
 * @return string the word that is located in the quotes. Empty string if there is no quoted string in the line
 */
inline string extractStringFromLine(const string &stringToSearch, const size_t &linePos) {

	size_t quoteStartPos = stringToSearch.find('\"', linePos);
	size_t quoteEndPos = stringToSearch.find('\"', quoteStartPos + 1);

	if(quoteStartPos == string::npos || quoteEndPos == string::npos)
		return "";

	return stringToSearch.substr(quoteStartPos + 1, quoteEndPos - quoteStartPos - 1);
}

//takes the given string, splits it up into words according to the delimiter
//puts the results in the given vector
inline void separateIntoWords(const string &stringToSplit, vector<string> &destination, char delimiter) {

	stringstream sstream(stringToSplit);
	string word;

	while(getline(sstream, word, delimiter)) {
		destination.push_back(word);
	}
}

#endif