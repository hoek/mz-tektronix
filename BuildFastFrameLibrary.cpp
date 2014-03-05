void BuildFastFrameLibrary(){
	gROOT->ProcessLine(".L myUtilities.cpp+");
	gROOT->ProcessLine(".L myFastFrameConverter.cpp+");
	gROOT->ProcessLine(".L TFastFrame.cpp+");
	gROOT->ProcessLine(".L TWaveform.cpp+");
	gROOT->ProcessLine(".L DigitalFiltersExample.cpp+");
}