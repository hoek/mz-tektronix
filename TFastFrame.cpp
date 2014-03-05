#include "TFastFrame.h"

ClassImp(TFastFrame);

TFastFrame::TFastFrame(string cUserDataFile):TObject(){

	// +++ open data file and read in data +++
	if(cUserDataFile.empty()){
		Error("TFastFrame","User filename is empty");
		return;
	}
	OpenFile(cUserDataFile);
	if(this->IsZombie())
		return;
	ExtractHeaderData();
	ExtractTimestamps();
}

TFastFrame::~TFastFrame(){
	if(fileUserData!=NULL) delete fileUserData;
	fSglFrmAmplitudes.clear();
	fTimestamps.clear();
}

TGraph TFastFrame::DrawFrame(Int_t nUserFrame){
	// +++ include checks of event range +++
	ExtractFrameData(nUserFrame);
	TGraph grSingleFrame(HeaderData.nRecordLength,&fTimestamps[0],&fSglFrmAmplitudes[0]);
	std::stringstream cGraphName;
	cGraphName << "FastFrameEvent_" << nUserFrame;
	grSingleFrame.SetName(cGraphName.str().c_str());
	std::stringstream cGraphTitle;
	cGraphTitle << "FastFrame - Event " << nUserFrame << "; time; amplitude";
	grSingleFrame.SetTitle(cGraphTitle.str().c_str());
	return (grSingleFrame);
}

Bool_t TFastFrame::ExtractFrameData(Int_t nUserFrameIndex){
	fSglFrmAmplitudes.clear();
	Double_t *fTempSglFrmAmplitudes = new Double_t[HeaderData.nRecordLength];
	TBranch *bSglFrameAmplitudes = tAmplitudeData->GetBranch(AMPLITUDES_BRANCH_NAME);
	bSglFrameAmplitudes->SetAddress(fTempSglFrmAmplitudes);
	if(tAmplitudeData->GetEvent(nUserFrameIndex) == 0){
		delete[] fTempSglFrmAmplitudes;
		return (kFALSE);
	}
	fSglFrmAmplitudes.clear();
	fSglFrmAmplitudes.reserve(HeaderData.nRecordLength);
	fSglFrmAmplitudes.assign(fTempSglFrmAmplitudes,fTempSglFrmAmplitudes+HeaderData.nRecordLength);
	delete[] fTempSglFrmAmplitudes;
	return (kTRUE);
}

void TFastFrame::ExtractHeaderData(){
	// +++ set branch addresses +++
	TBranch *bHdrBranchRecLen = tHeaderData->GetBranch(HEADER_BRANCH_NAME_RECORD_LENGTH);
	bHdrBranchRecLen->SetAddress(&HeaderData.nRecordLength);
	TBranch *bHdrBranchSmpInt = tHeaderData->GetBranch(HEADER_BRANCH_NAME_SAMPLE_INTERVAL);
	bHdrBranchSmpInt->SetAddress(&HeaderData.fSampleInterval);
	TBranch *bHdrBranchTrigPnt = tHeaderData->GetBranch(HEADER_BRANCH_NAME_TRIGGER_POINT);
	bHdrBranchTrigPnt->SetAddress(&HeaderData.nTriggerPoint);
	TBranch *bHdrBranchTrigTime = tHeaderData->GetBranch(HEADER_BRANCH_NAME_TRIGGER_TIME);
	bHdrBranchTrigTime->SetAddress(&HeaderData.fTriggerTime);
	TBranch *bHdrBranchHorOff = tHeaderData->GetBranch(HEADER_BRANCH_NAME_HOR_OFFSET);
	bHdrBranchHorOff->SetAddress(&HeaderData.fHorizontalOffset);
	TBranch *bHdrBranchFstFrmCount = tHeaderData->GetBranch(HEADER_BRANCH_NAME_FRAME_COUNT);
	bHdrBranchFstFrmCount->SetAddress(&HeaderData.nFastFrameCount);
	// +++ read header data from TTree +++
	tHeaderData->GetEvent(0); // only one event in this TTree	
}

void TFastFrame::ExtractTimestamps(){
	Double_t *fTempTimestamps = new Double_t[HeaderData.nRecordLength];
	TBranch *bTimestmpData = tTimestampData->GetBranch(TIMESTAMPS_BRANCH_NAME);
	bTimestmpData->SetAddress(fTempTimestamps);
	tTimestampData->GetEvent(0); // only one event in this TTree
	fTimestamps.clear();
	fTimestamps.reserve(HeaderData.nRecordLength);
	fTimestamps.assign(fTempTimestamps,fTempTimestamps+HeaderData.nRecordLength);
	delete[] fTempTimestamps;
}

std::vector<Double_t> TFastFrame::GetSglFrameAmpl(Int_t nUserFrame){
	fSglFrmAmplitudes.clear(); // empty frame amplitude vector
	// +++ get event data +++
	ExtractFrameData(nUserFrame);
	return (fSglFrmAmplitudes);
}

TWaveform TFastFrame::GetWaveform(Int_t nUserFrame){
	ExtractFrameData(nUserFrame);
	TWaveform SglFrameData(fSglFrmAmplitudes,fTimestamps);
	return (SglFrameData);
}

void TFastFrame::OpenFile(string cUserDataFile){
	// +++ open ROOT file +++
	fileUserData = new TFile(cUserDataFile.c_str(),"READ");
	if(fileUserData->IsZombie()){
		Error("TFastFrame::OpenFile","File %s cannot be read!",cUserDataFile.c_str());	
		delete fileUserData;
		fileUserData = NULL;
		MakeZombie();
		return;
	}
	// +++ extract TTrees from ROOT file +++
	tHeaderData = (TTree*)fileUserData->Get(HEADER_TREE_NAME);
	tTimestampData = (TTree*)fileUserData->Get(TIMESTAMPS_TREE_NAME);
	tAmplitudeData = (TTree*)fileUserData->Get(AMPLITUDES_TREE_NAME);
}