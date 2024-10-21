// STL
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

// ROOT
#include <TROOT.h>
#include <TStopwatch.h>
#include <TFile.h>

// HESS
#include <utilities/TNamedSaver.hh>
#include <sash/HESSArray.hh>
#include <sash/DataSet.hh>
#include <sash/TreeWriter.hh>
#include <sash/MakerChain.hh>

#include <sashfile/HandlerC.hh>

#include <sashastro/NominalPointingMaker.hh>

#include <pointing/CorrectionMaker.hh>

#include <plotters/PlotMaker.hh>

#include <dbutils/Handler.hh>
#include <dbutils/StandardMuonReader.hh>
#include <dbutils/RateReader.hh>

#include <intensity/IntensityMaker.hh>
#include <intensity/InPaintingMaker.hh>
#include <intensity/NSBCleaner.hh>
#include <intensity/StatsMaker2.hh>
//#include <intensity/TimingCleaner.hh>
#include <intensity/NSBTimingCleaner.hh>
#include <hapreco/HillasParameterMakerC.hh>
#include <hapreco/NewMonoImageParameters.hh>
#include <hapreco/NewMonoImageParameterMaker.hh>

#include <fromdb/CameraConfigurator.hh>
#include <fromdb/HeaderFixer.hh>

#include <calibrationmakers/BrokenPixelMerger2.hh>
#include <sash/Folder.hh>
#include <sashfile/Utils.hh>

#ifndef __CINT__
#include <summary/HeaderChecker.hh>
#include <dbtools/simpletable_frame.hh>
#include <dbtools/MySQLconnection.hh>
#endif


void CleanDSTs(int nrun, std::string outfilename,int firstevent = -1,int lastevent = -1); // Real Run
void CleanDSTs(std::string filename,std::string outfilename ,int firstevent = -1,int lastevent = -1); // MC
void InitChain(Sash::MakerChain *fChain, Sash::TreeWriter *fWriter, bool isMC);
bool InitHandler(SashFile::HandlerC *fHan, int nrun);
std::string GetEventFolderName() { return "^events$"; }


simpletable::MySQLconnection *fConnection = 0;


void CleanDSTs(int nrun,std::string outfilename,int firstevent ,int lastevent) {

  //int firstevent = 10000;
  //int lastevent = -1;
  //int lastevent = 5;

  SashFile::HandlerC han("events");
  if (!InitHandler(&han,nrun)) {
    return;
  }

  Sash::DataSet *ds_events = han.GetPrimaryDataSet();
  if (!ds_events) {
    return;
  }

  // Create DB connection :
  // simpletable::MyDBDefaults::set_config("test");
  // simpletable::MySQLconnection *fConnection =  new simpletable::MySQLconnection();
  fConnection = new simpletable::MySQLconnection("test");


  // Create output file name :
  //std::ostringstream oss_outfilename;
  //oss_outfilename << "run_" << nrun << "_DST_002.root"; // to have the same na
  //TFile *outfile = new TFile(oss_outfilename.str().c_str(),"RECREATE");
  // Create output file name :
  TFile *outfile = new TFile(outfilename.c_str(),"RECREATE");
  gROOT->cd();

  Sash::TreeWriter *fWriter = new Sash::TreeWriter("^run$","TimingTest");
  fWriter->SetOutDir(outfile);

  Sash::MakerChain fChain("Calibration Chain",true,true);
  InitChain(&fChain,fWriter,false);
  std::cout << "InitChain done" << std::endl;
  TStopwatch glob_watch;
  glob_watch.Start();
  int nread = ds_events->EventLoop(&fChain,firstevent,lastevent,true,true); // last true : send endrun and endanalysis
  int neventstot = (int)ds_events->GetEntries();

  glob_watch.Stop();
  std::cout << "Time to make DST: " << glob_watch.RealTime() << " s (" << glob_watch.CpuTime() << " s)" << std::endl;

  fChain.Finish(Sash::Folder::GetFolder("Finish"));
  delete outfile;
  outfile=0;
  fWriter->SetOutDir(outfile);


}

void CleanDSTs(std::string filename, std::string outfilename,int firstevent,int lastevent) {

  //int firstevent = -1;
  //int lastevent = -1;

  SashFile::HandlerC han("events");
  if (!han.ConnectFileByName(filename)) {
    std::cout << "Handler not working" << std::endl;
    return;
  }

  SashFile::CopyHistograms(han.GetPrimaryFile(), gROOT);

  Sash::DataSet *ds_events = han.GetPrimaryDataSet();
  if (!ds_events) {
    std::cout << "Couldn't get PrimaryDataSet" << std::endl;
    return;
  }

  // Create output file name :
  TFile *outfile = new TFile(outfilename.c_str(),"RECREATE");
  gROOT->cd();

  Sash::TreeWriter *fWriter = new Sash::TreeWriter("^run$","TimingTest");
  fWriter->SetOutDir(outfile);

  Sash::MakerChain fChain("Calibration Chain",true,true);
  std::cout << "Initiating Chain with cleaning etc." << std::endl;
  InitChain(&fChain,fWriter,true);

  TStopwatch glob_watch;
  glob_watch.Start();
  int nread = ds_events->EventLoop(&fChain,firstevent,lastevent,true,true); // last true : send endrun and endanalysis
  int neventstot = (int)ds_events->GetEntries();

  glob_watch.Stop();
  std::cout << "Time to make DST: " << glob_watch.RealTime() << " s (" << glob_watch.CpuTime() << " s)" << std::endl;


  fChain.Finish(Sash::Folder::GetFolder("Finish"));
  delete outfile;
  outfile=0;
  fWriter->SetOutDir(outfile);

  Utilities::TNamedSaver saver;

  saver.Save(outfilename.c_str(), true, true, true ); // VIM TEMPO
    // saver.Clean(true,true,true,true); // VIM TEST
    //}

  gROOT->cd();

}


bool InitHandler(SashFile::HandlerC *fHan, int nrun) {

  if ( ! fHan->ConnectCameraFile(nrun) ) {
    std::cout << "Can't open rawdata file for run [" << nrun << "]" << std::endl;
    return false;
  }

  fHan->ConnectFile("CameraMonitor");
  fHan->ConnectFile("HVBrokenPixel");
  fHan->ConnectFile("BrokenTiming");
  fHan->ConnectFile("BrokenPedestal");
  fHan->ConnectFile("BrokenARS");
  fHan->ConnectFile("Pedestal");

  return true;
}



void InitChain(Sash::MakerChain *fChain, Sash::TreeWriter *fWriter, bool is_simulated_data) {


  std::string analysisname = "DST";
  bool writeExtendedIntensityObject = true;
  int muonEntry = 100; // Not needed
  bool doCommonModeCorrection = false;

  fWriter->AddInputFolder( ("^"+analysisname+"$").c_str() );

  //======================================================================
  // DST CHAIN
  //======================================================================

  // ------------------------------------------------------------

  // ------------------------------------------------------------
  // Make plots of raw information before any processing is done

  Plotters::PlotMaker* preplotmaker =  new Plotters::PlotMaker(GetEventFolderName().c_str(), "PreCuts");
  preplotmaker->AddSystemHist("Sash::EventHeader","","NTelWData","",9,-0.25,4.25);
  preplotmaker->AddSystemHist("Sash::EventHeader","","NTelTriggered","",9,-0.25,4.25);
  fChain->UseMaker(preplotmaker);

  if ( false == is_simulated_data ) {
    std::string fromdbname = "hess";
    FromDB::HeaderFixer *fixer = new FromDB::HeaderFixer();
    fixer->SetDBConfigName(fromdbname.c_str());
    fChain->UseMaker(fixer);

    FromDB::CameraConfigurator *camconfig = new FromDB::CameraConfigurator();
    camconfig->SetDBConfigName(fromdbname.c_str());
    fChain->UseMaker(camconfig);

    // ------------------------------------------------------------
    // Set up trigger stats (RateReader) and BrokenPixel makers
    // outputs Calibration::TelescopeMuonEfficiency objects for
    // compatibility with Paris analysis:

    DBUtils::StandardMuonReader *std_muon = new DBUtils::StandardMuonReader( "^run$","MuonStats",analysisname.c_str(),muonEntry,false,true); // last true : NewHDMuon
    std_muon->SetConnectionName("test");
    fChain->UseMaker(std_muon);
    fWriter->AddInputFolder("MuonStats");

    // output Trigger rate information
    fChain->UseMaker(new DBUtils::RateReader("^run$","TriggerStats",fConnection,true));
    fWriter->AddInputFolder("TriggerStats");
  }
  else {
    // In the case of a MC DST register the MCTrueShower object in
    // the DST folder
    Sash::Folder::GetFolder(analysisname.c_str())->NextEntry(0,"Handle<Sash::MCTrueShower>","", Sash::Folder::Register);
  }


  // ------------------------------------------------------------
  // Correct for pointing errors
  fChain->UseMaker(new SashAstro::NominalPointingMaker(GetEventFolderName().c_str(),analysisname.c_str()));

  if (is_simulated_data) {
    fChain->UseMaker(new Pointing::CorrectionMaker("Pointing::NullModel", GetEventFolderName().c_str(),analysisname.c_str()));
  } else {
    fChain->UseMaker(new Pointing::CorrectionMaker("Pointing::MechanicalModel", GetEventFolderName().c_str(),analysisname.c_str(),false,true,true,true));
  }


  // Add the broken pixel merging maker
  Calibration::BrokenPixelMerger2 *bpmerger = new Calibration::BrokenPixelMerger2("BrokenPixel");
  bpmerger->AddBrokenPixel("HVBrokenPixel", "HV", Calibration::PixelBroken::HVUnstable, Calibration::PixelBroken::HVOff);
  bpmerger->AddBrokenPixel("BrokenTiming", "BadTiming", Calibration::PixelBroken::MiscHG, Calibration::PixelBroken::MiscLG);
  bpmerger->AddBrokenPixel("BrokenPedestal", "Pedestal", Calibration::PixelBroken::BadPedHi, Calibration::PixelBroken::BadPedLo);
  bpmerger->AddBrokenPixel("BrokenARS", "ARS", Calibration::PixelBroken::NoSig, Calibration::PixelBroken::MiscHG, Calibration::PixelBroken::MiscLG, Calibration::PixelBroken::ARSHG, Calibration::PixelBroken::ARSLG, Calibration::PixelBroken::BadHiLo);

  // I am not sure I want to use it as it is low level trigger stuff
  fChain->UseMaker(bpmerger);



  // ------------------------------------------------------------
  // Common-mode noise correction
  if( doCommonModeCorrection ) {
    //fChain->UseMaker( new Calibration::CommonModeCorrectionMakerHD( "CommonMode",GetEventFolderName().c_str(),"temp" ) );
  }
  std::cout << "Starting Intensity Reconstruction"<<std::endl;
  // ------------------------------------------------------------
  // Intensity reconstruction
  Intensity::IntensityMaker *newintmaker = new Intensity::IntensityMaker(GetEventFolderName().c_str(), "events", "");
  newintmaker->UseCommonModeCorrection(false); // Not working yet !
  newintmaker->SetIntensityType(Sash::TelescopeConfig::HESS1_960,     Intensity::Mode::ChargePe);
  newintmaker->SetIntensityType(Sash::TelescopeConfig::HESS1U_960,    Intensity::Mode::ChargePe);
  newintmaker->SetIntensityType(Sash::TelescopeConfig::HESS2_2048,    Intensity::Mode::ChargePe);
  newintmaker->SetIntensityType(Sash::TelescopeConfig::FlashCam_1764, Intensity::Mode::ChargePe);

  newintmaker->SetChannelType(Intensity::Mode::AllSmooth);
  newintmaker->UseBadADCtoPePixel(true);
  newintmaker->UseBadHiLoPixel(true);
  newintmaker->UseBadFFPixel(true);

  fChain->UseMaker(newintmaker);

  fChain->UseMaker( new Intensity::InPaintingMaker(GetEventFolderName(),"events","",false) );


  std::cout << "----------------------------------------------" << std::endl;

  std::cout << " Welcome to Jelena's cleaning scripts!" << std::endl;
  char cleanname0[80];
  sprintf(cleanname0, "CleanTailcutsSTD");
  char hillasname0[80];
  sprintf(hillasname0,"TailcutsSTD");

    
  std::cout << "---> TailcutsSTD" << std::endl;
  
  std::map<int, std::pair<std::pair<float, float>, int> > standard_thresholds1;
  standard_thresholds1[Sash::TelescopeConfig::HESS1_960] =
    std::make_pair(std::make_pair(5, 10), 1);
  standard_thresholds1[Sash::TelescopeConfig::HESS1U_960] =
    std::make_pair(std::make_pair(5, 10), 1);
  standard_thresholds1[Sash::TelescopeConfig::FlashCam_1764] =
    std::make_pair(std::make_pair(9, 16), 2);

    Intensity::NSBCleaner *nsbclean0 = new Intensity::NSBCleaner(standard_thresholds1, GetEventFolderName().c_str(), "temp", 3, cleanname0);// in case you want to keep the intensity object Intensity::NSBCleaner *nsbclean0 = new Intensity::NSBCleaner(standard_thresholds1, GetEventFolderName().c_str(), analysisname.c_str(), 3, cleanname0);
    std::cout << "EventFolder used: " << GetEventFolderName().c_str() << std::endl;
    nsbclean0->DoPedestalInPainting();
    nsbclean0->SetMinimumNumberOfPixelForInPainting(4);
    fChain->UseMaker(nsbclean0);

    Reco::HillasParameterMakerC* hillasmaker0 = new Reco::HillasParameterMakerC(GetEventFolderName().c_str(), analysisname.c_str(), hillasname0);
    hillasmaker0->GetIntensityName() = cleanname0;
    fChain->UseMaker(hillasmaker0);

    Reco::NewMonoImageParameterMaker *newvarmaker0 = new Reco::NewMonoImageParameterMaker( GetEventFolderName().c_str(), analysisname.c_str(), hillasname0 );
    newvarmaker0->GetHillasName() = hillasname0;
    newvarmaker0->GetIntensityName() = cleanname0;
    fChain->UseMaker( newvarmaker0 );


std::cout << "----------------------------------------------" << std::endl;
    
  std:: cout << "Extended 0407 for ImPACT" << std::endl;
  int tailcutmin2 = 4;
  int tailcutmax2 = 7;
  char cleanname2[80];
  sprintf(cleanname2, "Extended0407");

    int numberOfRowsToAddInExtendedImage = 4;

    Intensity::NSBCleaner *nsbclean2 = new Intensity::NSBCleaner( GetEventFolderName().c_str(),  analysisname.c_str(), tailcutmin2, tailcutmax2, 3, cleanname2 ,numberOfRowsToAddInExtendedImage);
    nsbclean2->DoPedestalInPainting();
    nsbclean2->SetMinimumNumberOfPixelForInPainting( 4 );
    fChain->UseMaker( nsbclean2 );

std::cout << "------------------------------------------------" << std::endl;
std::cout << "TimeCleaningPerformance3D (TIME3D_1) " << std::endl;
char fCleanNameTime1[80];
sprintf(fCleanNameTime1, "CleanTimeCleaningPerformance3D");
char fHillasNameTime1[80];
sprintf(fHillasNameTime1, "TimeCleaningPerformance3D");

double nc1 = 1;
double time1= 0.75;
double space1= 0.3;
int pixsize1=9;
double nc_hard1 = 3;

std::cout << "         --> Intensity Name" << fCleanNameTime1 << std::endl;
std::cout << "         --> noisecut: " << nc1 << "RMD_ped" << std::endl;
std::cout << "         --> Time scale: " << time1 << "ns" << std::endl;
std::cout << "         --> Spatial scale: " << space1 << "m" << std::endl;
std::cout << "         --> Clustersize: " << pixsize1 << "pix" << std::endl;
std::cout << "         --> NC hard: " << nc_hard1 << "pe" << std::endl;



std::map<int,TimeCleaningParvec> nsbtc_parameters1;
nsbtc_parameters1[Sash::TelescopeConfig::FlashCam_1764] = TimeCleaningParvec(space1,time1,pixsize1,0,nc1,nc_hard1, false);



Intensity::NSBTimingCleaner *timingclean1 = new Intensity::NSBTimingCleaner(nsbtc_parameters1,GetEventFolderName().c_str(),"temp",fCleanNameTime1);

timingclean1->DoPedestalInPainting();
timingclean1->SetMinimumNumberOfPixelForInPainting(4);
fChain->UseMaker(timingclean1);

Reco::HillasParameterMakerC* hillasmakerTime1 = new Reco::HillasParameterMakerC(GetEventFolderName().c_str(), analysisname.c_str(), fHillasNameTime1);
hillasmakerTime1->GetIntensityName() = fCleanNameTime1;
fChain->UseMaker(hillasmakerTime1);

    Reco::NewMonoImageParameterMaker *newvarmakerTime1 = new Reco::NewMonoImageParameterMaker( GetEventFolderName().c_str(), analysisname.c_str(), fHillasNameTime1 );
    newvarmakerTime1->GetHillasName() = fHillasNameTime1;
    newvarmakerTime1->GetIntensityName() = fCleanNameTime1;
    fChain->UseMaker( newvarmakerTime1 );

std::cout << "------------------------------------------------" << std::endl;
std::cout << "TimeCleaningDetection3D (TIME3D_12)" << std::endl;
char fCleanNameTime2[80];
sprintf(fCleanNameTime2, "CleanTimeCleaningDetection3D");
char fHillasNameTime2[80];
sprintf(fHillasNameTime2, "TimeCleaningDetection3D");

double nc2 = 0;
double time2 = 0.75;
double space2 = 0.3;
int pixsize2 = 7;

int nc_hard2 = 2;

std::cout << "         --> Intensity Name" << fCleanNameTime2 << std::endl;
std::cout << "         --> noisecut: " << nc2 << "RMD_ped" << std::endl;
std::cout << "         --> Time scale: " << time2 << "ns" << std::endl;
std::cout << "         --> Spatial scale: " << space2 << "m" << std::endl;
std::cout << "         --> Clustersize: " << pixsize2 << "pix" << std::endl;
std::cout << "         --> NC hard: " << nc_hard2 << "pe" << std::endl;



std::map<int,TimeCleaningParvec> nsbtc_parameters2;
nsbtc_parameters2[Sash::TelescopeConfig::FlashCam_1764] = TimeCleaningParvec(space2,time2,pixsize2,0,nc2,nc_hard2, false);



Intensity::NSBTimingCleaner *timingclean2 = new Intensity::NSBTimingCleaner(nsbtc_parameters2,GetEventFolderName().c_str(),"temp",fCleanNameTime2);

timingclean2->DoPedestalInPainting();
timingclean2->SetMinimumNumberOfPixelForInPainting(4);
fChain->UseMaker(timingclean2);

Reco::HillasParameterMakerC* hillasmakerTime2 = new Reco::HillasParameterMakerC(GetEventFolderName().c_str(), analysisname.c_str(), fHillasNameTime2);
hillasmakerTime2->GetIntensityName() = fCleanNameTime2;
fChain->UseMaker(hillasmakerTime2);

    Reco::NewMonoImageParameterMaker *newvarmakerTime2 = new Reco::NewMonoImageParameterMaker( GetEventFolderName().c_str(), analysisname.c_str(), fHillasNameTime2 );
    newvarmakerTime2->GetHillasName() = fHillasNameTime2;
    newvarmakerTime2->GetIntensityName() = fCleanNameTime2;
    fChain->UseMaker( newvarmakerTime2 );



std::cout << "------------------------------------------------" << std::endl;
std::cout << "TimeCleaningDetection2 (TIME4D_8)" << std::endl;
char fCleanNameTime3[80];
sprintf(fCleanNameTime3, "CleanTimeCleaningDetection4D");
char fHillasNameTime3[80];
sprintf(fHillasNameTime3, "TimeCleaningDetection4D");

double nc3 = 7.2;
double time3 = 4.16;
double space3 = 0.1;
int pixsize3 = 3;
double logsize_frac_scale3 = -2.67;
int nc_hard3 = 3;

std::cout << "         --> Intensity Name" << fCleanNameTime3 << std::endl;
std::cout << "         --> noisecut: " << nc3 << "RMD_ped" << std::endl;
std::cout << "         --> Time scale: " << time3 << "ns" << std::endl;
std::cout << "         --> Spatial scale: " << space3 << "m" << std::endl;
std::cout << "         --> Log Size fraction scale: " << logsize_frac_scale3 <<std::endl;
std::cout << "         --> Clustersize: " << pixsize3 << "pix" << std::endl;
std::cout << "         --> NC hard: " << nc_hard3 << "pe" << std::endl;



std::map<int,TimeCleaningParvec> nsbtc_parameters3;
nsbtc_parameters3[Sash::TelescopeConfig::FlashCam_1764] = TimeCleaningParvec(space3,time3,pixsize3,logsize_frac_scale3,nc3,nc_hard3, true);



Intensity::NSBTimingCleaner *timingclean3 = new Intensity::NSBTimingCleaner(nsbtc_parameters3,GetEventFolderName().c_str(),"temp",fCleanNameTime3);

timingclean3->DoPedestalInPainting();
timingclean3->SetMinimumNumberOfPixelForInPainting(4);
fChain->UseMaker(timingclean3);

Reco::HillasParameterMakerC* hillasmakerTime3 = new Reco::HillasParameterMakerC(GetEventFolderName().c_str(), analysisname.c_str(), fHillasNameTime3);
hillasmakerTime3->GetIntensityName() = fCleanNameTime3;
fChain->UseMaker(hillasmakerTime3);

    Reco::NewMonoImageParameterMaker *newvarmakerTime3 = new Reco::NewMonoImageParameterMaker( GetEventFolderName().c_str(), analysisname.c_str(), fHillasNameTime3 );
    newvarmakerTime3->GetHillasName() = fHillasNameTime3;
    newvarmakerTime3->GetIntensityName() = fCleanNameTime3;
    fChain->UseMaker( newvarmakerTime3 );

  if (is_simulated_data == false) {

    // VIM : Code duplication : I don't like it
    //simpletable::MySQLconnection* statconn = 0;
    //if (fDBFlag) statconn = fConnection;

    //Summary::StatsMaker* smaker = new Summary::StatsMaker(analysisname.c_str(), "Reco::HillasParameters", fOpts.separateOutputAndRawData, statconn, fDBFlag);
    // DEBUG_OUT << "HDCALIBCHAININFO> Use Maker Summary::StatsMaker  (HillasParameters) named : " << smaker->GetName() << " fOpts.separateOutputAndRawData = " << fOpts.separateOutputAndRawData << " fDBFlag = " << fDBFlag << std::endl;
    // fChain->UseMaker(smaker);

    // Summary::StatsMaker* smaker2 = new Summary::StatsMaker("^stats$", "Intensity::Stats", fOpts.separateOutputAndRawData, statconn, fDBFlag);
    // DEBUG_OUT << "HDCALIBCHAININFO> Use Maker Summary::StatsMaker (Intensity::Stats) named : " << smaker->GetName() << " fOpts.separateOutputAndRawData = " << fOpts.separateOutputAndRawData << " fDBFlag = " << fDBFlag << std::endl;
    // fChain->UseMaker(smaker2);

  }




  // ----------------------------------------------------------------------
  // Optionally add the Model3D reconstruction. This requires nearly
  // a fully new chain, due to the assumptions needed for parisMVA
  // which are slightly different from the standard HD analysis

  // if (fOpts.includeModel3DInDST) {
  //   AddParisMVAChain( fChain, GetEventFolderName() );
  // }




  /// Add the calibration coefficient
  DBUtils::Handler::ReadOption defflag = DBUtils::Handler::Default;
  // simpletable::MyDBDefaults::set_config("test");
  // simpletable::MySQLconnection *fConnection =  new simpletable::MySQLconnection();
  //simpletable::MySQLconnection *fConnection = 0;
  DBUtils::Handler *fDBHandler = new DBUtils::Handler("^run$","DBHandler",defflag,fConnection);
  fDBHandler->Connect("HighLowRatio");
  fDBHandler->Connect("ADCtoPe");
  fDBHandler->Connect("FlatFieldCoeff");
  fDBHandler->Connect("CurrentOffsets");

  fWriter->AddInputFolder("FlatFieldCoeff$");
  fWriter->AddInputFolder("ADCtoPe$");
  fWriter->AddInputFolder("HighLowRatio$");

  fChain->UseMaker( fDBHandler );
  fChain->UseMaker( fWriter );

}
