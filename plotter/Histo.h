#ifndef Histo_h
#define Histo_h

#include <TROOT.h>
#include <TChain.h>
#include <THStack.h>
#include <TH1F.h>
#include <TFile.h>
#include <TMath.h>
#include <TLegend.h>
#include <iostream>
#include "TCut.h"

enum types{itBkg, itSignal, itData, itCompare, itSys, nTypes};
enum isysts{iJESUp, iJESDown, iJER, iBtagUp, iBtagDown, iMisTagUp, iMisTagDown, iLepEffUp, iLepEffDown, iTrigUp, iTrigDown, iPUUp, iPUDown, iFSUp, iFSDown, iGenMETUp, iGenMETDown, iISRJetsUp, iISRJetsDown, iQ2Up, iQ2Down, iPDFUp, iPDFDown, iHadUp, iHadDown, iNLOUp, iNLODown, nSysts};
TString SystName[nSysts] = {"JESUp", "JESDown", "JER", "BtagUp", "BtagDown", "MisTagUp", "MisTagDown", "LepEffUp", "LepEffDown", "TrigUp", "TrigDown", "PUUp", "PUDown", "FSUp", "FSDown", "GenMETUp", "GenMETDown", "ISRJetsUp", "ISRJetsDown", "Q2Up", "Q2Down", "PDFUp", "PDFDown", "HadUp", "HadDown", "NLOUp", "NLODown"};

const TString DefaultPath = "../temp/";
const TString prefix = "Tree_"; const TString sufix = ".root";
const TString DefaultTreeName = "MiniTree";


class Histo : public TH1F{
	public:
		Histo(const char *name, const char *title, Int_t nbins, Double_t xlow, Double_t xup);
		//Histo(const char *name, const char *title, Int_t nbins, const Float_t* xbins);
		Histo(const TH1F &h, Int_t tipo = 0, Int_t c = 1){
			((Histo&)h).Copy(*this);
			cuts = "cut"; xlabel = "[GeV]"; sysNorm = 0.25;
      SysTag = "";
			SetType(tipo);
			SetColor(c);
			SetStyle();
		}
		~Histo(){
			if(vsysd) delete vsysd;
			if(vsysu) delete vsysu;
		};

		void SetType(Int_t tipo = 0);
		void SetColor(Int_t c);
		void SetStyle();
		void SetStatUnc();

    TString GetTag(){return tag;}
    Int_t GetType(){return type;}
    Float_t GetYield(){return yield;}
    Float_t GetMax(){return max;}
    TString GetProcess(){return process;}
    Float_t GetSysNorm(){return sysNorm;}
    Int_t GetColor(){ return color;}
		void StackOverflow(Bool_t doStackOverflow = 1);
		void SetTag(TString p, TString t="", TString x = "", TString c = "");
		void SetProcess(TString p);
		void SetTitles(TString x, TString c = "");
		void SetParams(Int_t tipo = 0, Int_t color = 1, Float_t s = 0.25, TString t = "", TString x = "", TString c = "");
    void SetSysNorm(Float_t s){ sysNorm = s;}
    void SetSysTag(TString s){ SysTag = s;}
    TString GetSysTag(){return SysTag;}

		void AddToLegend(TLegend* leg, Bool_t doyi = 1);
		TH1F* GetVarHistoStatBin(Int_t bin = 0, TString dir = "Up");
		void AddToSystematics(Histo* hsys, TString dir = "Down");
    void SetBinsErrorFromSyst();

		Float_t *vsysu = NULL; 
		Float_t *vsysd = NULL;

	protected:
		Int_t type; 
		Int_t color;
		TString tag = "0"; TString process="0"; TString cuts="0"; TString xlabel="0";
		Float_t sysNorm;
		Double_t yield;
		Double_t max;
		Float_t haveStyle = 0;
		Bool_t IsStackOverflow = true;
    TString SysTag = "0";

};

Histo::Histo(const char *name, const char *title, Int_t nbins, Double_t xlow, Double_t xup)	: TH1F(name, title, nbins, xlow, xup){
	SetType(0);
	SetStyle();
	SetColor(1);
	tag = TString(name);
	process = TString(title);
}
/*
Histo::Histo(const char *name, const char *title, Int_t nbins, const Float_t* xbins): TH1F(name,title,nbins,xbins){
	SetType(0);
	SetStyle();
	SetColor(1);
	tag = TString(name);
	process = TString(title);
}*/

class AnalHisto{
  public:
   //AnalHisto(){};
   //AnalHisto(TString sample = "TTbar_Powheg", TString ct="", TString channel = "All", TString path = "", TString treeName = "", Int_t isyst = -1);
   AnalHisto(TString sample = "TTbar_Powheg", TCut    ct="", TString channel = "All", TString path = "", TString treeName = "", TString systag = "0");
   ~AnalHisto(){
     delete h;
		 delete tree->GetCurrentFile();
   };

   void SetCut(TCut t){cut = TCut(t);}
   void SetChannel(TString t){chan = t;}
   void SetChannel(Int_t i){
     if(i == 0 || i == 1) chan = "ElMu";
     else if(i == 2) chan = "Muon";
     else if(i == 3) chan = "Elec";
     else if(i == 4) chan = "SF";
     else chan = "All";
   }
   void SetSystematic(TString t){SysTag = t;}
   Histo* GetHisto(){return h;}
   void Fill(TString variable = "", TString sys = "0", Bool_t isFastSim = false);   
   void SetHisto(TString name, Int_t nb, Double_t xi, Double_t xe);
   //void SetHisto(TString name, Int_t nb, Float_t thebins[100]);

   Float_t *xbins = NULL;
   TTree* tree = NULL;
	 void SetTreeName(TString t);
	 void SetPath(TString t);
   Double_t GetYield(TString sys = "0");

  protected:
   TString path;
   TString treeName;
   Histo *h = NULL;
   TString sampleName;
   TCut cut;
   TString chan;
   TString weight;
   TString SysTag;
   TString var;
   Int_t AnalHistoBins; Double_t AnalHistoX0; Double_t AnalHistoXf;
   TString histoname;

   void loadTree();

};

#endif
