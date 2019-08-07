#include <iostream>
#include <sstream>

#include <getopt.h>

#include "cetlib_except/exception.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2.h"
#include "TCut.h"

#include "Main/inc/Analysis.hh"

namespace trkana {

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
      throw cet::exception("trkana::main") << "Input file " << filename << " is a zombie";
    }

    std::string treename = config.getString("input.treename");
    TTree* trkana = (TTree*) file->Get(treename.c_str());
    if (!trkana) {
      throw cet::exception("trkana::main") << "Input tree " << treename << " is not in file";
    }

    std::vector<Analysis> analyses;
    std::vector<std::string> ana_names;
    config.getVectorString("analyses", ana_names);
    for (const auto& i_ana_name : ana_names) {
      analyses.push_back(Analysis(i_ana_name, config));
    }

    for (auto& i_ana : analyses) {
      if (config.getBool(i_ana.name+".fillHist", false)) {
	i_ana.fillData(trkana);
	std::cout << "hist Entries = " << i_ana.hist->GetEntries() << std::endl;

	if (config.getBool(i_ana.name+".fit", false)) {
	  i_ana.fit();
	  i_ana.calculate();
	}
      }
    }

    std::string outfilename = config.getString("output.filename");
    TFile* outfile = new TFile(outfilename.c_str(), "RECREATE");
    for (auto& i_ana : analyses) {
      TDirectory* outdir = outfile->mkdir(i_ana.name.c_str());
      outdir->cd();
      i_ana.Write();
      outfile->cd();
    }
    outfile->Write();
    outfile->Close();

    std::cout << "Done" << std::endl;
    return 0;
  }
}

int main(int argc, char **argv) {

  trkana::main(argc, argv);
  return 0;
}
