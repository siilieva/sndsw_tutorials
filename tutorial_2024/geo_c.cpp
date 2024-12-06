#include "TPython.h"
R__LOAD_LIBRARY(/cvmfs/sndlhc.cern.ch/SNDLHC-2024/June25/sw/slc9_x86-64/ROOT/v6-28-04-local1/lib/libROOTTPython.so)

void geo_c(){

    TFile * f_in;
    f_in = new TFile("/eos/experiment/sndlhc/convertedData/physics/2024/run_245/run_009280/sndsw_raw-0010.root");
    // Read the event header and the hits
    TTree * cbmsim = (TTree*) f_in->Get("rawConv");
    TClonesArray * DigiScifiHits = new TClonesArray("sndScifiHit");
    cbmsim->SetBranchAddress("Digi_ScifiHits", &DigiScifiHits);
    TClonesArray * DigiMuFilterHits = new TClonesArray("MuFilterHit");
    cbmsim->SetBranchAddress("Digi_MuFilterHits", &DigiMuFilterHits);
    SNDLHCEventHeader* header = new SNDLHCEventHeader();
    cbmsim->SetBranchAddress("EventHeader.", &header);

    TPython::Exec("import SndlhcGeo");
    TPython::Exec("SndlhcGeo.GeoInterface('/eos/experiment/sndlhc/convertedData/physics/2024/run_245/geofile_sndlhc_TI18_V5_2024.root')");
    // Define the detector objects
    Scifi * ScifiDet = new Scifi("Scifi", kTRUE);
    MuFilter * MufiDet = new MuFilter("MuFilter", kTRUE);
    // Read them! SndlhcGeo alredy added the contents of the geofile to the ListOfGlobals
    ScifiDet = (Scifi*) gROOT->GetListOfGlobals()->FindObject("Scifi");
    MufiDet = (MuFilter*) gROOT->GetListOfGlobals()->FindObject("MuFilter");
    
    // Take the first event and pass the event header to the det objects
    // so that proper alignment const are read from the geofile
    cbmsim->GetEvent(0);
    ScifiDet->InitEvent(header);
    MufiDet->InitEvent(header);
    // Some printout for sanity check!
    //cout<<ScifiDet->GetConfParF("Scifi/station2t")<<endl;
    //cout<<ScifiDet->GetConfParI("Scifi/nscifi")<<endl;

    for (int i_event=0, Nevents=cbmsim->GetEntries(); i_event<Nevents; i_event++){
         if (i_event>1000) break;
         cbmsim->GetEntry(i_event);

         for (int i =0, max = DigiScifiHits->GetEntries(); i<max; i++){
           auto hit = ((sndScifiHit*)DigiScifiHits->At(i));
           // do some analysis
         }
         for (int i =0, max = DigiMuFilterHits->GetEntries(); i<max; i++){
           auto hit = ((MuFilterHit*)DigiMuFilterHits->At(i));
           //if( hit->GetImpactT(true) == -999 ) continue;
         }
    }
}
