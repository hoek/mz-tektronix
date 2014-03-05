#ifndef _MY_FAST_FRAME_CONVERTER_H
#define _MY_FAST_FRAME_CONVERTER_H
// +++ include header files +++
#include <bitset>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#include "TTree.h"
#include "TFile.h"

#include "myUtilities.h"

// +++ define special structures +++
struct FASTFRAME_HEADER{
	Int_t nRecordLength;		// number of sampled points per event
	Double_t fSampleInterval;	// time interval between two sampling points in s
	Int_t nTriggerPoint;		// sample point index of oscilloscope's trigger
	Double_t fTriggerTime;		// time between sample point timestamp and true trigger time
	Double_t fHorizontalOffset;	// horizontal offset of oscilloscope time base
	Int_t nFastFrameCount;		// number of events 
};

struct SINGLE_FRAME_DATA{
	std::vector<Double_t> fTimestamps;
	std::vector<Double_t> fAmplitudes;
	Double_t fAbsTriggerTimestamp;
};

// +++ define constants & TTree and TBranch names +++
#define N_HEADER_LINES 6
#define HEADER_TREE_NAME "tHeaderData"
#define HEADER_BRANCH_NAME_RECORD_LENGTH "nRecordLength"
#define HEADER_BRANCH_NAME_SAMPLE_INTERVAL "fSampleInterval"
#define HEADER_BRANCH_NAME_TRIGGER_POINT "nTriggerPoint"
#define HEADER_BRANCH_NAME_TRIGGER_TIME "fTriggerTime"
#define HEADER_BRANCH_NAME_HOR_OFFSET "fHorizontalOffset"
#define HEADER_BRANCH_NAME_FRAME_COUNT "nFastFrameCount"
#define TIMESTAMPS_TREE_NAME "tTimestampData"
#define TIMESTAMPS_BRANCH_NAME "fTimestamps"
#define AMPLITUDES_TREE_NAME "tAmplitudeData"
#define AMPLITUDES_BRANCH_NAME "fAmplitudes"

// +++ functions etc. +++
void ConvertFastFrameData(string cUserFileName="", string cUserColSep=",", Bool_t bIsGermanDecimal=kFALSE);
TTree* ParseForHeaderData(ifstream *myFile=NULL, FASTFRAME_HEADER *UserHeaderData=NULL, string cUserColSep=",", Bool_t bIsGermanDecimal=kFALSE);
TTree* ParseForAmplitudeData(ifstream *myFile=NULL, Int_t nRecordLength=-1, string cUserColSep=",", Bool_t bIsGermanDecimal=kFALSE);
TTree* ParseForTimestampData(ifstream *myFile=NULL, Int_t nRecordLength=-1, string cUserColSep=",", Bool_t bIsGermanDecimal=kFALSE);

#endif