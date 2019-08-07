#include <iostream>
#include <sstream>

#include <getopt.h>

#include "cetlib_except/exception.h"

#include "TFile.h"
#include "TTree.h"
#include "TH2.h"
#include "TCut.h"

#include "Main/inc/Analysis.hh"

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
    TFile* file = new TFile(filename.c_str(), "READ");
    if (file->IsZombie()) {
      throw cet::exception("TrkAnaAnalysis::main") << "Input file " << filename << " is a zombie";
    }

    std::string treename = config.getString("input.treename");
    TTree* trkana = (TTree*) file->Get(treename.c_str());
    if (!trkana) {
      throw cet::exception("TrkAnaANalysis::main") << "Input tree " << treename << " is not in file";
    }

    Analysis ana(config);

    trkana->Draw(ana.drawcmd().c_str(), ana.cutcmd(), "goff");

    std::cout << "hMomT0 Entries = " << ana.hMomT0.GetEntries() << std::endl;

    std::cout << "Done" << std::endl;

    std::string outfilename = config.getString("output.filename");
    TFile* outfile = new TFile(outfilename.c_str(), "RECREATE");
    ana.hMomT0.Write();
    outfile->Write();
    outfile->Close();

    return 0;
  }
}

int main(int argc, char **argv) {

  TrkAnaAnalysis::main(argc, argv);
  return 0;
}
