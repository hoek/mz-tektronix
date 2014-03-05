#include <algorithm>
#include <iterator>
#include "TGraph.h"
#include "TWaveform.h"

struct CFD_OUTPUT{
	Double_t fTimePickOff;
	Double_t fIntervalSize;
	TGraph grDelayedSignal;
	TGraph grInvertedSignal;
	TGraph grCfdSumSignal;
};


Double_t BisectionMethod(TWaveform UserWaveform, Double_t fMin, Double_t fMax, Double_t fPrecision=1.0e-6, Double_t fDeltaRoot=1.0e-6, Int_t nMaxIter=1e4){
	if(UserWaveform.IsZombie()){
		return (-1.0);
	}
	Double_t fFcnLeft = UserWaveform.Evaluate(fMin);
	Double_t fIntervalLength = fMax - fMin;
	Double_t fIntervalMidPoint;

	for(Int_t i=0; i<nMaxIter; i++){
		fIntervalLength *= 0.5; // shrink interval by half
		fIntervalMidPoint = fMin + fIntervalLength; // update interval middle point
		Double_t fFcnMidPoint = UserWaveform.Evaluate(fIntervalMidPoint); // evaluate waveform at new interval midpoint
		if( fabs(fIntervalLength)<fDeltaRoot || fabs(fFcnMidPoint)<fPrecision ){ 
			//cout << i << ", " << fFcnMidPoint << ", " << fIntervalLength << ", " << fIntervalMidPoint << endl;
			return (fIntervalMidPoint);
		}
		( (fFcnLeft>0.0 && fFcnMidPoint<0.0) || (fFcnLeft<0.0 && fFcnMidPoint>0.0) ) ? (fMax=fIntervalMidPoint) : (fMin=fIntervalMidPoint, fFcnLeft=fFcnMidPoint);
	}
	return (fIntervalMidPoint);
}


Double_t LeadingEdgeDiscriminator(TWaveform UserWaveform, Double_t fUserThreshold=0.0){
	Double_t fTiming = -9999.0;
	if(fUserThreshold>UserWaveform.GetMinAmplitude()){ // check if trigger condition is true
		UserWaveform.ShiftBaseline(fUserThreshold);
		std::vector<Double_t> fTempAmpl = UserWaveform.GetAmplitudes();
		std::vector<Double_t> fTempTimestamps = UserWaveform.GetTimestamps();
		for(Int_t i=distance(fTempAmpl.begin(),min_element(fTempAmpl.begin(),fTempAmpl.end())); i>-1; i--){
			if((fTempAmpl.at(i))> 0.0){ // only falling slope, negative signals
				Double_t fMin = fTempTimestamps.at(i);
				Double_t fMax = fTempTimestamps.at(i+1);
				//fTiming = BisectionMethod(UserWaveform,fMin,fMax,1.0e-08,1.0e-11,1e4);
				fTiming = FindWaveformRoot(UserWaveform,0.0,fMin,fMax,1.0e-08,1.0e-11,1e4);
				break; 
			}
		}
	}
	return (fTiming);
}

Double_t ConstantFractionDiscriminator(TWaveform UserWaveform, Double_t fUserThreshold = 0.0, Double_t fUserDelay=0.0, Double_t fUserFraction=0.3){
	// +++ create inverted & attenuated signal +++
	TWaveform Fraction = UserWaveform;
	Fraction.Invert();
	Fraction.Scale(fUserFraction);
	Fraction.ShiftTimestamps(-fabs(fUserDelay));
	// +++ add both waveforms +++
	TWaveform CfdSum = Fraction.Add(UserWaveform);
	if(CfdSum.IsZombie()) exit(1);
	// +++ arm discriminator +++
	if(fUserThreshold>UserWaveform.GetMaxAmplitude() || fUserThreshold<UserWaveform.GetMinAmplitude()){
		//cout << "Threshold out of range!" << endl;
		//CfdResults.fTimePickOff = -9999.0;
		//return (CfdResults);
		return (-9999.0);
	}
	// +++ find zero crossing +++
	Double_t fCfdRoot = 0.0;
	std::vector<Double_t> fTemp = CfdSum.GetAmplitudes();
	for(Int_t i=distance(fTemp.begin(),max_element(fTemp.begin(),fTemp.end())); i<fTemp.size(); i++){
		if(fTemp.at(i)<0.0){
			// root-finding algorithm goes here...
			Double_t fMin = CfdSum.GetTimestamps().at(i-1);
			Double_t fMax = CfdSum.GetTimestamps().at(i);
			fCfdRoot = BisectionMethod(CfdSum,fMin,fMax,1.0e-08,1.0e-11,1e4);
			break;
		}
	}
	return (fCfdRoot);
}


//#include <iostream>
//
//template<double F(double)>
//double trapezoidal(double a, double b, int n){
//	double h = (b-a)/n;
//	double sum = F(a)*0.5;
//	for(int i = 1; i < n; i++) sum += F(a+i*h);
//	sum += F(b)*0.5;
//	return sum*h;
//}
//
//double myintegrand(double x){
//	return 1.0;
//}
//
//int myMain(){
//	cout << trapezoidal<myintegrand>(0,5,100) << endl;
//	return (0);
//}
