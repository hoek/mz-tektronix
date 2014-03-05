#include "myUtilities.h"

std::vector<string> LineParser(string cUserLine, char cUserDelimiter, Bool_t bVerboseMode){
	// parse line provided by user and return a vector of strings containing individual tokens
	// user needs to provide column separator


	std::vector<string> cTokens; // vector for storing individual tokens from lime
	cTokens.reserve(TOKEN_SIZE);

	if(cUserLine.empty()) // return if user provided line is empty
		return (cTokens);

	string cBuffer; // buffer for storing temorary token
	stringstream cParsingLine(cUserLine);
	//while(!cParsingLine.eof()){ // parse string containing line
	while(std::getline(cParsingLine,cBuffer,cUserDelimiter)){ 
		//cParsingLine >> cBuffer; // extract token from line and store in temporary variable
		if(!cBuffer.empty()) // if token is not empty add to list of tokens
			cTokens.push_back(cBuffer);
		cBuffer.clear(); // empty temporary variable 
	}
	if(bVerboseMode){
		for(std::vector<string>::const_iterator ShowTokens=cTokens.begin(); ShowTokens!=cTokens.end(); ShowTokens++){
			cout << *ShowTokens << endl;
		}
	}

	return (cTokens);
}