#include "TPython.h"
R__LOAD_LIBRARY(/cvmfs/sndlhc.cern.ch/SNDLHC-2024/June25/sw/slc9_x86-64/ROOT/v6-28-04-local1/lib/libROOTTPython.so)

void geo_c(){

    TFile * f_in;
    f_in = new TFile("/eos/experiment/sndlhc/convertedData/physics/2024/run_245/run_009280/sndsw_raw-0010.root");

    // Create the FairRun and provide input and output
    FairRootManager* ioman = FairRootManager::Instance();
    FairFileSource* source = new FairFileSource(f_in);
    ioman->SetSource(source);
    TMemFile* outFile = new TMemFile("dummy","CREATE");
    FairRootFileSink* sink = new FairRootFileSink(outFile);
    ioman->SetSink(sink);
    FairRunAna* run = new FairRunAna();

    // Read the geometry
    TPython::Exec("import SndlhcGeo");
    TPython::Exec("SndlhcGeo.GeoInterface('/eos/experiment/sndlhc/convertedData/physics/2024/run_245/geofile_sndlhc_TI18_V5_2024.root')");
    // Define the detector objects
    Scifi * ScifiDet = new Scifi("Scifi", kTRUE);
    MuFilter * MufiDet = new MuFilter("MuFilter", kTRUE);
    // Read them! SndlhcGeo already added the contents of the geofile to the ListOfGlobals
    ScifiDet = (Scifi*) gROOT->GetListOfGlobals()->FindObject("Scifi");
    MufiDet = (MuFilter*) gROOT->GetListOfGlobals()->FindObject("MuFilter");

    // Embed simple tracking in C++
    TPython::Exec("import SndlhcTracking");
    TPython::Exec("simple_tracking_py = SndlhcTracking.Tracking()");
    // Pass the py class object to cpp using FairTask.
    FairTask* simple_tracking_cpp = TPython::Eval("simple_tracking_py");
    // Need to set the track type through python as this is a member function of the py class
    // and not the FairTask.
    TPython::Exec("simple_tracking_py.SetTrackClassType(0)");

    run->AddTask(simple_tracking_cpp);
    run->Init();
    // Get the tracks found by the simple tracking task
    TObjArray* st_tracks = TPython::Eval("simple_tracking_py.fittedTracks");
    
    // Read the event header and the hits
    TTree* eventTree = dynamic_cast<class TTree*>(ioman->GetInChain());
    TClonesArray * DigiScifiHits = new TClonesArray("sndScifiHit");
    eventTree->SetBranchAddress("Digi_ScifiHits", &DigiScifiHits);
    TClonesArray * DigiMuFilterHits = new TClonesArray("MuFilterHit");
    eventTree->SetBranchAddress("Digi_MuFilterHits", &DigiMuFilterHits);
    SNDLHCEventHeader* header = new SNDLHCEventHeader();
    eventTree->SetBranchAddress("EventHeader.", &header);

    // Take the first event and pass the event header to the det objects
    // so that proper alignment const are read from the geofile
    eventTree->GetEvent(0);
    ScifiDet->InitEvent(header);
    MufiDet->InitEvent(header);
    // Some printout for sanity check!
    //std::cout<<ScifiDet->GetConfParF("Scifi/station2t")<<std::endl;
    //std::cout<<ScifiDet->GetConfParI("Scifi/nscifi")<<std::endl;

    for (int i_event=0, Nevents=eventTree->GetEntries(); i_event<Nevents; i_event++){
         st_tracks->Clear();
         if (i_event>100) break;
         eventTree->GetEntry(i_event);
         simple_tracking_cpp->ExecuteTask("DS");
         for (Int_t i=0; i< st_tracks->GetEntries(); i++) {
           sndRecoTrack* aTrack = (sndRecoTrack*)st_tracks->At(i);
           //std::cout<<"found a track with slopeXZ "<<aTrack->getSlopeXZ()<<std::endl;
           TVector3 pos = aTrack->extrapolateToPlaneAtZ(250.);
           std::cout<<"Extrap pos x,y = "<<pos.x()<<", "<<pos.y()<<" for event N "<<i_event<<std::endl;
         }

         for (int i =0, max = DigiScifiHits->GetEntries(); i<max; i++){
           auto hit = ((sndScifiHit*)DigiScifiHits->At(i));
           // do some analysis
         }
         for (int i =0, max = DigiMuFilterHits->GetEntries(); i<max; i++){
           auto hit = ((MuFilterHit*)DigiMuFilterHits->At(i));
            if( hit->GetImpactT(true) == -999 ) continue;
            //std::cout<<"ImpactT: "<<hit->GetImpactT(true)<<std::endl;
         }
    }
}
