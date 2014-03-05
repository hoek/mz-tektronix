#include "TWaveform.h"

ClassImp(TWaveform);

TWaveform::TWaveform( std::vector<Double_t> fUserAmplitudes, std::vector<Double_t> fUserTimestamps) : TObject(){ // standard constructor
	Init();
	if(fUserAmplitudes.empty() || fUserTimestamps.empty()){
		MakeZombie();
		return;
	}
	if(fUserAmplitudes.size() != fUserTimestamps.size()){
		MakeZombie();
		return;
	}
	fTimestamps = fUserTimestamps;
	fAmplitudes = fUserAmplitudes;
}

TWaveform::TWaveform( Double_t *fUserAmplitudes, Double_t *fUserTimestamps, Int_t nUserSampleLength){
	//Init();
	if(fUserAmplitudes==NULL || fUserTimestamps==NULL || nUserSampleLength<1){
		MakeZombie();
		return;
	}
	fTimestamps.assign(fUserTimestamps,fUserTimestamps+nUserSampleLength);
	fAmplitudes.assign(fUserAmplitudes,fUserAmplitudes+nUserSampleLength);
}

TWaveform::TWaveform(const TWaveform& UserWaveform) : TObject(UserWaveform){ // copy constructor
	fTimestamps = UserWaveform.fTimestamps;
	fAmplitudes = UserWaveform.fAmplitudes;
	fIntplConst	= UserWaveform.fIntplConst;
	fBaselineOffset = UserWaveform.fBaselineOffset;
	kIsInterpolated = UserWaveform.kIsInterpolated;
	fTimingPrecision = UserWaveform.fTimingPrecision;
}

TWaveform::~TWaveform(){

}

TWaveform TWaveform::Add(TWaveform UserAddend){
	std::vector<Double_t> fSum;
	std::vector<Double_t> fCommonTimestamps;
	for(Int_t i=0; i<fTimestamps.size(); i++){
		if(fTimestamps.at(i)>=UserAddend.fTimestamps.front() && fTimestamps.at(i)<=UserAddend.fTimestamps.back()){
			Double_t fTempSum = fAmplitudes.at(i) + UserAddend.Evaluate(fTimestamps.at(i));
			fSum.push_back(fTempSum);
			fCommonTimestamps.push_back(fTimestamps.at(i));
		}
	}
	return(TWaveform(fSum,fCommonTimestamps));
}

void TWaveform::CheckUserRange(Int_t &nUserStartIndex, Int_t &nUserStopIndex) const{
	// check user start and stop indices
	if(nUserStartIndex>nUserStopIndex) swap(nUserStartIndex,nUserStopIndex);
	if(nUserStartIndex<0) nUserStartIndex = 0;
	if(nUserStopIndex>(fTimestamps.size()-1)) nUserStopIndex = fTimestamps.size()-1;
}

TGraph TWaveform::Draw(){
	TGraph grWaveform(fAmplitudes.size(),&fTimestamps[0],&fAmplitudes[0]);
	grWaveform.SetName("grWaveform"); grWaveform.SetTitle("Waveform; time; amplitude");
	return (grWaveform);
}

Double_t TWaveform::Evaluate(Double_t fUserDatum){ // evaluation of waveform at arbitrary time
	Double_t fWaveformAmplitude = 0.0;
	if(fUserDatum < fTimestamps.at(0) || fUserDatum > fTimestamps.at(fTimestamps.size()-1)){
		cout << fUserDatum << "is out of sampled waveform range!" << endl;
		return (-9999);
	}
	if(!kIsInterpolated) Interpolate(); // create interpolation constants
	for(Int_t i=1; i<fTimestamps.size(); i++){
		if((fTimestamps.at(i)-fUserDatum) > 0.0){
			fWaveformAmplitude = fAmplitudes.at(i-1) + fIntplConst.at(i-1)*(fUserDatum-fTimestamps.at(i-1));
			break;
		}
	}
	return (fWaveformAmplitude);
}

void TWaveform::Export(string cUserFilename) const {
	ofstream UserExportfile(cUserFilename.c_str()); // open csv file
	if(UserExportfile.fail()){ // if opening fails, exit
		cerr << "Failed to open " << cUserFilename << "!" << endl;
		return;
	}
	std::vector<Double_t>::const_iterator TimestampIndex; // iterator for timestamp vector
	std::vector<Double_t>::const_iterator AmplitudeIndex; // iterator for amplitude vector
	for(TimestampIndex=fTimestamps.begin(), AmplitudeIndex=fAmplitudes.begin(); TimestampIndex!=fTimestamps.end(); TimestampIndex++, AmplitudeIndex++){
		UserExportfile << *TimestampIndex << " , " << *AmplitudeIndex << endl; // write timestamp and amplitude to file
	}
	UserExportfile.close(); // close csv file
}

Double_t FindWaveformRoot(TWaveform& UserWaveform, Double_t fTargetValue, Double_t fTimeMin, Double_t fTimeMax, Double_t fPrecision, Double_t fDeltaRoot, Int_t nMaxIter){
	Double_t fFcnLeft = UserWaveform.Evaluate(fTimeMin) - fTargetValue;
	Double_t fIntervalLength = fTimeMax - fTimeMin;
	Double_t fIntervalMidPoint;

	for(Int_t i=0; i<nMaxIter; i++){
		fIntervalLength *= 0.5; // shrink interval by half
		fIntervalMidPoint = fTimeMin + fIntervalLength; // update interval middle point
		Double_t fFcnMidPoint = UserWaveform.Evaluate(fIntervalMidPoint) - fTargetValue; // evaluate waveform at new interval midpoint
		if( fabs(fIntervalLength)<fDeltaRoot || fabs(fFcnMidPoint)<fPrecision ){ 
			//cout << i << ", " << fFcnMidPoint << ", " << fIntervalLength << ", " << fIntervalMidPoint << endl;
			return (fIntervalMidPoint);
		}
		( (fFcnLeft>0.0 && fFcnMidPoint<0.0) || (fFcnLeft<0.0 && fFcnMidPoint>0.0) ) ? (fTimeMax=fIntervalMidPoint) : (fTimeMin=fIntervalMidPoint, fFcnLeft=fFcnMidPoint);
	}
	return (fIntervalMidPoint);
}

Double_t TWaveform::GetArea(Int_t nUserStartIndex, Int_t nUserStopIndex){
	CheckUserRange(nUserStartIndex,nUserStopIndex);
	//if(nUserStartIndex>nUserStopIndex) swap(nUserStartIndex,nUserStopIndex);
	//if(nUserStartIndex<0) nUserStartIndex = 0;
	//if(nUserStopIndex>(fTimestamps.size()-1)) nUserStopIndex = fTimestamps.size()-1;
	Double_t fSignalArea = 0.0;
	for(Int_t i=nUserStartIndex; i<nUserStopIndex; i++){
		fSignalArea += fAmplitudes.at(i) * (fTimestamps.at(i+1)-fTimestamps.at(i));
	}
	return (fSignalArea);
}

Double_t TWaveform::GetMean(Int_t nUserStartIndex, Int_t nUserStopIndex) const{
	CheckUserRange(nUserStartIndex,nUserStopIndex);
	//if(nUserStartIndex>nUserStopIndex) swap(nUserStartIndex,nUserStopIndex);
	//if(nUserStartIndex<0) nUserStartIndex = 0;
	//if(nUserStopIndex>(fTimestamps.size()-1)) nUserStopIndex = fTimestamps.size()-1;
	Double_t fAvgAmplitude = std::accumulate(fAmplitudes.begin()+nUserStartIndex,fAmplitudes.begin()+nUserStopIndex+1,0.0);
	fAvgAmplitude /= (Double_t)(nUserStopIndex-nUserStartIndex+1);
	return (fAvgAmplitude);
}

Double_t TWaveform::GetNegFallTime(Double_t fUserLevelLow, Double_t fUserLevelHigh){
	Double_t fEdgeLevelLow	= fabs(fUserLevelLow)*GetMinAmplitude();
	Double_t fEdgeLevelHigh = fabs(fUserLevelHigh)*GetMinAmplitude();
	Bool_t bLowLevelDetected	= kFALSE;
	Bool_t bHighLevelDetected	= kFALSE;
	Double_t fEdgeStart = 0.0;
	Double_t fEdgeStop	= 0.0;
	Double_t fAbsTimingPrecision;
	for(std::vector<Double_t>::iterator CurrentIndex=fAmplitudes.begin()+1; CurrentIndex<fAmplitudes.end(); CurrentIndex++){ // begin of loop over all amplitude entries
		if(*CurrentIndex<fEdgeLevelLow && !bLowLevelDetected){ // detect start of edge
			bLowLevelDetected = kTRUE;
			fAbsTimingPrecision = fTimingPrecision * (fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex))-fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex)-1));
			fEdgeStart = FindWaveformRoot(*this,fEdgeLevelLow,fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex)-1),fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex)),1.0e-6,fAbsTimingPrecision);
		}
		if(bLowLevelDetected && *CurrentIndex<fEdgeLevelHigh){ // detect end of edge
			bHighLevelDetected = kTRUE;
			fAbsTimingPrecision = fTimingPrecision * (fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex))-fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex)-1));
			fEdgeStop = FindWaveformRoot(*this,fEdgeLevelHigh,fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex)-1),fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex)),1.0e-6,fAbsTimingPrecision);
			break;
		}
	} // end of loop over all amplitude entries
	if(!bLowLevelDetected || !bHighLevelDetected){ // return error if either low or high level has not been found
		return (-1.0);
	}
	return (fEdgeStop-fEdgeStart);
}

Double_t TWaveform::GetNegWidth(Int_t nUserStartIndex, Int_t nUserStopIndex, Double_t fUserLevel, Bool_t bIsAbsolute){
	CheckUserRange(nUserStartIndex,nUserStopIndex);
	Double_t fLevel = (bIsAbsolute) ? fUserLevel : GetMinAmplitude()*fabs(fUserLevel);
	Int_t nLeftIndex, nRightIndex;
	nLeftIndex	= -1; // set left index marker to invalid value
	nRightIndex = -1; // set right index marker to invalid value
	std::vector<Double_t>::iterator CurrentIndex;
	for(CurrentIndex=min_element(fAmplitudes.begin()+nUserStartIndex,fAmplitudes.begin()+nUserStopIndex); CurrentIndex>=fAmplitudes.begin()+nUserStartIndex; --CurrentIndex){ // search backwards for left-hand edge
		if(*CurrentIndex>fLevel){
			nLeftIndex = distance(fAmplitudes.begin(),CurrentIndex); // get index of this element
			break; // exit from for-loop
		}
		if(distance(fAmplitudes.begin(),CurrentIndex)==0){ // this is the first element
			break; // exit from for loop
		}
	}
	for(CurrentIndex=min_element(fAmplitudes.begin()+nUserStartIndex,fAmplitudes.begin()+nUserStopIndex); CurrentIndex<fAmplitudes.begin()+nUserStopIndex; ++CurrentIndex){ // search forwards for right-hand edge
		if(*CurrentIndex>fLevel){
			nRightIndex = distance(fAmplitudes.begin(),CurrentIndex); // get index of this element
			break; // exit from for-loop
		}
		if(distance(fAmplitudes.end(),CurrentIndex)==0){ // this is the last element
			break; // exit from for-loop
		}
	}
	if(nLeftIndex<0 || nRightIndex<0)
		return (-1.0); // return invalid width
	Double_t fAbsTimingPrecision = (fTimestamps.at(nLeftIndex+1)-fTimestamps.at(nLeftIndex)) * fTimingPrecision; // set timing precision to one per mill of sampling time interval
	Double_t fLeftMarker	= FindWaveformRoot(*this,fLevel,fTimestamps.at(nLeftIndex),fTimestamps.at(nLeftIndex+1),1.0e-6,fAbsTimingPrecision);
	Double_t fRightMarker	= FindWaveformRoot(*this,fLevel,fTimestamps.at(nRightIndex-1),fTimestamps.at(nRightIndex),1.0e-6,fAbsTimingPrecision);
	Double_t fWidth = fRightMarker - fLeftMarker;
	return (fWidth);
}

Double_t TWaveform::GetPosRiseTime(Double_t fUserLevelLow, Double_t fUserLevelHigh){
	Double_t fEdgeLevelLow	= fabs(fUserLevelLow)*GetMaxAmplitude();
	Double_t fEdgeLevelHigh = fabs(fUserLevelHigh)*GetMaxAmplitude();
	Bool_t bLowLevelDetected	= kFALSE;
	Bool_t bHighLevelDetected	= kFALSE;
	Double_t fEdgeStart = 0.0;
	Double_t fEdgeStop	= 0.0;
	Double_t fAbsTimingPrecision;
	for(std::vector<Double_t>::iterator CurrentIndex=fAmplitudes.begin()+1; CurrentIndex<fAmplitudes.end(); CurrentIndex++){ // begin of loop over all amplitude entries
		if(*CurrentIndex>fEdgeLevelLow && !bLowLevelDetected){ // detect start of edge
			bLowLevelDetected = kTRUE;
			fAbsTimingPrecision = fTimingPrecision * (fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex))-fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex)-1));
			fEdgeStart = FindWaveformRoot(*this,fEdgeLevelLow,fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex)-1),fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex)),1.0e-6,fAbsTimingPrecision);
		}
		if(bLowLevelDetected && *CurrentIndex>fEdgeLevelHigh){ // detect end of edge
			bHighLevelDetected = kTRUE;
			fAbsTimingPrecision = fTimingPrecision * (fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex))-fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex)-1));
			fEdgeStop = FindWaveformRoot(*this,fEdgeLevelHigh,fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex)-1),fTimestamps.at(distance(fAmplitudes.begin(),CurrentIndex)),1.0e-6,fAbsTimingPrecision);
			break;
		}
	} // end of loop over all amplitude entries
	if(!bLowLevelDetected || !bHighLevelDetected){ // return error if either low or high level has not been found
		return (-1.0);
	}
	return (fEdgeStop-fEdgeStart);
}

Double_t TWaveform::GetPosWidth(Int_t nUserStartIndex, Int_t nUserStopIndex, Double_t fUserLevel){
	CheckUserRange(nUserStartIndex,nUserStopIndex);
	Double_t fLevel = GetMaxAmplitude()*fabs(fUserLevel);
	Int_t nLeftIndex, nRightIndex;
	nLeftIndex	= -1; // set left index marker to invalid value
	nRightIndex = -1; // set right index marker to invalid value
	std::vector<Double_t>::iterator CurrentIndex;
	for(CurrentIndex=max_element(fAmplitudes.begin()+nUserStartIndex,fAmplitudes.begin()+nUserStopIndex); CurrentIndex>=fAmplitudes.begin()+nUserStartIndex; CurrentIndex--){ // search backwards for left-hand edge
		if(*CurrentIndex<fLevel){
			nLeftIndex = distance(fAmplitudes.begin(),CurrentIndex); // get index of this element
			break; // exit from for-loop
		}
	}
	for(CurrentIndex=max_element(fAmplitudes.begin()+nUserStartIndex,fAmplitudes.begin()+nUserStopIndex); CurrentIndex<fAmplitudes.begin()+nUserStopIndex; CurrentIndex++){ // search forwards for right-hand edge
		if(*CurrentIndex<fLevel){
			nRightIndex = distance(fAmplitudes.begin(),CurrentIndex); // get index of this element
			break; // exit from for-loop
		}
	}
	if(nLeftIndex<0 || nRightIndex<0)
		return (-1.0); // return invalid width
	Double_t fAbsTimingPrecision = (fTimestamps.at(nLeftIndex+1)-fTimestamps.at(nLeftIndex)) * fTimingPrecision; // set timing precision to one per mill of sampling time interval
	Double_t fLeftMarker	= FindWaveformRoot(*this,fLevel,fTimestamps.at(nLeftIndex),fTimestamps.at(nLeftIndex+1),1.0e-6,fAbsTimingPrecision);
	Double_t fRightMarker	= FindWaveformRoot(*this,fLevel,fTimestamps.at(nRightIndex-1),fTimestamps.at(nRightIndex),1.0e-6,fAbsTimingPrecision);
	Double_t fWidth = fRightMarker - fLeftMarker;
	return (fWidth);
}

Double_t TWaveform::GetRMS(Int_t nUserStartIndex, Int_t nUserStopIndex) const{
	CheckUserRange(nUserStartIndex,nUserStopIndex);
	Double_t fTotSum2 = std::inner_product(fAmplitudes.begin()+nUserStartIndex,fAmplitudes.begin()+nUserStopIndex+1,fAmplitudes.begin()+nUserStartIndex,0.0);
	Double_t fLength = (Double_t)(nUserStopIndex-nUserStartIndex+1);
	Double_t fMean = GetMean(nUserStartIndex,nUserStopIndex);
	Double_t fRms = sqrt(fabs(fTotSum2/fLength - fMean*fMean));
	return (fRms);
}

Int_t TWaveform::GetTimestampIndex(Double_t fUserDate){
	if(fUserDate<fTimestamps.front() || fUserDate>fTimestamps.back()){
		return (-1);
	}
	Int_t nNearestTimestampIndex = 0;
	Double_t fTimeGap = fabs(fTimestamps.at(0)-fUserDate);
	for(Int_t i=1; i<fTimestamps.size(); i++){
		Double_t fTempTimeGap = fabs(fTimestamps.at(i)-fUserDate);
		if(fTempTimeGap<fTimeGap){
			fTimeGap = fTempTimeGap;
			nNearestTimestampIndex = i;
		}
	}
	return (nNearestTimestampIndex);
}

void TWaveform::Init(){
	fBaselineOffset = 0.0;
	kIsInterpolated = kFALSE;
	fTimingPrecision = 0.001;
}

void TWaveform::Interpolate(){ // interpolation algorithm goes here...
	// do linear intrepolation for the moment
	// we will need to compute n-1 parameters
	for(Int_t i=0; i<fAmplitudes.size()-1; i++){
		Double_t fSlope = (fAmplitudes.at(i+1) - fAmplitudes.at(i)) / (fTimestamps.at(i+1) - fTimestamps.at(i));
		fIntplConst.push_back(fSlope);
	}
	kIsInterpolated = kTRUE;
}

void TWaveform::Invert(){
	if(kIsInterpolated){ // delete interpolation parameters
		fIntplConst.clear();
	}
	kIsInterpolated = kFALSE;
	for(Int_t i=0; i<fAmplitudes.size(); i++){
		fAmplitudes.at(i) = fAmplitudes.at(i) * -1.0;
	}
}

TWaveform TWaveform::MovingAverageFilter(Int_t nUserWindowSize){
	std::vector<Double_t> fFilteredAmplitudes;
	fFilteredAmplitudes.reserve(fTimestamps.size());
	std::vector<Double_t> fFilteredTimestamps;
	fFilteredTimestamps.reserve(fTimestamps.size());
	Double_t fTempAmpAccumulator	= 0.0;
	Double_t fTempTimeAccumulator	= 0.0;
	// +++ compute first filtered data point +++
	fTempAmpAccumulator = std::accumulate(fAmplitudes.begin(),fAmplitudes.begin()+nUserWindowSize,0.0);
	fFilteredAmplitudes.push_back(fTempAmpAccumulator/(Double_t)nUserWindowSize);
	fFilteredTimestamps.push_back(fTimestamps.at(0));
	// +++ now filter remaining waveform +++
	for(Int_t i=1; i<fAmplitudes.size()-nUserWindowSize+1; i++){
		fTempAmpAccumulator += fAmplitudes.at(i+nUserWindowSize-1) - fAmplitudes.at(i-1);
		fFilteredAmplitudes.push_back(fTempAmpAccumulator/(Double_t)nUserWindowSize);
		fFilteredTimestamps.push_back(fTimestamps.at(i));
	}
	return (TWaveform(fFilteredAmplitudes,fFilteredTimestamps));
}

TWaveform& TWaveform::operator=(const TWaveform& UserWaveform){
	if(this != &UserWaveform){
		TObject::operator=(UserWaveform);
		fTimestamps = UserWaveform.fTimestamps;
		fAmplitudes = UserWaveform.fAmplitudes;
		fIntplConst = UserWaveform.fIntplConst;
		fBaselineOffset = UserWaveform.fBaselineOffset;
		kIsInterpolated = UserWaveform.kIsInterpolated;
		fTimingPrecision = UserWaveform.fTimingPrecision;
	}
	return *this;
}

void TWaveform::Scale(Double_t fUserScaleFactor){
	for(Int_t i=0; i<fAmplitudes.size(); i++){
		fAmplitudes.at(i) *= fabs(fUserScaleFactor);
	}
	if(kIsInterpolated){ 
		fIntplConst.clear(); // delete interpolation parameters
		Interpolate(); // generate new interpolation parameters
	}
}

void TWaveform::ScaleTimestamps(Double_t fUserScaleFactor){
	for(Int_t i=0; i<fTimestamps.size(); i++){
		fTimestamps.at(i) *= fabs(fUserScaleFactor);
	}
	if(kIsInterpolated){ 
		fIntplConst.clear(); // delete interpolation parameters
		Interpolate(); // generate new interpolation parameters
	}
}

void TWaveform::ShiftBaseline(Double_t fUserOffset){
	fBaselineOffset = fUserOffset;
	for(Int_t i=0; i<fAmplitudes.size(); i++){
		fAmplitudes.at(i) -= fBaselineOffset;
	}
	if(kIsInterpolated){
		fIntplConst.clear(); // delete interpolation parameters
		Interpolate(); // generate new interpolation parameters
	}
}

void TWaveform::ShiftTimestamps(Double_t fUserDelay){
	for(Int_t i=0; i<fTimestamps.size(); i++){
		fTimestamps.at(i) += fUserDelay;
	}
	if(kIsInterpolated){
		fIntplConst.clear(); // delete interpolation parameters
		Interpolate(); // generate new interpolation parameters
	}
}
