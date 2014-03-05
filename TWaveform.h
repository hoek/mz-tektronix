#ifndef _T_WAVEFORM_H
#define _T_WAVEFORM_H
// +++ include header files +++
// standard C++ header
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

// ROOT header
#include "TAxis.h"
#include "TGraph.h"

// +++ class definition +++
class TWaveform : public TObject{
private:
	std::vector<Double_t> fTimestamps; // vector for storing timestamps of this waveform
	std::vector<Double_t> fAmplitudes; // vector for storing amplitudes of this waveform
	std::vector<Double_t> fIntplConst; // vector for storing interpolation constants of this waveform
	void CheckUserRange(Int_t &nUserStartIndex, Int_t &nUserStopIndex) const; // check user supplied range indices order and against vector length
	friend Double_t FindWaveformRoot(TWaveform& UserWaveform, Double_t fTargetValue, Double_t fTimeMin, Double_t fTimeMax, Double_t fPrecision=1.0e-6, Double_t fDeltaRoot=1.0e-6, Int_t nMaxIter=1e4); // return interpolated timing position for given amplitude value
	void Init(); // initialise waveform object
	void Interpolate(); // perform interpolation
protected:
	Bool_t kIsInterpolated; // interpolation flag
	Bool_t kIsNegativeSignal; // flag for negative signal type
	Double_t fBaselineOffset;
	Double_t fTimingPrecision; // timing precision factor used in root-finding algorithm
public:
	TWaveform( std::vector<Double_t> fUserAmplitudes, std::vector<Double_t> fUserTimestamps); // standard constructor using vectors
	TWaveform( Double_t *fUserAmplitudes=NULL, Double_t *fUserTimestamps=NULL, Int_t nUserSampleLength=-1); // standard constructor using C-style arrays
	~TWaveform(); // destructor
	TWaveform(const TWaveform& UserWaveform); // copy constructor
	//template<TWaveform > TWaveform ApplyFilter(TWaveform (*myDigFilterFcn)(std::vector<Double_t>, std::vector<Double_t>, std::vector<Double_t>), std::vector<Double_t> fUserDigFiltFcnParam);
	TWaveform Add(TWaveform UserAddend); // add two waveforms
	TGraph Draw();
	Double_t Evaluate(Double_t fUserDatum); // evaluate waveform amplitude at given point in time (does not need to be a timestamp!)
	void Export(string cUserFilename) const; // write waveform data to file as csv table
	std::vector<Double_t> GetAmplitudes(){ return fAmplitudes; };
	Double_t GetArea(){ return (GetArea(0,fTimestamps.size()-1)); };
	Double_t GetArea(Int_t nUserStartIndex, Int_t nUserStopIndex);
	Double_t GetMaxAmplitude(){ return (*max_element(fAmplitudes.begin(),fAmplitudes.end())); };
	Int_t GetMaxAmplitudeIndex(){ return(distance(fAmplitudes.begin(),max_element(fAmplitudes.begin(),fAmplitudes.end()))); };
	Double_t GetMean() const { return (GetMean(0,fTimestamps.size()-1)); };
	Double_t GetMean(Int_t nUserStartIndex, Int_t nUserStopIndex) const;
	Double_t GetMinAmplitude(){ return (*min_element(fAmplitudes.begin(),fAmplitudes.end())); };
	Int_t GetMinAmplitudeIndex(){ return(distance(fAmplitudes.begin(),min_element(fAmplitudes.begin(),fAmplitudes.end()))); };
	Int_t GetN(){return fTimestamps.size(); }; // get number of entries
	Double_t GetNegFallTime(Double_t fUserLevelLow=0.1, Double_t fUserLevelHigh=0.9);
	Double_t GetNegWidth(Int_t nUserStartIndex, Int_t nUserStopIndex, Double_t fUserLevel=0.5, Bool_t bIsAbsolute=kFALSE);
	Double_t GetNegWidth(Double_t fUserLevel=0.5, Bool_t bIsAbsolute=kFALSE) { return(GetNegWidth(0,fTimestamps.size()-1,fUserLevel,bIsAbsolute)); }; // get negative width of signal
	Double_t GetPosRiseTime(Double_t fUserLevelLow=0.1, Double_t fUserLevelHigh=0.9);
	Double_t GetPosWidth(Int_t nUserStartIndex, Int_t nUserStopIndex, Double_t fUserLevel=0.5); // get width of positive signal
	Double_t GetPosWidth(Double_t fUserLevel=0.5) { return(GetPosWidth(0,fTimestamps.size()-1,fUserLevel)); };
	Double_t GetRMS() const { return (GetRMS(0,fAmplitudes.size()-1)); };
	Double_t GetRMS(Int_t nUserStartIndex, Int_t nUserStopIndex) const;
	Int_t GetTimestampIndex(Double_t fUserDate);
	std::vector<Double_t> GetTimestamps() const { return fTimestamps; };
	void Invert(); // invert waveform
	TWaveform MovingAverageFilter(Int_t nUserWindowSize=1);
	TWaveform& operator=(const TWaveform& UserWaveform); // copy assignment
	void Scale(Double_t fUserScaleFactor=1.0); // scale amplitude values
	void ScaleTimestamps(Double_t fUserScaleFactor=1.0); // scale timestamps, e.g. 1.0e09 sets timebase to nanoseconds
	void SetTimingPrecision(Double_t fUserPrecision) { fTimingPrecision=fabs(fUserPrecision); }; //  set timing precision factor
	void ShiftBaseline(Double_t fUserOffset=0.0); // subtract common offset
	void ShiftTimestamps(Double_t fUserDelay=0.0); // shift timestamps
	/* some magic ROOT stuff... */
	ClassDef(TWaveform,1);
};

#endif