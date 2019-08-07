#include <iostream>
#include <sstream>

#include <getopt.h>

#include "cetlib_except/exception.h"

#include "TFile.h"
#include "TTree.h"
#include "TH2.h"
#include "TCut.h"

#include "Main/inc/InputParameters.hh"

namespace TrkAnaAnalysis {

  void PrintHelp() {
    std::cout << "Input Arguments:" << std::endl;
    std::cout << "\t-c, --config: configuration file" << std::endl;
    std::cout << "\t-h, --help: print this help message" << std::endl;
  }

  void ProcessArgs(int argc, char** argv, InputParameters& params) {
    const char* const short_opts = "c:h";

    const option long_opts[] = {
      {"config", required_argument, nullptr, 'c'},
      {"help", no_argument, nullptr, 'h'},
      {nullptr, no_argument, nullptr, 0}
    };

    while (true) {
      const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

      if (-1 == opt)
	break;

      switch (opt) {
    
      case 'c':
	params.Import(mu2e::SimpleConfig(std::string(optarg)));
	break;

      case 'h': // -h or --help
      case '?': // Unrecognized option
      default:
	PrintHelp();
	break;
      }
    }

    if (params.input_filename.empty()) {
      throw cet::exception("TrkAnaAnalysis::ProcessArgs") << "Input filename not specified!";
    }
    if (params.input_treename.empty()) {
      throw cet::exception("TrkAnaAnalysis::ProcessArgs") << "Input treename not specified!";
    }
  }

  int main(int argc, char **argv) {

    InputParameters params;
    ProcessArgs(argc, argv, params);

    TFile* file = new TFile(params.input_filename.c_str(), "READ");
 
    TTree* trkanaNeg = (TTree*) file->Get(params.input_treename.c_str());
  
    double min_t0 = 400;
    double max_t0 = 1800;
    double t0_width = 100;
    int n_t0_bins = (max_t0 - min_t0) / t0_width;
  
    double min_mom = 95;
    double max_mom = 115;
    double mom_width = 0.1;
    int n_mom_bins = (max_mom - min_mom) / mom_width;
  
    TH2F* hMomT0 = new TH2F("hMomT0", "", n_t0_bins,min_t0,max_t0, n_mom_bins,min_mom,max_mom);
    std::string drawcmd = "deent.mom:de.t0>>hMomT0";
    TCut goodfit = "de.status>0";
    TCut triggered = "(trigbits&0x208)>0";
    TCut inTimeWindow = "de.t0>700 && de.t0<1695";
    TCut inTanDipCut = "deent.td>0.577350 && deent.td<1.000";
    TCut inD0Cut = "deent.d0>-80 && deent.d0<105";
    TCut inMaxRCut = "deent.d0+2./deent.om>450 && deent.d0+2./deent.om<680";
    TCut noCRVHit = "(bestcrv<0||(de.t0-crvinfo._timeWindowStart[bestcrv]<-50||de.t0-crvinfo._timeWindowStart[bestcrv]>150.0))";
    TCut passTrkQual = "dequal.TrkQual>0.8";
    TCut recomom = "deent.mom>95";
  
    TCut cutcmd = goodfit + triggered + inTimeWindow + inTanDipCut + inD0Cut + inMaxRCut + noCRVHit + passTrkQual + recomom;
    trkanaNeg->Draw(drawcmd.c_str(), cutcmd, "goff");

    std::cout << "hMomT0 Entries = " << hMomT0->GetEntries() << std::endl;

    hMomT0->Draw("COLZ");

    std::cout << "Done" << std::endl;
  
    return 0;
  }
}

int main(int argc, char **argv) {

  TrkAnaAnalysis::main(argc, argv);
  return 0;
}
