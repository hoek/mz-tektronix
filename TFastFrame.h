#ifndef _T_FAST_FRAME_H
#define _T_FAST_FRAME_H
// +++ include header files +++
#include <vector>

#include "TGraph.h"

#include "myFastFrameConverter.h"
#include "TWaveform.h"

class TFastFrame : public TObject{
private:
	TFile *fileUserData;
	std::vector<Double_t> fSglFrmAmplitudes;
	std::vector<Double_t> fTimestamps;
	FASTFRAME_HEADER HeaderData;

	TTree *tHeaderData;
	TTree *tTimestampData;
	TTree *tAmplitudeData;

	Bool_t ExtractFrameData(Int_t nUserFrameIndex);
	void ExtractHeaderData();
	void ExtractTimestamps();
	void OpenFile(string cUserDataFile);
	TFastFrame(const TFastFrame &);
	void operator=(const TFastFrame &);
	//void ReadData();

public:	// public function of TFastFrame class
	TFastFrame(string cUserDataFile=""); // constructor
	~TFastFrame();	// destructor
	TGraph DrawFrame(Int_t nUserFrame);
	Int_t GetFrameCount() const { return (HeaderData.nFastFrameCount); }; // get number of frames in data set
	Double_t GetHorizontalOffset() const { return (HeaderData.fHorizontalOffset); }; // get temporal offset of trigger point from slice start
	Int_t GetRecordLength() const { return (HeaderData.nRecordLength); }; // get number of samples per frame
	Double_t GetSampleInterval() const { return (HeaderData.fSampleInterval); }; // get sampling interval (unit is s)
	std::vector<Double_t> GetSglFrameAmpl(Int_t nUserFrame); // get vector of amplitudes for one frame
	std::vector<Double_t> GetTimestamps() const { return (fTimestamps);	}; // get vector of timestamps
	Int_t GetTriggerPoint() const { return (HeaderData.nTriggerPoint); }; // get index of slice in which the trigger occurred
	Double_t GetTriggerTime() const { return (HeaderData.fTriggerTime); }; // 
	TWaveform GetWaveform(Int_t nUserFrame); // get waveform at given index
	/* some magic ROOT stuff... */
  ClassDef(TFastFrame,1);
};

#endif