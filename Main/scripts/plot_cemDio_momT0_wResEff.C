void plot_cemDio_momT0_wResEff() {

  int first_run = 7;
  int last_run = 7;
  for (int run_number = first_run; run_number <= last_run; ++run_number) {
    std::string analysis_name = "cemDio_momT0_wResEff";

    std::stringstream filename;
    filename.str("");
    filename << "ana-run" << run_number << "-t0.root";
    TFile* file = new TFile(filename.str().c_str(), "READ");

    std::string wsname = analysis_name + "/" + analysis_name;
    RooWorkspace* ws = (RooWorkspace*) file->Get(wsname.c_str());

    ws->Print();

    TCanvas* c1 = new TCanvas();
    RooAbsData* data = ws->data("data");
    TH2D* hist = (TH2D*) data->createHistogram("hist2d", *ws->var("mom"), RooFit::YVar(*ws->var("t0")));
    hist->Draw("COLZ");

    RooAbsPdf* pdf = ws->pdf("model");
    pdf->Print();

    TCanvas* c2 = new TCanvas();
    c2->SetLogy();
    RooRealVar* mom = ws->var("mom");
    RooAbsPdf* mom_pdf = pdf->createProjection(*ws->var("t0"));
    mom_pdf->Print();
    RooPlot* mom_plot = mom->frame();
    data->plotOn(mom_plot);
    mom_pdf->plotOn(mom_plot);
    pdf->plotOn(mom_plot, RooFit::Components("cemLL2D"), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed), RooFit::NormRange("fit"));
    pdf->plotOn(mom_plot, RooFit::Components("dioPol582D"), RooFit::LineColor(kBlue), RooFit::LineStyle(kDashed), RooFit::NormRange("fit"));
    mom_plot->Draw();

    
    TCanvas* c3 = new TCanvas();
    RooRealVar* t0 = ws->var("t0");
    RooAbsPdf* t0_pdf = pdf->createProjection(*ws->var("mom"));
    RooPlot* t0_plot = t0->frame();
    data->plotOn(t0_plot);
    t0_pdf->plotOn(t0_plot);
    pdf->plotOn(t0_plot, RooFit::Components("cemLL2D"), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed), RooFit::NormRange("fit"));
    pdf->plotOn(t0_plot, RooFit::Components("dioPol582D"), RooFit::LineColor(kBlue), RooFit::LineStyle(kDashed), RooFit::NormRange("fit"));
    t0_plot->Draw();    
  }
}
