# roofitter
This program runs RooFit on a TTree. The user can define cuts, observables and pdfs for their analysis with a SimpleConfig file.

## Setup
To setup:
> source setup.sh

Note that you will need to be able to see Mu2e Offline releases on cvmfs

## Compilation
To compile:
> scons -j4

## Example
roofitter does not need to run on TrkAna trees, but we will use one for this example:
> mu2e -c $MU2E_BASE_RELEASE/TrkDiag/fcl/TrkAnaRecoEnsemble-Data.fcl -s ensemble-file.art --TFileName trkana.root

To fit the momentum spectrum we can use this example:
> roofitter -c Main/cfg/example.cfg -i trkana-file.root -t TrkAnaNeg/trkana -o ana.root

And you can plot the result:
> root -l  Main/scripts/plot_cemDio_mom.C\(\"ana.root\"\)

## More Details
If you open up example.cfg, you will see that we #include two files specific to TrkAna:
> cuts/cd3_trkana.cfg -- defines the leaves for the cut variables
> observables/leaves_trkana.cfg -- defines the leaves for the observables

The analysis itself is defined in analyses/cemDio_mom.cfg. It is worth noting that in a single run of roofitter, you can run more than one analysis by adding to the line:
> vector<string> analyses = {"cemDio_mom"}

If you look in analyses/cemDio_mom.cfg, you will see that we #include files to define the observable and components of this analysis. 
 * the observable file defines histogram limits, and possible efficiency and resolution functions for the variable
 * the component file defines possible truth PDFs for the component for a given observable

Back in the cemDIO_mom.cfg file, we select the observable ("mom") and components ("cemLL" and "dioPol58") that we want as well as the cuts (note that you can invert a cut with "!"). We also define which of the resolution and efficiency models we want for our  observable.

Finally, we define the full model that we want to fit. Note that the PDF names for each component are:
 * true PDF name + observable + "EffRes"

where "EffRes" is if you want the efficiency and resolution effects included.

## Input Arguments
     -c, --config [cfg file]: input configuration file
     -i, --input [root file]: input ROOT file containing the tree (overrides anything in cfg file)
     -t, --tree [tree name]: tree name (inc. directory) in the input file (overrides anything in cfg file)
     -o, --output [root file]: output ROOT file that will be created (overrides anything in cfg file)
     -d, --debug-config [filename]: print out the final config file to file
     -h, --help: print this help message

