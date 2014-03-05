#include <algorithm>
#include "TProfile.h"

void WaveformAnalysisExample(string cUserFileName, string cUserSignalType="-"){
	gROOT->ProcessLine(".x BuildFastFrameLibrary.cpp");
	// get data
	TFastFrame DataSet(cUserFileName);
	cout << DataSet.GetFrameCount() << " frames in data set" << endl;
	// define analysis result storage
	std::vector<Double_t> fSigWidths; // vector for signal widths
	fSigWidths.reserve(DataSet.GetFrameCount());
	std::vector<Double_t> fSigAmplitudes; // vector for signal amplitudes
	fSigAmplitudes.reserve(DataSet.GetFrameCount());
	std::vector<Double_t> fSigWidthsFiltered; // vector for filtered signal widths
	fSigWidthsFiltered.reserve(DataSet.GetFrameCount());
	std::vector<Double_t> fSigAmplitudesFiltered; // vector for filtered signal amplitudes
	fSigAmplitudesFiltered.reserve(DataSet.GetFrameCount());
	// define analysis parameters
	const Double_t fWidthLevel = 0.5;
	for(Int_t nIndex=0; nIndex<DataSet.GetFrameCount(); nIndex++){ // begin of loop over all recorded frames
		TWaveform CurrentFrame = DataSet.GetWaveform(nIndex);
		if(CurrentFrame.IsZombie())
			continue;
		//CurrentFrame.ScaleTimestamps(1.0e9); // change from s to ns
		CurrentFrame.ShiftBaseline(CurrentFrame.GetMean(0,50)); // adjust baseline based on the first 50 samples
		TWaveform FilteredFrame = CurrentFrame.MovingAverageFilter(10); // use moving average filter to remove noise, width of moving window is set to 10 samples
		if(cUserSignalType=="-"){
			fSigAmplitudes.push_back(CurrentFrame.GetMinAmplitude()); // get negative amplitude
			fSigWidths.push_back(CurrentFrame.GetNegWidth(fWidthLevel)); // get negative width of signal
			fSigAmplitudesFiltered.push_back(FilteredFrame.GetMinAmplitude()); // get negative amplitude of filtered signal
			fSigWidthsFiltered.push_back(FilteredFrame.GetNegWidth(fWidthLevel)); // get negative width of filtered signal
		}
		else if(cUserSignalType=="+"){
			fSigAmplitudes.push_back(CurrentFrame.GetMaxAmplitude()); // get negative amplitude
			fSigWidths.push_back(CurrentFrame.GetPosWidth(fWidthLevel)); // get negative width of signal
			fSigAmplitudesFiltered.push_back(FilteredFrame.GetMaxAmplitude()); // get positive amplitude of filtered signal
			fSigWidthsFiltered.push_back(FilteredFrame.GetPosWidth(fWidthLevel)); // get positive width of filtered signal
		}
		else
			exit;
	} //  end of loop over all recorded frames

	// define output histograms and graphs
	// first, raw signal
	Double_t fRangeAmplitudes = fabs(*std::min_element(fSigAmplitudes.begin(),fSigAmplitudes.end()) - *std::max_element(fSigAmplitudes.begin(),fSigAmplitudes.end()));
	TH1D hAmplitudes("hAmplitudes","Negative Amplitude Distribution; amplitude (V); frequency",30,*std::min_element(fSigAmplitudes.begin(),fSigAmplitudes.end())-0.1*fRangeAmplitudes,*std::max_element(fSigAmplitudes.begin(),fSigAmplitudes.end())+0.1*fRangeAmplitudes);
	hAmplitudes.FillN(fSigAmplitudes.size(),&fSigAmplitudes[0],NULL);

	Double_t fRangeWidths = *std::max_element(fSigWidths.begin(),fSigWidths.end()) - *std::min_element(fSigWidths.begin(),fSigWidths.end());
	TH1D hWidths("hWidths","Negative Width Distribution; width (s); frequency",30,*std::min_element(fSigWidths.begin(),fSigWidths.end())-0.1*fRangeWidths,*std::max_element(fSigWidths.begin(),fSigWidths.end())+0.1*fRangeWidths);
	hWidths.FillN(fSigWidths.size(),&fSigWidths[0],NULL);

	TGraph grCorrelation(fSigAmplitudes.size(),&fSigAmplitudes[0],&fSigWidths[0]);
	grCorrelation.SetTitle("Signal Amplitude and Width Correlation; amplitude (V); width (s)");
	grCorrelation.SetMarkerStyle(24);
	
	TProfile hCorrelation("hCorrelation","Correlation of signal amplitude and width; amplitude (V); width (s)",20,*std::min_element(fSigAmplitudes.begin(),fSigAmplitudes.end())-0.1*fRangeAmplitudes,*std::max_element(fSigAmplitudes.begin(),fSigAmplitudes.end())+0.1*fRangeAmplitudes,*std::min_element(fSigWidths.begin(),fSigWidths.end())-0.1*fRangeWidths,*std::max_element(fSigWidths.begin(),fSigWidths.end())+0.1*fRangeWidths);
	hCorrelation.FillN(fSigAmplitudes.size(),&fSigAmplitudes[0],&fSigWidths[0],NULL,1);

	TCanvas *canResults = new TCanvas("canResults","Results of Raw Waveform Analysis");
	canResults->Divide(2,2);
	canResults->cd(1);
	hAmplitudes.DrawCopy();
	canResults->cd(2);
	hWidths.DrawCopy();
	canResults->cd(3);
	grCorrelation.DrawClone("AP");
	canResults->cd(4);
	hCorrelation.DrawCopy();

	// now filtered signal
	Double_t fRangeAmplitudesFiltered = fabs(*std::min_element(fSigAmplitudesFiltered.begin(),fSigAmplitudesFiltered.end()) - *std::max_element(fSigAmplitudesFiltered.begin(),fSigAmplitudesFiltered.end()));
	TH1D hAmplitudesFiltered("hAmplitudesFiltered","Filtered Signal Amplitude Distribution; amplitude (V); frequency",30,*std::min_element(fSigAmplitudesFiltered.begin(),fSigAmplitudesFiltered.end())-0.1*fRangeAmplitudesFiltered,*std::max_element(fSigAmplitudesFiltered.begin(),fSigAmplitudesFiltered.end())+0.1*fRangeAmplitudesFiltered);
	hAmplitudesFiltered.FillN(fSigAmplitudesFiltered.size(),&fSigAmplitudesFiltered[0],NULL);

	Double_t fRangeWidthsFiltered = *std::max_element(fSigWidthsFiltered.begin(),fSigWidthsFiltered.end()) - *std::min_element(fSigWidthsFiltered.begin(),fSigWidthsFiltered.end());
	TH1D hWidthsFiltered("hWidthsFiltered","Filtered Signal Width Distribution; width (s); frequency",30,*std::min_element(fSigWidthsFiltered.begin(),fSigWidthsFiltered.end())-0.1*fRangeWidthsFiltered,*std::max_element(fSigWidthsFiltered.begin(),fSigWidthsFiltered.end())+0.1*fRangeWidthsFiltered);
	hWidthsFiltered.FillN(fSigWidthsFiltered.size(),&fSigWidthsFiltered[0],NULL);

	TGraph grCorrelationFiltered(fSigAmplitudesFiltered.size(),&fSigAmplitudesFiltered[0],&fSigWidthsFiltered[0]);
	grCorrelationFiltered.SetTitle("Filtered Signal Amplitude and Width Correlation; amplitude (V); width (s)");
	grCorrelationFiltered.SetMarkerStyle(24);
	
	TProfile hCorrelationFiltered("hCorrelationFiltered","Correlation of Filtered Signal Amplitude and Width; amplitude (V); width (s)",20,*std::min_element(fSigAmplitudesFiltered.begin(),fSigAmplitudesFiltered.end())-0.1*fRangeAmplitudesFiltered,*std::max_element(fSigAmplitudesFiltered.begin(),fSigAmplitudesFiltered.end())+0.1*fRangeAmplitudesFiltered,*std::min_element(fSigWidthsFiltered.begin(),fSigWidthsFiltered.end())-0.1*fRangeWidthsFiltered,*std::max_element(fSigWidthsFiltered.begin(),fSigWidthsFiltered.end())+0.1*fRangeWidthsFiltered);
	hCorrelationFiltered.FillN(fSigAmplitudesFiltered.size(),&fSigAmplitudesFiltered[0],&fSigWidthsFiltered[0],NULL,1);

	TCanvas *canResultsFiltered = new TCanvas("canResultsFiltered","Results of Filtered Waveform Analysis");
	canResultsFiltered->Divide(2,2);
	canResultsFiltered->cd(1);
	hAmplitudesFiltered.DrawCopy();
	canResultsFiltered->cd(2);
	hWidthsFiltered.DrawCopy();
	canResultsFiltered->cd(3);
	grCorrelationFiltered.DrawClone("AP");
	canResultsFiltered->cd(4);
	hCorrelationFiltered.DrawCopy();
}

void WaveformCorrelationAnalysisExample(string cUserFileNameNeg, string cUserFileNamePos){
	gROOT->ProcessLine(".x BuildFastFrameLibrary.cpp"); // build and load required libraries
	// get data
	TFastFrame DataSetNeg(cUserFileNameNeg); // open data file with negative waveform
	TFastFrame DataSetPos(cUserFileNamePos); // open data file with positive waveform
	if(DataSetNeg.IsZombie() || DataSetPos.IsZombie()) // exit ROOT if any of the two files is not loaded properly
		exit;
	if(DataSetNeg.GetFrameCount()!=DataSetPos.GetFrameCount()) // exit ROOT if the number of frames of the two datasets do not match
		exit;
	cout << DataSetNeg.GetFrameCount() << " frames in data set" << endl;
	// define analysis result storage
	std::vector<Double_t> fNegSigWidths; // vector for negative signal widths
	fNegSigWidths.reserve(DataSetNeg.GetFrameCount());
	std::vector<Double_t> fPosSigWidths; // vector for positive signal widths
	fPosSigWidths.reserve(DataSetPos.GetFrameCount());
	std::vector<Double_t> fNegSigAmplitudes; // vector for negative signal amplitudes
	fNegSigAmplitudes.reserve(DataSetNeg.GetFrameCount());
	std::vector<Double_t> fPosSigAmplitudes; // vector for positive signal amplitudes
	fPosSigAmplitudes.reserve(DataSetPos.GetFrameCount());
	// define analysis parameters
	const Double_t fWidthLevelNeg = 0.5; // level for width analysis of negative signal
	const Double_t fWidthLevelPos = 0.5; // level for width analysis of positive signal
	for(Int_t nIndex=0; nIndex<DataSetNeg.GetFrameCount(); nIndex++){ // begin of loop over all recorded frames
		TWaveform CurrentFrameNeg = DataSetNeg.GetWaveform(nIndex); // get negative waveform
		TWaveform CurrentFramePos = DataSetPos.GetWaveform(nIndex); // get positive waveform
		if(CurrentFrameNeg.IsZombie() || CurrentFramePos.IsZombie())
			continue;
		CurrentFrameNeg.ShiftBaseline(CurrentFrameNeg.GetMean(0,50)); // adjust baseline of negative sample based on the first 50 samples
		fNegSigAmplitudes.push_back(CurrentFrameNeg.GetMinAmplitude()); // get negative amplitude
		fNegSigWidths.push_back(CurrentFrameNeg.GetNegWidth(fWidthLevelNeg)); // get negative width of signal
		CurrentFramePos.ShiftBaseline(CurrentFramePos.GetMean(0,50)); // adjust baseline of positive sample based on the first 50 samples
		fPosSigAmplitudes.push_back(CurrentFramePos.GetMaxAmplitude()); // get positive amplitude
		fPosSigWidths.push_back(CurrentFramePos.GetPosWidth(fWidthLevelPos)); // get positive width of signal
	} //  end of loop over all recorded frames

	// define output histograms and graphs
	// first, negative signals
	Double_t fRangeNegAmplitudes = fabs(*std::min_element(fNegSigAmplitudes.begin(),fNegSigAmplitudes.end()) - *std::max_element(fNegSigAmplitudes.begin(),fNegSigAmplitudes.end()));
	TH1D hNegAmplitudes("hNegAmplitudes","Negative Amplitude Distribution; amplitude (V); frequency",30,*std::min_element(fNegSigAmplitudes.begin(),fNegSigAmplitudes.end())-0.1*fRangeNegAmplitudes,*std::max_element(fNegSigAmplitudes.begin(),fNegSigAmplitudes.end())+0.1*fRangeNegAmplitudes);
	hNegAmplitudes.FillN(fNegSigAmplitudes.size(),&fNegSigAmplitudes[0],NULL);

	Double_t fRangeNegWidths = *std::max_element(fNegSigWidths.begin(),fNegSigWidths.end()) - *std::min_element(fNegSigWidths.begin(),fNegSigWidths.end());
	TH1D hNegWidths("hNegWidths","Negative Width Distribution; width (s); frequency",30,*std::min_element(fNegSigWidths.begin(),fNegSigWidths.end())-0.1*fRangeNegWidths,*std::max_element(fNegSigWidths.begin(),fNegSigWidths.end())+0.1*fRangeNegWidths);
	hNegWidths.FillN(fNegSigWidths.size(),&fNegSigWidths[0],NULL);

	TGraph grNegSignalCorrelation(fNegSigAmplitudes.size(),&fNegSigAmplitudes[0],&fNegSigWidths[0]);
	grNegSignalCorrelation.SetTitle("Negative Signal Amplitude and Width Correlation; amplitude (V); width (s)");
	grNegSignalCorrelation.SetMarkerStyle(24);
	
	TProfile hNegSignalCorrelation("hNegSignalCorrelation","Correlation of Negative Signal Amplitude and Width; amplitude (V); width (s)",20,*std::min_element(fNegSigAmplitudes.begin(),fNegSigAmplitudes.end())-0.1*fRangeNegAmplitudes,*std::max_element(fNegSigAmplitudes.begin(),fNegSigAmplitudes.end())+0.1*fRangeNegAmplitudes,*std::min_element(fNegSigWidths.begin(),fNegSigWidths.end())-0.1*fRangeNegWidths,*std::max_element(fNegSigWidths.begin(),fNegSigWidths.end())+0.1*fRangeNegWidths);
	hNegSignalCorrelation.FillN(fNegSigAmplitudes.size(),&fNegSigAmplitudes[0],&fNegSigWidths[0],NULL,1);

	TCanvas *canNegSignalResults = new TCanvas("canNegSignalResults","Results of Negative Signal Waveform Analysis");
	canNegSignalResults->Divide(2,2);
	canNegSignalResults->cd(1);
	hNegAmplitudes.DrawCopy();
	canNegSignalResults->cd(2);
	hNegWidths.DrawCopy();
	canNegSignalResults->cd(3);
	grNegSignalCorrelation.DrawClone("AP");
	canNegSignalResults->cd(4);
	hNegSignalCorrelation.DrawCopy();

	// then positive signals (should be NINO logic output)
	Double_t fRangePosAmplitudes = fabs(*std::min_element(fPosSigAmplitudes.begin(),fPosSigAmplitudes.end()) - *std::max_element(fPosSigAmplitudes.begin(),fPosSigAmplitudes.end()));
	TH1D hPosAmplitudes("hPosAmplitudes","Positive Amplitude Distribution; amplitude (V); frequency",30,*std::min_element(fPosSigAmplitudes.begin(),fPosSigAmplitudes.end())-0.1*fRangePosAmplitudes,*std::max_element(fPosSigAmplitudes.begin(),fPosSigAmplitudes.end())+0.1*fRangePosAmplitudes);
	hPosAmplitudes.FillN(fPosSigAmplitudes.size(),&fPosSigAmplitudes[0],NULL);

	Double_t fRangePosWidths = *std::max_element(fPosSigWidths.begin(),fPosSigWidths.end()) - *std::min_element(fPosSigWidths.begin(),fPosSigWidths.end());
	TH1D hPosWidths("hPosWidths","Positive Width Distribution; width (s); frequency",30,*std::min_element(fPosSigWidths.begin(),fPosSigWidths.end())-0.1*fRangePosWidths,*std::max_element(fPosSigWidths.begin(),fPosSigWidths.end())+0.1*fRangePosWidths);
	hPosWidths.FillN(fPosSigWidths.size(),&fPosSigWidths[0],NULL);

	TGraph grPosSignalCorrelation(fPosSigAmplitudes.size(),&fPosSigAmplitudes[0],&fPosSigWidths[0]);
	grPosSignalCorrelation.SetTitle("Positive Signal Amplitude and Width Correlation; amplitude (V); width (s)");
	grPosSignalCorrelation.SetMarkerStyle(24);
	
	TProfile hPosSignalCorrelation("hPosSignalCorrelation","Correlation of Positive Signal Amplitude and Width; amplitude (V); width (s)",20,*std::min_element(fPosSigAmplitudes.begin(),fPosSigAmplitudes.end())-0.1*fRangePosAmplitudes,*std::max_element(fPosSigAmplitudes.begin(),fPosSigAmplitudes.end())+0.1*fRangePosAmplitudes,*std::min_element(fPosSigWidths.begin(),fPosSigWidths.end())-0.1*fRangePosWidths,*std::max_element(fPosSigWidths.begin(),fPosSigWidths.end())+0.1*fRangePosWidths);
	hPosSignalCorrelation.FillN(fPosSigAmplitudes.size(),&fPosSigAmplitudes[0],&fPosSigWidths[0],NULL,1);

	TCanvas *canPosSignalResults = new TCanvas("canPosSignalResults","Results of Positive Signal Waveform Analysis");
	canPosSignalResults->Divide(2,2);
	canPosSignalResults->cd(1);
	hPosAmplitudes.DrawCopy();
	canPosSignalResults->cd(2);
	hPosWidths.DrawCopy();
	canPosSignalResults->cd(3);
	grPosSignalCorrelation.DrawClone("AP");
	canPosSignalResults->cd(4);
	hPosSignalCorrelation.DrawCopy();

	// and finally the correlation between negative amplitude and positive width
	if(fNegSigAmplitudes.size()!=fPosSigWidths.size()) // skip correlation plots if vector sizes don't match
		return;
	TGraph grSignalCorrelation(fNegSigAmplitudes.size(),&fNegSigAmplitudes[0],&fPosSigWidths[0]);
	grSignalCorrelation.SetTitle("Negative Signal Amplitude and Positive Signal Width Correlation; amplitude (V); width (s)");
	grSignalCorrelation.SetMarkerStyle(24);

	TProfile hSignalCorrelation("hSignalCorrelation","Correlation of Negative Signal Amplitude and Positive Signal Width; amplitude (V); width (s)",20,*std::min_element(fNegSigAmplitudes.begin(),fNegSigAmplitudes.end())-0.1*fRangeNegAmplitudes,*std::max_element(fNegSigAmplitudes.begin(),fNegSigAmplitudes.end())+0.1*fRangeNegAmplitudes,*std::min_element(fPosSigWidths.begin(),fPosSigWidths.end())-0.1*fRangePosWidths,*std::max_element(fPosSigWidths.begin(),fPosSigWidths.end())+0.1*fRangePosWidths);
	hSignalCorrelation.FillN(fNegSigAmplitudes.size(),&fNegSigAmplitudes[0],&fPosSigWidths[0],NULL,1);

	TCanvas *canCorrelationResults = new TCanvas("canCorrelationResults","Results of Signal Correlation Analysis");
	canCorrelationResults->Divide(1,2);
	canCorrelationResults->cd(1);
	grSignalCorrelation.DrawClone("AP");
	canCorrelationResults->cd(2);
	hSignalCorrelation.DrawCopy();
}