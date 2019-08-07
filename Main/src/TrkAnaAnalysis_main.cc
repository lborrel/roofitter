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

  void ProcessArgs(int argc, char** argv, std::string& config_filename) {
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
	config_filename = std::string(optarg);
	break;

      case 'h': // -h or --help
      case '?': // Unrecognized option
      default:
	PrintHelp();
	break;
      }
    }
  }

  int main(int argc, char **argv) {

    std::string config_filename;
    ProcessArgs(argc, argv, config_filename);
    mu2e::SimpleConfig config(config_filename);

    std::string filename = config.getString("input.filename");
    if (filename.empty()) {
      throw cet::exception("TrkAnaAnalysis::main") << "Input filename not specified!";
    }
    TFile* file = new TFile(filename.c_str(), "READ");
    if (file->IsZombie()) {
      throw cet::exception("TrkAnaAnalysis::main") << "Input file " << filename << " is a zombie";
    }

    std::string treename = config.getString("input.treename");
    if (treename.empty()) {
      throw cet::exception("TrkAnaAnalysis::main") << "Input treename not specified!";
    } 
    TTree* trkanaNeg = (TTree*) file->Get(treename.c_str());
    if (!trkanaNeg) {
      throw cet::exception("TrkAnaANalysis::main") << "Input tree " << treename << " is not in file";
    }

    InputParameters params(config);

    TH2F* hMomT0 = new TH2F("hMomT0", "", params.t0.n_bins(),params.t0.min,params.t0.max, params.mom.n_bins(),params.mom.min,params.mom.max);
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

    std::cout << "Done" << std::endl;
  
    return 0;
  }
}

int main(int argc, char **argv) {

  TrkAnaAnalysis::main(argc, argv);
  return 0;
}
