import ROOT
f = ROOT.TFile("/eos/experiment/sndlhc/convertedData/physics/2022/run_004705/sndsw_raw-0010.root")

import SndlhcGeo
geo = SndlhcGeo.GeoInterface("/eos/experiment/sndlhc/convertedData/physics/2022/geofile_sndlhc_TI18_V0_2022.root")

# Use FairRoot framework to arrange the workflow
# A FairRun is a wrapper of a collection of tasks
run = ROOT.FairRunAna()
# Input/output manager
ioman = ROOT.FairRootManager.Instance()
source = ROOT.FairFileSource(f)
ioman.SetSource(source)
#run.SetSource(source)
outFile = ROOT.TMemFile('dummy','CREATE')
sink = ROOT.FairRootFileSink(outFile)
ioman.SetSink(sink)
#run.SetSink(sink)

#load the simple tracking module
import SndlhcTracking
trackTask = SndlhcTracking.Tracking() 
# Add all task you'd like to run
run.AddTask(trackTask)
# Initialize the task collection(Fair run)
#run.Init()

# Add Hough Transform tracking
import SndlhcMuonReco
ht_ds = SndlhcMuonReco.MuonReco()
run.AddTask(ht_ds)
# Order is important! First set parameters and then initialize the run
ht_ds.SetParFile("/afs/cern.ch/user/s/sii/work/baseline/sw/rhel9_x86-64/sndsw/master-local1/python/TrackingParams_V2_28January2023.xml")
ht_ds.SetHoughSpaceFormat("linearSlopeIntercept")
ht_ds.SetTrackingCase('passing_mu_DS')
run.Init()

#loop over events
for i, event in enumerate(ioman.GetInTree()):
   trackTask.fittedTracks.Clear('C')
   ht_ds.kalman_tracks.Clear('C')
   #execute tasks
   rc=trackTask.ExecuteTask("DS")
   ht_ds.Exec(0)
   if len(ht_ds.kalman_tracks)+len(trackTask.fittedTracks)>0:
      print('HT-ST DS tracks',len(ht_ds.kalman_tracks)-len(trackTask.fittedTracks))
   if i >100: break
