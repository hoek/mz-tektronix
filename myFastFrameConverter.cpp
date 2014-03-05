#include "myFastFrameConverter.h"

void ConvertFastFrameData(string cUserFileName, string cUserColSep, Bool_t bIsGermanDecimal){
	// +++ open Fast Frame data file +++
	ifstream UserDataFile(cUserFileName.c_str()); // open data file
	if(UserDataFile.fail()){ // if opening fails, exit
		cerr << "Failed to open " << cUserFileName << "!" << endl;
		exit (-1);
	}
	// +++ create ROOT output file +++
	string cOutputFileName = cUserFileName + ".root"; // append .root to existing file name
	TFile OutputFile(cOutputFileName.c_str(),"RECREATE"); // create new ROOT file, if existing already it will be overwritten
	if (OutputFile.IsZombie()) { // if creating new ROOT file fails, exit program
       cout << "Error opening file" << endl;
		UserDataFile.close();
       exit(-1);
    }
	// +++ parse header information +++
	FASTFRAME_HEADER FastFrameHeaderData;
	TTree *tFastFrameHeaderData = ParseForHeaderData(&UserDataFile,&FastFrameHeaderData,cUserColSep,bIsGermanDecimal);
	if(tFastFrameHeaderData==NULL){
		cerr << "Error while parsing header data!" << endl;
		exit (-1);
	}
	// +++ parse timestamp data +++
	TTree *tFastFrameTimestamps = ParseForTimestampData(&UserDataFile,FastFrameHeaderData.nRecordLength,cUserColSep,bIsGermanDecimal);
	if(tFastFrameTimestamps==NULL){
		cerr << "Error while parsing timestamp data!" << endl;
		exit (-1);
	}
	// +++ parse amplitude data +++
	TTree *tFastFrameAmplitudes = ParseForAmplitudeData(&UserDataFile,FastFrameHeaderData.nRecordLength,cUserColSep,bIsGermanDecimal);
	if(tFastFrameAmplitudes==NULL){
		cerr << "Error while parsing amplitude data!" << endl;
		exit (-1);
	}
	if(FastFrameHeaderData.nFastFrameCount!=tFastFrameAmplitudes->GetEntries()){
		cerr << "Mismatch of decoded event numbers!" << endl;
		exit (-1);
	}
	// +++ write TTrees to output file +++
	OutputFile.cd();
	tFastFrameHeaderData->Write();
	tFastFrameTimestamps->Write();
	tFastFrameAmplitudes->Write();
	// +++ cleaning up +++
	UserDataFile.close();
	delete tFastFrameHeaderData;
	delete tFastFrameTimestamps;
	delete tFastFrameAmplitudes;
}

TTree* ParseForHeaderData(ifstream *myFile, FASTFRAME_HEADER *UserHeaderData, string cUserColSep, Bool_t bIsGermanDecimal){
	if(myFile->tellg()>0){
		myFile->clear();		// clear eof-bit
		myFile->seekg(0, ios::beg);	// reset stream to beginning
	}
	// create TTree for storing results
	static FASTFRAME_HEADER myHeaderData = {-1,-1.0,-1,0.0,0.0,-1};
	TTree *tUserHeaderData = new TTree(HEADER_TREE_NAME,"Tektronix Fast Frame Header Data");
	// split header data into separate branches!
	tUserHeaderData->Branch(HEADER_BRANCH_NAME_RECORD_LENGTH,&myHeaderData.nRecordLength,"nRecordLength/I");
	tUserHeaderData->Branch(HEADER_BRANCH_NAME_SAMPLE_INTERVAL,&myHeaderData.fSampleInterval,"fSampleInterval/D");
	tUserHeaderData->Branch(HEADER_BRANCH_NAME_TRIGGER_POINT,&myHeaderData.nTriggerPoint,"nTriggerPoint/I");
	tUserHeaderData->Branch(HEADER_BRANCH_NAME_TRIGGER_TIME,&myHeaderData.fTriggerTime,"fTriggerTime/D");
	tUserHeaderData->Branch(HEADER_BRANCH_NAME_HOR_OFFSET,&myHeaderData.fHorizontalOffset,"fHorizontalOffset/D");
	tUserHeaderData->Branch(HEADER_BRANCH_NAME_FRAME_COUNT,&myHeaderData.nFastFrameCount,"nFastFrameCount/I");
	// header format information (fixed)
	const int nHeaderLines = N_HEADER_LINES;
	string HeaderKeywords[nHeaderLines]; // replace this by a STL map
	HeaderKeywords[0] = "Record Length";
	HeaderKeywords[1] = "Sample Interval";
	HeaderKeywords[2] = "Trigger Point";
	HeaderKeywords[3] = "Trigger Time";
	HeaderKeywords[4] = "Horizontal Offset";
	HeaderKeywords[5] = "FastFrame Count";
	// start parsing file
	if(myFile == NULL){
		cerr << "Data file pointer is invalid!" << endl;
		exit(1);
	}
	// use stl:bitset here!!
	bitset<6> bpDecodedHeaderWords;
	std::vector<string> cHeaderTokens;
	Int_t nDecodedHeaderWords = 0;
	Int_t nLinesFound = 0;
	while(myFile->good() && bpDecodedHeaderWords.count()!= N_HEADER_LINES){ // loop over data file until all header lines are decoded or end of file is reached
		nLinesFound++; // increment number of lines in file
		//cout << nLinesFound << endl;
		string CurrentHeaderLine;
		size_t CurrentHeaderInfoPos[2];
		getline(*myFile,CurrentHeaderLine,'\n');
		if(CurrentHeaderLine[0] == '\"') // all header lines begin with ", so only attempt to parse if this is true
			cHeaderTokens = LineParser(CurrentHeaderLine,*cUserColSep.c_str());
		if(cHeaderTokens.size()>0){
			for(Int_t i=0; i<N_HEADER_LINES; i++){ // begin loop over header keywords
				if(cHeaderTokens[0].find(HeaderKeywords[i])!= string::npos){
					//cout << cHeaderTokens[0] << " " << HeaderKeywords[i] << " : " << cHeaderTokens[1] << endl;
					switch (i) { // all header data decoded corresponds to 0x3F
						case 0:
							myHeaderData.nRecordLength = atoi(cHeaderTokens[1].c_str());
							bpDecodedHeaderWords.set(0);
							break;
						case 1:
							if(bIsGermanDecimal){
								cHeaderTokens.at(1) += "." + cHeaderTokens.at(2);
								//cout << cHeaderTokens.at(1) << endl;
							}
							myHeaderData.fSampleInterval = atof(cHeaderTokens[1].c_str());
							bpDecodedHeaderWords.set(1);
							break;
						case 2:
							myHeaderData.nTriggerPoint = atoi(cHeaderTokens[1].c_str());
							bpDecodedHeaderWords.set(2);
							break;
						case 3:
							if(bIsGermanDecimal){
								cHeaderTokens.at(1) += "." + cHeaderTokens.at(2);
								//cout << cHeaderTokens.at(1) << endl;
							}
							myHeaderData.fTriggerTime = atof(cHeaderTokens[1].c_str());
							bpDecodedHeaderWords.set(3);
							break;
						case 4:
							if(bIsGermanDecimal){
								cHeaderTokens.at(1) += "." + cHeaderTokens.at(2);
								//cout << cHeaderTokens.at(1) << endl;
							}
							myHeaderData.fHorizontalOffset = atof(cHeaderTokens[1].c_str());
							bpDecodedHeaderWords.set(4);
							break;
						case 5:
							myHeaderData.nFastFrameCount = atoi(cHeaderTokens[1].c_str());
							bpDecodedHeaderWords.set(5);
							break;
					}
					break;
				} 
				//clog << nDecodedHeaderWords << ", " << CurrentHeaderLine << endl;
			} //  end of loop over header key words
		}
		cHeaderTokens.clear();
	} // end of loop over data file
	//cout << bpDecodedHeaderWords.to_string<char,char_traits<char>,allocator<char> >() << endl;
	if(myFile->eof() || bpDecodedHeaderWords.count() != N_HEADER_LINES){ // not all header data available
		if(!bpDecodedHeaderWords.test(0)){ // record length was not decoded
			cout << "Header decoding incomplete!" << endl;
			delete tUserHeaderData;
			delete UserHeaderData;
			return (NULL);
		}
		if(!bpDecodedHeaderWords.test(5)){ //  number of frames not in header information, so reconstruct it
			myHeaderData.nFastFrameCount = (nLinesFound-1)/myHeaderData.nRecordLength; // change this check to see if there is a reminder after the division!
			if(myHeaderData.nFastFrameCount*myHeaderData.nRecordLength != (nLinesFound-1)){
				cout << "Error reconstructing number of frames in file!" << endl;
				delete tUserHeaderData;
				delete UserHeaderData;
				return (NULL);
			}
		}
	}
	myFile->clear();		// clear eof-bit
	myFile->seekg(0, ios::beg);	// reset stream to beginning
	tUserHeaderData->Fill();
	if(UserHeaderData!=NULL){ // copy header data
		UserHeaderData->nRecordLength		= myHeaderData.nRecordLength;
		UserHeaderData->fSampleInterval		= myHeaderData.fSampleInterval;
		UserHeaderData->nTriggerPoint		= myHeaderData.nTriggerPoint;
		UserHeaderData->fTriggerTime		= myHeaderData.fTriggerTime;
		UserHeaderData->fHorizontalOffset	= myHeaderData.fHorizontalOffset;
		UserHeaderData->nFastFrameCount		= myHeaderData.nFastFrameCount;
	}
	return (tUserHeaderData);
}


TTree* ParseForAmplitudeData(ifstream *myFile, Int_t nRecordLength, string cUserColSep, Bool_t bIsGermanDecimal){
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Parse Fast Frame ASCII text file for amplitude data
	// Amplitude digitisation resolution is 8 bit (DPO7254)
	// Event length is given by nRecordLength
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	if(nRecordLength<1){
		cout << "Illegal record length: " << nRecordLength << endl;
		return (NULL);
	}
	if(myFile->tellg()>0){
		myFile->clear();		// clear eof-bit
		myFile->seekg(0, ios::beg);	// reset stream to beginning
	}
	// +++ create TTree for storing timestamp data +++
	std::vector<Double_t> fAmplitudes;
	fAmplitudes.reserve(nRecordLength); fAmplitudes.resize(nRecordLength,-9999.0);
	//Double_t *fAmplitudes = new Double_t[nRecordLength]; // this needs to be changed into a vector
	TTree *tUserAmplitudeData = new TTree(AMPLITUDES_TREE_NAME,"Tektronix Fast Frame Amplitude Data");
	std::stringstream cAmplitudeTreeEntry;
	cAmplitudeTreeEntry << "fAmplitudes[" << nRecordLength << "]/D";
	tUserAmplitudeData->Branch(AMPLITUDES_BRANCH_NAME,&fAmplitudes[0],cAmplitudeTreeEntry.str().c_str());
	// +++ extract amplitude data +++
	string cCurrentLine;
	Int_t nCurrentLineIndex		= 0;
	Int_t nCurrentEventIndex	= 0;
	std::vector<string> cDatumTokens;
	while(getline(*myFile,cCurrentLine,'\n')){ // loop over FastFrame data file
		string cTempDatum;
		cDatumTokens = LineParser(cCurrentLine,*cUserColSep.c_str()); // last column is amplitude (last two in case of German decimal identifier)
		if(cDatumTokens.size()<2){
			return (NULL);
		}
		if(bIsGermanDecimal){
			cTempDatum = cDatumTokens.at(cDatumTokens.size()-2) + "." + cDatumTokens.at(cDatumTokens.size()-1);
		}
		else{
			cTempDatum = cDatumTokens.at(cDatumTokens.size()-1);
		}
		fAmplitudes.at(nCurrentEventIndex) = atof(cTempDatum.c_str()); // convert datum word to double precision number
		nCurrentLineIndex++; nCurrentEventIndex++;
		if(nCurrentEventIndex==nRecordLength){
			tUserAmplitudeData->Fill();
			nCurrentEventIndex = 0; // reset event index
		}
		cDatumTokens.clear();
	} // end of loop over FastFrame data file
	// +++ reset file input stream status +++
	myFile->clear();		// clear eof-bit
	myFile->seekg(0, ios::beg);	// reset stream to beginning
	//delete[] fAmplitudes;
	fAmplitudes.clear();
	return (tUserAmplitudeData);
}

TTree* ParseForTimestampData(ifstream *myFile, Int_t nRecordLength, string cUserColSep, Bool_t bIsGermanDecimal){
	// +++ reset file reading position marker +++
	if(myFile->tellg()>0){
		myFile->clear();		// clear eof-bit
		myFile->seekg(0, ios::beg);	// reset stream to beginning
	}
	// +++ decode time base information +++
	if(nRecordLength<1){
		cout << "Illegal record length: " << nRecordLength << endl;
		return (NULL);
	}
	//Double_t *fTimestamps = new Double_t[nRecordLength];
	std::vector<Double_t> fTimestamps;
	fTimestamps.reserve(nRecordLength);
	// +++ extract first timestamp +++
	Double_t fFirstTimestamp;
	string cCurrentLine;
	std::vector<string> cTimestampTokens;
	getline(*myFile, cCurrentLine, '\n'); // extract one line of data from file
	cTimestampTokens = LineParser(cCurrentLine,*cUserColSep.c_str()); // last column is amplitude (last two in case of German decimal identifier)
	if(cTimestampTokens.size()<2){ // check if enough columns have been found
		return (NULL);
	}
	string cTempDatum;
	if(bIsGermanDecimal){
		cTempDatum = cTimestampTokens.at(cTimestampTokens.size()-4) + "." + cTimestampTokens.at(cTimestampTokens.size()-3);
	}
	else{
		cTempDatum = cTimestampTokens.at(cTimestampTokens.size()-2);
	}
	fFirstTimestamp = atof(cTempDatum.c_str());
	fTimestamps.push_back(fFirstTimestamp);
	while(myFile->good()){ // begin of loop over data file
		getline(*myFile, cCurrentLine, '\n'); // extract one line of data from file
		cTimestampTokens = LineParser(cCurrentLine,*cUserColSep.c_str()); // last column is amplitude (last two in case of German decimal identifier)
		if(cTimestampTokens.size()<2){ // check if enough columns have been found
			return (NULL);
		}
		string cTempDatum;
		if(bIsGermanDecimal){
			cTempDatum = cTimestampTokens.at(cTimestampTokens.size()-4) + "." + cTimestampTokens.at(cTimestampTokens.size()-3);
		}
		else{
			cTempDatum = cTimestampTokens.at(cTimestampTokens.size()-2);
		}
		if(fFirstTimestamp==atof(cTempDatum.c_str()))
			break;
		fTimestamps.push_back(atof(cTempDatum.c_str()));

		cTimestampTokens.clear();
		cCurrentLine.clear();
	} // end of loop over data file
	cout << fTimestamps.size() << " : " << nRecordLength << endl;
	if(fTimestamps.size()!=nRecordLength){
		return (NULL);
	}
	// +++ create TTree for storing timestamp data +++
	TTree *tUserTimestampData = new TTree(TIMESTAMPS_TREE_NAME,"Tektronix Fast Frame Timestamp Data");
	std::stringstream cTimestampTreeEntry;
	cTimestampTreeEntry << "fTimestamps[" << nRecordLength << "]/D";
	tUserTimestampData->Branch(TIMESTAMPS_BRANCH_NAME,&fTimestamps[0],cTimestampTreeEntry.str().c_str());
	// +++ fill TTree +++
	tUserTimestampData->Fill();
	// +++ reset file input stream status +++
	myFile->clear();		// clear eof-bit
	myFile->seekg(0, ios::beg);	// reset stream to beginning
	//delete[] fTimestamps;
	fTimestamps.clear();
	return (tUserTimestampData);
}