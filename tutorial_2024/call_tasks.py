import os # to access env variables
import ROOT
f = ROOT.TFile("/eos/experiment/sndlhc/convertedData/physics/2024/run_245/run_009243/sndsw_raw-0010.root")

import SndlhcGeo
geo = SndlhcGeo.GeoInterface("/eos/experiment/sndlhc/convertedData/physics/2024/run_245/geofile_sndlhc_TI18_V5_2024.root")

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

#avoiding some error messages
xrdb = ROOT.FairRuntimeDb.instance()
xrdb.getContainer("FairBaseParSet").setStatic()
xrdb.getContainer("FairGeoParSet").setStatic()

#load the simple tracking module
import SndlhcTracking
trackTask = SndlhcTracking.Tracking() 
# Add all task you'd like to run
run.AddTask(trackTask)
trackTask.SetTrackClassType(0)
# Initialize the task collection(Fair run)
#run.Init()

# Add Hough Transform tracking
import SndlhcMuonReco
ht_ds_task = SndlhcMuonReco.MuonReco()
run.AddTask(ht_ds_task)
# Order is important! First set parameters and then initialize the run
ht_ds_task.SetParFile("{}/python/TrackingParams_V2_28January2023.xml".format(os.environ["SNDSW_ROOT"]))
ht_ds_task.SetHoughSpaceFormat("linearSlopeIntercept")
ht_ds_task.SetTrackingCase('passing_mu_DS')
run.Init()

#loop over events
for i, event in enumerate(ioman.GetInTree()):# dont use f.rawConv
   trackTask.fittedTracks.Clear()
   ht_ds_task.kalman_tracks.Clear()
   #execute tasks
   rc=trackTask.ExecuteTask("DS")
   #for index in range(len(trackTask.fittedTracks)):
   #  print(trackTask.fittedTracks[index].getTrackType(),trackTask.fittedTracks[index].getSlopeXZ())
   ht_ds_task.Exec(0)
   if abs(len(ht_ds_task.kalman_tracks)-len(trackTask.fittedTracks))>0:
      print('HT-ST DS tracks',len(ht_ds_task.kalman_tracks)-len(trackTask.fittedTracks))
   if i >1000: break
