void plot_cemDio_mom(std::string filename) {

  TFile* file = new TFile(filename.c_str(), "READ");
  
  RooWorkspace* ws = (RooWorkspace*) file->Get("cemDioRPC_mom/cemDioRPC_mom");

  ws->Print();

  RooRealVar* mom = ws->var("mom");
  RooPlot* plot = mom->frame(RooFit::Range("fit"));
  
  RooAbsData* data = ws->data("data");
  data->plotOn(plot);
  
  RooAbsPdf* pdf = ws->pdf("model");
  pdf->plotOn(plot);
  RooHist* mom_pull = plot->pullHist();
  pdf->plotOn(plot, RooFit::Components("cemLLmomEffRes"), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));
  pdf->plotOn(plot, RooFit::Components("dioPol58momEffRes"), RooFit::LineColor(kBlue), RooFit::LineStyle(kDashed));
  pdf->plotOn(plot, RooFit::Components("RPCEffRes"), RooFit::LineColor(kBlue), RooFit::LineStyle(kDashed));

  RooRealVar* NCe = ws->var("NCe");
  RooRealVar* NDio = ws->var("NDio");
  RooRealVar* NRPC = ws->var("NRPC");

  std::cout << "NCe = " << NCe->getValV() << " +/- " << NCe->getError() << std::endl;
  std::cout << "NDio = " << NDio->getValV() << " +/- " << NDio->getError() << std::endl;
  std::cout << "NRPC = " << NRPC->getValV() << " +/- " << NRPC->getError() << std::endl;

  TCanvas* c = new TCanvas();
  TPad* pad1 = new TPad("", "", 0.0, 0.4, 1.0, 1.0);
  TPad* pad2 = new TPad("", "", 0.0, 0.0, 1.0, 0.4);
  pad1->Draw();
  pad2->Draw();
  
  pad1->SetLogy();
  pad1->cd();
  plot->Draw();
  TLatex* latex = new TLatex();
  latex->SetTextAlign(22);
  latex->SetTextSize(0.06);
  std::stringstream text;
  double ndc_x = 0.7;
  double first_ndc_y = 0.6;
  double step_ndc_y = -0.05;
  double current_ndc_y = first_ndc_y;
  text.str("");
  text << std::fixed << std::setprecision(1);
  text << "N Ce = " << NCe->getVal() << " #pm " << NCe->getError();
  latex->SetTextColor(kRed);
  latex->DrawLatexNDC(ndc_x, current_ndc_y, text.str().c_str());
  current_ndc_y += step_ndc_y;
  
  text.str("");
  text << std::fixed << std::setprecision(1);
  text << "N Dio = " << NDio->getVal() << " #pm " << NDio->getError();
  latex->SetTextColor(kBlue);
  latex->DrawLatexNDC(ndc_x, current_ndc_y, text.str().c_str());
  current_ndc_y += step_ndc_y;

  text.str("");
  text << std::fixed << std::setprecision(1);
  text << "N RPC = " << NRPC->getVal() << " #pm " << NRPC->getError();
  latex->SetTextColor(kBlue);
  latex->DrawLatexNDC(ndc_x, current_ndc_y, text.str().c_str());
  current_ndc_y += step_ndc_y;


  pad2->cd();
  RooPlot* mom_pull_frame = mom->frame();
  mom_pull_frame->addPlotable(mom_pull, "P");
  mom_pull_frame->Draw();    

}
