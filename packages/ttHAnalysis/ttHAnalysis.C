/*==============================================================================

							ttHAnalysis selector

==============================================================================*/
////////////////////////////////////////////////////////////////////////////////
//		Preprocessor directives
////////////////////////////////////////////////////////////////////////////////
// C++ usual debugging procedures
#ifdef DEBUGC
	#undef DEBUGC
#endif
//#define DEBUGC								// Uncomment for C++ debugging

//	Package inclusion
#include "ttHAnalysis.h"

ClassImp(ttHAnalysis); // PAF definition as class

////////////////////////////////////////////////////////////////////////////////
//		Initial definitions
////////////////////////////////////////////////////////////////////////////////
ttHAnalysis::ttHAnalysis() : PAFChainItemSelector() {
	// Defining to zero the aim of the histogram pointers
	for (UInt_t icat = 0; icat < gNCATEGORIES; icat++) {
		for (UInt_t ichan = 0; ichan < gNCHANNELS; ichan++) {
			fHEvents    		  [icat][ichan]	=	0; // Events
			fHTightLep		   	[icat][ichan]	=	0; // Yields
			fHFakeLep			    [icat][ichan]	=	0;
			fHLooseLep			  [icat][ichan]	=	0;
			fHTau				      [icat][ichan]	=	0;
			fHJet				      [icat][ichan]	=	0;
			fHMedBTagJet		  [icat][ichan]	=	0;
			fHLosBTagJet		  [icat][ichan]	=	0;
			fHPtLeading			  [icat][ichan]	=	0; // Kinematic
			fHPtSubLeading	  [icat][ichan]	=	0;
			fHPtSubSubLeading	[icat][ichan]	=	0;
			fHMET				      [icat][ichan]	=	0; // MET
			fHMHT				      [icat][ichan]	=	0;
			fHMETLD				    [icat][ichan]	=	0;
	    fHChargeSum			  [icat][ichan]	=	0; // Misc
			fHMass				    [icat][ichan]	=	0;
		}
	}
  fTree       = 0;
}

//	Core PAF-analysis methods
//------------------------------------------------------------------------------
void ttHAnalysis::Initialise() {
	PAF_INFO("ttHAnalysis", "+ Preprocess DONE");
	cout << endl;
	PAF_INFO("ttHAnalysis", "======================================== Initialization");

	PAF_INFO("ttHAnalysis", "+ Initializing analysis parameters");
	GetParameters();

	PAF_INFO("ttHAnalysis", "+ Initializing histograms");
	TH1::SetDefaultSumw2();
	InitialiseEventHistos();
	InitialiseYieldHistos();
	InitialiseKinematicHistos();
	InitialiseMETHistos();
	InitialiseMiscHistos();

	PAF_INFO("ttHAnalysis", "+ Initializing minitrees");
  fTree = CreateTree("MiniTree","Created with PAF");
  SetLeptonBranches();
  SetJetBranches();
  SetEventBranches();

	PAF_INFO("ttHAnalysis", "+ Initializing variables");
  InitialiseVariables();

  cout << endl;
	PAF_INFO("ttHAnalysis", "+ Initialization DONE");
	cout << endl;
	PAF_INFO("ttHAnalysis", "======================================== Analysis");
}

void ttHAnalysis::InsideLoop() {
	#ifdef DEBUGC
		cout << "DEBUGC - Beginning of InsideLoop" << endl;
	#endif

	// Get and set data members
	GetTreeVariables();
  GetEventVariables();

  if (!passTrigger)     return;
  if (!PassesPreCuts()) return;

	// Fill histograms
	FillEventHistos();
	FillYieldHistos();
	FillKinematicHistos();
	FillMETHistos();
	FillMiscHistos();

	SetMiniTreeVariables();
  fTree->Fill();

	#ifdef DEBUGC
		cout << "DEBUGC - End of InsideLoop" << endl;
	#endif
}

void ttHAnalysis::Summary() {
	PAF_INFO("ttHAnalysis", "+ Analysis DONE");
	cout << endl;
}

////////////////////////////////////////////////////////////////////////////////
//		Tree-related methods
////////////////////////////////////////////////////////////////////////////////
void ttHAnalysis::GetTreeVariables() {
	#ifdef DEBUGC
		cout << "DEBUGC - Beginning of GetTreeVariables"<< endl;
	#endif

  if (!gIsData){
		genWeight   = Get<Float_t>("genWeight");
	}

  MET  = 0.;
  MHT  = 0.;
  MET  = Get<Float_t>("met_pt");
	MHT	 = Get<Float_t>("mhtJet25");
}

void ttHAnalysis::SetLeptonBranches() {
  fTree->Branch("TnTightLepton",     &nTightLepton,     "nTightLepton/I");
  fTree->Branch("TnFakeableLepton",  &nFakeableLepton,  "nFakeableLepton/I");
  fTree->Branch("TnLooseLepton",     &nLooseLepton,     "nLooseLepton/I");
  fTree->Branch("TnTaus",            &nTaus,            "nTaus/I");
  fTree->Branch("TPtLeading",        &TPtLeading,       "TPtLeading/F");
  fTree->Branch("TPtSubLeading",     &TPtSubLeading,    "TPtSubLeading/F");
  fTree->Branch("TPtSubSubLeading",  &TPtSubSubLeading, "TPtSubSubLeading/F");
}

void ttHAnalysis::SetJetBranches() {
  fTree->Branch("TnMediumBTags" ,    &nMediumBTags,     "nMediumBTags/I");
  fTree->Branch("TnLooseBTags",      &nLooseBTags,      "nLooseBTags/I");
  fTree->Branch("TnJets",            &nJets,            "nJets/I");
}

void ttHAnalysis::SetEventBranches() {
  fTree->Branch("TIsEvent",      &TIsEvent,      "TIsEvent/I");
  fTree->Branch("TChannel",      &gChannel,      "gChannel/I");
  fTree->Branch("TCat",          &TCat,          "TCat/I");
  fTree->Branch("TpassTrigger",  &TpassTrigger,  "TpassTrigger/I");
  fTree->Branch("TisSS",         &TisSS,         "TisSS/I");
  fTree->Branch("TMET",          &MET,           "MET/F");
  fTree->Branch("TMHT",          &MHT,           "MHT/F");
  fTree->Branch("TMETLD",        &TMETLD,        "TMETLD/F");
  fTree->Branch("TCS",           &TCS,           "TCS/I");
  fTree->Branch("TMass",         &TMass,         "TMass/F");
  fTree->Branch("TWeight",       &TWeight,       "TWeight/F");
}

////////////////////////////////////////////////////////////////////////////////
//		Histogram-related methods
////////////////////////////////////////////////////////////////////////////////
// Initialising
//------------------------------------------------------------------------------
void ttHAnalysis::InitialiseEventHistos() {
	for (UInt_t icat = 0; icat < gNCATEGORIES; icat++) {
		for (UInt_t ichan = 0; ichan < gNCHANNELS; ichan++) {
			if (icat == threel 		&& ichan != All) 	continue;
			if (icat == fourl 		&& ichan != All) 	continue;
			if (icat == Total 		&& ichan != All) 	continue;
			fHEvents     [icat][ichan] = CreateH1F("H_Events_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""						, 1, 0, 1);
		}
	}
}

void ttHAnalysis::InitialiseYieldHistos() {
	for (UInt_t icat = 0; icat < gNCATEGORIES; icat++) {
		for (UInt_t ichan = 0; ichan < gNCHANNELS; ichan++) {
			if (icat == threel 		&& ichan != All) 	continue;
			if (icat == fourl 		&& ichan != All) 	continue;
			if (icat == Total 		&& ichan != All) 	continue;
			fHTightLep	[icat][ichan] = CreateH1F("H_TightLep_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""				, 6, 0, 6);
			fHFakeLep	  [icat][ichan] = CreateH1F("H_FakeLep_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""					, 5, 0, 5);
			fHLooseLep	[icat][ichan] = CreateH1F("H_LooseLep_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""				, 5, 0, 5);
			fHTau		    [icat][ichan] = CreateH1F("H_Tau_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""						, 3, 0, 3);
			fHJet		    [icat][ichan] = CreateH1F("H_Jet_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""						, 10, 0, 10);
			fHMedBTagJet[icat][ichan] = CreateH1F("H_MedBTagJet_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""				, 6, 0, 6);
			fHLosBTagJet[icat][ichan] = CreateH1F("H_LosBTagJet_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""				, 6, 0, 6);
		}
	}
}

void ttHAnalysis::InitialiseKinematicHistos() {
	for (UInt_t icat = 0; icat < gNCATEGORIES; icat++) {
		for (UInt_t ichan = 0; ichan < gNCHANNELS; ichan++) {
			if (icat == threel 		&& ichan != All) 	continue;
			if (icat == fourl 		&& ichan != All) 	continue;
			if (icat == Total 		&& ichan != All) 	continue;
			fHPtLeading			  [icat][ichan] = CreateH1F("H_PtLeading_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""		, 10, 0, 200);
			fHPtSubLeading		[icat][ichan] = CreateH1F("H_PtSubLeading_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""	, 10, 0, 200);
			fHPtSubSubLeading	[icat][ichan] = CreateH1F("H_PtSubSubLeading_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""	, 10, 0, 200);
		}
	}
}

void ttHAnalysis::InitialiseMETHistos() {
	for (UInt_t icat = 0; icat < gNCATEGORIES; icat++) {
		for (UInt_t ichan = 0; ichan < gNCHANNELS; ichan++) {
			if (icat == threel 		&& ichan != All) 	continue;
			if (icat == fourl 		&& ichan != All) 	continue;
			if (icat == Total 		&& ichan != All) 	continue;
			fHMET				[icat][ichan] = CreateH1F("H_MET_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""				, 10, 0, 400);
			fHMHT				[icat][ichan] = CreateH1F("H_MHT_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""				, 10, 0, 1000);
			fHMETLD			[icat][ichan] = CreateH1F("H_METLD_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""			, 10, 0, 2);
		}
	}
}

void ttHAnalysis::InitialiseMiscHistos() {
	for (UInt_t icat = 0; icat < gNCATEGORIES; icat++) {
		for (UInt_t ichan = 0; ichan < gNCHANNELS; ichan++) {
			if (icat == twolSS 		&& ichan != All) 	continue;
			if (icat == threel 		&& ichan != All) 	continue;
			if (icat == fourl 		&& ichan != All) 	continue;
			if (icat == Total 		&& ichan != All) 	continue;
			fHChargeSum		[icat][ichan] = CreateH1F("H_ChargeSum_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""		, 7, -3.5, 3.5);
			fHMass				[icat][ichan] = CreateH1F("H_Mass_"+gCatLabel[icat]+"_"+gChanLabel[ichan],""			, 10, 0, 400);
		}
	}
}

// Filling methods
//------------------------------------------------------------------------------
void ttHAnalysis::FillEventHistos() {
	for (UInt_t icat = 0; icat < gNCATEGORIES; icat++) {
		for (UInt_t ichan = 0; ichan < gNCHANNELS; ichan++) {
			if (icat 	== twolSS 	&& !Is2lSSEvent()) 						       continue;
			if (ichan == MuMu 		&& (gChannel != 2)) 					       continue;
			if (ichan == ElEl 		&& (gChannel != 3)) 					       continue;
			if (ichan == ElMu 		&& (gChannel != 1)) 			           continue;
			if (icat 	== threel 	&& (!Is3lEvent() || ichan != All)) 	 continue;
			if (icat 	== fourl 	  && (!Is4lEvent() || ichan != All)) 	 continue;
			if (icat 	== Total 		&& ichan != All ) 	                 continue;
			if (icat 	== Total 		&& (!Is2lSSEvent() || (gChannel != 2)) && (!Is2lSSEvent() || (gChannel != 3)) && (!Is2lSSEvent() || (gChannel != 1)) && !Is3lEvent()) 	continue;

			fHEvents     [icat][ichan]->Fill(0.5,EventWeight);
		}
	}
}

void ttHAnalysis::FillYieldHistos() {
	for (UInt_t icat = 0; icat < gNCATEGORIES; icat++) {
		for (UInt_t ichan = 0; ichan < gNCHANNELS; ichan++) {
      if (icat 	== twolSS 	&& !Is2lSSEvent()) 						       continue;
			if (ichan == MuMu 		&& (gChannel != 2)) 					       continue;
			if (ichan == ElEl 		&& (gChannel != 3)) 					       continue;
			if (ichan == ElMu 		&& (gChannel != 1)) 			           continue;
			if (icat 	== threel 	&& (!Is3lEvent() || ichan != All)) 	 continue;
			if (icat 	== fourl 	  && (!Is4lEvent() || ichan != All)) 	 continue;
			if (icat 	== Total 		&& ichan != All ) 	                 continue;
			if (icat 	== Total 		&& (!Is2lSSEvent() || (gChannel != 2)) && (!Is2lSSEvent() || (gChannel != 3)) && (!Is2lSSEvent() || (gChannel != 1)) && !Is3lEvent()) 	continue;
			fHTightLep	 [icat][ichan]->Fill(nTightLepton,EventWeight);
			fHFakeLep	   [icat][ichan]->Fill(nFakeableLepton,EventWeight);
			fHLooseLep	 [icat][ichan]->Fill(nLooseLepton,EventWeight);
			fHTau		     [icat][ichan]->Fill(nTaus,EventWeight);
			fHJet		     [icat][ichan]->Fill(nJets,EventWeight);
			fHMedBTagJet [icat][ichan]->Fill(nMediumBTags,EventWeight);
			fHLosBTagJet [icat][ichan]->Fill(nLooseBTags,EventWeight);
		}
	}
}

void ttHAnalysis::FillKinematicHistos() {
	for (UInt_t icat = 0; icat < gNCATEGORIES; icat++) {
		for (UInt_t ichan = 0; ichan < gNCHANNELS; ichan++) {
      if (icat 	== twolSS 	&& !Is2lSSEvent()) 						       continue;
			if (ichan == MuMu 		&& (gChannel != 2)) 					       continue;
			if (ichan == ElEl 		&& (gChannel != 3)) 					       continue;
			if (ichan == ElMu 		&& (gChannel != 1)) 			           continue;
			if (icat 	== threel 	&& (!Is3lEvent() || ichan != All)) 	 continue;
			if (icat 	== fourl 	  && (!Is4lEvent() || ichan != All)) 	 continue;
			if (icat 	== Total 		&& ichan != All ) 	                 continue;
			if (icat 	== Total 		&& (!Is2lSSEvent() || (gChannel != 2)) && (!Is2lSSEvent() || (gChannel != 3)) && (!Is2lSSEvent() || (gChannel != 1)) && !Is3lEvent()) 	continue;
			fHPtLeading			  [icat][ichan]->Fill(TightLepton[0].p.Pt(),EventWeight);
			fHPtSubLeading		[icat][ichan]->Fill(TightLepton[1].p.Pt(),EventWeight);
			fHPtSubSubLeading	[icat][ichan]->Fill(TightLepton[2].p.Pt(),EventWeight);
		}
	}
}

void ttHAnalysis::FillMETHistos() {
	for (UInt_t icat = 0; icat < gNCATEGORIES; icat++) {
		for (UInt_t ichan = 0; ichan < gNCHANNELS; ichan++) {
      if (icat 	== twolSS 	&& !Is2lSSEvent()) 						       continue;
			if (ichan == MuMu 		&& (gChannel != 2)) 					       continue;
			if (ichan == ElEl 		&& (gChannel != 3)) 					       continue;
			if (ichan == ElMu 		&& (gChannel != 1)) 			           continue;
			if (icat 	== threel 	&& (!Is3lEvent() || ichan != All)) 	 continue;
			if (icat 	== fourl 	  && (!Is4lEvent() || ichan != All)) 	 continue;
			if (icat 	== Total 		&& ichan != All ) 	                 continue;
			if (icat 	== Total 		&& (!Is2lSSEvent() || (gChannel != 2)) && (!Is2lSSEvent() || (gChannel != 3)) && (!Is2lSSEvent() || (gChannel != 1)) && !Is3lEvent()) 	continue;
			fHMET	  [icat][ichan]->Fill(MET,EventWeight);
			fHMHT	  [icat][ichan]->Fill(MHT,EventWeight);
			fHMETLD	[icat][ichan]->Fill(getMETLD(),EventWeight);
		}
	}
}

void ttHAnalysis::FillMiscHistos() {
	for (UInt_t icat = 0; icat < gNCATEGORIES; icat++) {
		for (UInt_t ichan = 0; ichan < gNCHANNELS; ichan++) {
			if (icat 	== twolSS 	&& (!Is2lSSEvent()	|| ichan != All))	continue;
			if (ichan == MuMu 		&& (gChannel != 2)) 					        continue;
			if (ichan == ElEl 		&& (gChannel != 3)) 					        continue;
			if (ichan == ElMu 		&& (gChannel != 1)) 					        continue;
			if (icat 	== threel 	&& (!Is3lEvent() || ichan != All)) 	  continue;
			if (icat 	== fourl 	  && (!Is4lEvent() || ichan != All)) 	  continue;
			if (icat 	== Total 		&& ichan != All) 	                    continue;
			if (icat 	== Total 		&& (!Is2lSSEvent() || (gChannel != 2)) && (!Is2lSSEvent() || (gChannel != 3)) && (!Is2lSSEvent() || (gChannel != 1)) && !Is3lEvent()) 	continue;
			fHChargeSum	[icat][ichan]->Fill(getCS(),EventWeight);
			if (icat == twolSS || icat == threel || icat == Total) fHMass	[icat][ichan]->Fill((TightLepton[0].p+TightLepton[1].p).M(),EventWeight);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	   Events selection
////////////////////////////////////////////////////////////////////////////////
Bool_t ttHAnalysis::Is2lSSEvent() {
	if (nTightLepton != 2) 	         return false;
	if (!isSS) 					             return false;
	if (TightLepton[0].p.Pt() < 20)  return false;
	if (TightLepton[1].p.Pt() < 15)  return false;
  if (TightLepton[2].p.Pt() > 10)  return false;

	if (nJets < 4) 					         return false;

	if (gChannel == 3) {
		if (abs((TightLepton[0].p + TightLepton[1].p).M() - Zm) < 10) return false;
		if (getMETLD() < 0.2) return false;
	}
	return true;
}

Bool_t ttHAnalysis::Is3lEvent() {
	if (nTightLepton != 3) return false;
	if (TightLepton[0].p.Pt() < 25) return false;
	if (TightLepton[1].p.Pt() < 15) return false;
	if (TightLepton[2].p.Pt() < 15) return false;

	for (UInt_t i = 0; i < nLooseLepton; i++) {
		for (UInt_t j = i+1; j < nLooseLepton; j++) {
			if (LooseLepton[i].type != LooseLepton[j].type) continue;
			if (LooseLepton[i].charge*LooseLepton[j].charge > 0) continue;
			if (abs((LooseLepton[i].p+LooseLepton[j].p).M() - Zm) < 10) return false;
		}
	}

	UInt_t twolds = 0;
	for (UInt_t i = 0; i < nTightLepton; i++) {
		for (UInt_t j = i+1; j < nTightLepton; j++) {
			if (TightLepton[i].type != TightLepton[j].type) continue;
			if (TightLepton[i].charge*TightLepton[j].charge < 0) twolds = 1;
		}
	}

	if (twolds != 1 && nJets < 4) {
		if (getMETLD() < 0.2) return false;
	} else if (nJets < 4) {
		if (getMETLD() < 0.3) return false;
	}

	if (abs(TightLepton[0].charge + TightLepton[1].charge + TightLepton[2].charge) != 1) return false;

  Lepton tmp_L1;
  Lepton tmp_L2;
  Lepton tmp_L3;
  Lepton tmp_L4;

  UInt_t OSSF = 0;
  for (UInt_t i = 0; i < nLooseLepton; i++) {
		for (UInt_t j = i+1; j < nLooseLepton; j++) {
			if (LooseLepton[i].type != LooseLepton[j].type)      continue;
			if (LooseLepton[i].charge*LooseLepton[j].charge > 0) continue;
      OSSF++;
      if (OSSF == 0) {
        tmp_L1 = LooseLepton[i];
        tmp_L2 = LooseLepton[j];
      }
      else if (OSSF == 1) {
        tmp_L3 = LooseLepton[i];
        tmp_L4 = LooseLepton[j];
      }
		}
	}
  if (OSSF == 1){
    if ((tmp_L1.p+tmp_L2.p+tmp_L3.p+tmp_L4.p).M() < 140) return false;
  }
	return true;
}

Bool_t ttHAnalysis::Is4lEvent() {
  if (nTightLepton <= 3) return false;
	if (TightLepton[0].p.Pt() < 25) return false;
	if (TightLepton[1].p.Pt() < 15) return false;
	if (TightLepton[2].p.Pt() < 15) return false;
	if (TightLepton[3].p.Pt() < 10) return false;

	for (UInt_t i = 0; i < nLooseLepton; i++) {
		for (UInt_t j = i+1; j < nLooseLepton; j++) {
			if (LooseLepton[i].type != LooseLepton[j].type) continue;
			if (LooseLepton[i].charge*LooseLepton[j].charge > 0) continue;
			if (abs((LooseLepton[i].p+LooseLepton[j].p).M() - Zm) < 10) return false;
		}
	}

	UInt_t twolds = 0;
	for (UInt_t i = 0; i < nTightLepton; i++) {
		for (UInt_t j = i+1; j < nTightLepton; j++) {
			if (TightLepton[i].type != TightLepton[j].type) continue;
			if (TightLepton[i].charge*TightLepton[j].charge < 0) twolds = 1;
		}
	}

	if (twolds != 1 && nJets < 4) {
		if (getMETLD() < 0.2) return false;
	} else if (nJets < 4) {
		if (getMETLD() < 0.3) return false;
	}

	if (abs(TightLepton[0].charge + TightLepton[1].charge + TightLepton[2].charge) != 1) return false;

  Lepton tmp_L1;
  Lepton tmp_L2;
  Lepton tmp_L3;
  Lepton tmp_L4;

  UInt_t OSSF = 0;
  for (UInt_t i = 0; i < nLooseLepton; i++) {
		for (UInt_t j = i+1; j < nLooseLepton; j++) {
			if (LooseLepton[i].type != LooseLepton[j].type)      continue;
			if (LooseLepton[i].charge*LooseLepton[j].charge > 0) continue;
      OSSF++;
      if (OSSF == 0) {
        tmp_L1 = LooseLepton[i];
        tmp_L2 = LooseLepton[j];
      }
      else if (OSSF == 1) {
        tmp_L3 = LooseLepton[i];
        tmp_L4 = LooseLepton[j];
      }
		}
	}
  if (OSSF == 1){
    if ((tmp_L1.p+tmp_L2.p+tmp_L3.p+tmp_L4.p).M() < 140) return false;
  }
	return true;
}

Bool_t ttHAnalysis::PassesPreCuts(){
	if (nTightLepton < 2) return false;

	for (UInt_t i = 0; i < nLooseLepton; i++) {
		for (UInt_t j = i+1; j < nLooseLepton; j++) {
		  if ((LooseLepton[i].p + LooseLepton[j].p).M() < 12) return false;
	  }
  }

	if (nJets < 2) return false;
	if (nLooseBTags < 2) {
		if (nMediumBTags < 1) return false;
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////
//	   Get methods
////////////////////////////////////////////////////////////////////////////////
void ttHAnalysis::GetParameters() {
  // Import essential and global variables of the execution
  gSampleName	    =	GetParam<TString>("sampleName");
  gIsData         =	GetParam<Bool_t>("IsData");
  gWeight         =	GetParam<Float_t>("weight"); // cross section / events in the sample
	gIsMCatNLO      =	GetParam<Bool_t>("IsMCatNLO");
}

void ttHAnalysis::GetEventVariables() {
  // Clean all event variables (also the minitree ones)
  TightLepton.clear();
  FakeableLepton.clear();
  LooseLepton.clear();
  Tau.clear();
  Jets.clear();
  nTightLepton    = 0;
  nFakeableLepton = 0;
  nLooseLepton    = 0;
  nTaus           = 0;
  nJets           = 0;
  nMediumBTags    = 0;
  nLooseBTags     = 0;
  gChannel        = 0;
  passTrigger     = 0;
  isSS            = 0;

  TCat            = 0;
  TisSS           = 0;
  TIsEvent        = 0;
  TPtLeading      = 0;
  TPtSubLeading   = 0;
  TPtSubSubLeading= 0;
  TCS             = 0;
  TMass           = 0;
  TMETLD          = 0;
  TWeight         = 0;
  TpassTrigger    = 0;

  // Import event-dependent variables
  TightLepton     = GetParam<vector<Lepton>>("selLeptons");
  FakeableLepton  = GetParam<vector<Lepton>>("vetoLeptons");
  LooseLepton     = GetParam<vector<Lepton>>("looseLeptons");
  Tau             = GetParam<vector<Lepton>>("selTaus");
  Jets            = GetParam<vector<Jet>>("selJets");

  nTightLepton    = GetParam<Int_t>("nSelLeptons");
  nFakeableLepton = GetParam<Int_t>("nVetoLeptons");
  nLooseLepton    = GetParam<Int_t>("nLooseLeptons");
  nTaus           = GetParam<Int_t>("nSelTaus");
  nJets           = GetParam<Int_t>("nSelJets");
  nMediumBTags    = GetParam<Int_t>("nSelBJets");

  gChannel        = GetParam<Int_t>("gChannel");
  passTrigger     = GetParam<Bool_t>("passTrigger");
  isSS            = GetParam<Bool_t>("isSS");

  EventWeight 	= 1.;
  if (!gIsData) {
    EventWeight = gWeight;
    if (gIsMCatNLO) EventWeight *= genWeight;
  }


  nLooseBTags     = GetnLooseBTags();
}

void ttHAnalysis::SetMiniTreeVariables(){
  if (Is2lSSEvent())    TCat  = 2;
  else if (Is3lEvent()) TCat  = 3;
  else if (Is4lEvent()) TCat  = 4;
  if (Is2lSSEvent() || Is3lEvent() || Is4lEvent()) TIsEvent = 1;
  TPtLeading      = TightLepton[0].p.Pt();
  TPtSubLeading   = TightLepton[1].p.Pt();
  if (TightLepton.size() > 2) TPtSubSubLeading = TightLepton[2].p.Pt();
  TCS             = getCS();
  TMass           = (TightLepton[0].p+TightLepton[1].p).M();
  TMETLD          = getMETLD();
  TWeight         = EventWeight;
  TisSS           = isSS;
  TpassTrigger    = passTrigger;
}

Float_t ttHAnalysis::getMETLD() {
	Float_t metld;
	metld = MET * 0.00397 + MHT * 0.00265;
	return metld;
}

Float_t ttHAnalysis::getMET(){ return MET; }

Int_t ttHAnalysis::getCS(){
	Int_t cs = 0;
	for (UInt_t i = 0; i < TightLepton.size(); i++) {
		cs += TightLepton[i].charge;
	}
	return cs;
}

Int_t ttHAnalysis::GetnLooseBTags(){
  UInt_t nloosebtag = 0;
  for(Int_t i = 0; i < Jets.size(); i++) {
    if (Jets[i].csv > 0.5426) nloosebtag++;
  }
  return nloosebtag;
}

void ttHAnalysis::InitialiseVariables() {
  TightLepton.clear();
  FakeableLepton.clear();
  LooseLepton.clear();
  Tau.clear();
  Jets.clear();
  nTightLepton    = 0;
  nFakeableLepton = 0;
  nLooseLepton    = 0;
  nTaus           = 0;
  nJets           = 0;
  nMediumBTags    = 0;
  nLooseBTags     = 0;
  gChannel        = 0;
  passTrigger     = 0;
  isSS            = 0;

  TCat            = 0;
  TisSS           = 0;
  TIsEvent        = 0;
  TPtLeading      = 0;
  TPtSubLeading   = 0;
  TPtSubSubLeading= 0;
  TCS             = 0;
  TMass           = 0;
  TMETLD          = 0;
  TWeight         = 0;
  TpassTrigger    = 0;
}
