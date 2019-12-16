void plot_cemDio_mom_comp() {

  TFile* file_binned = new TFile("ana_binned.root", "READ");
  TFile* file_unbinned = new TFile("ana_unbinned.root", "READ");
  
  RooWorkspace* ws_binned = (RooWorkspace*) file_binned->Get("cemDio_mom/cemDio_mom");
  RooWorkspace* ws_unbinned = (RooWorkspace*) file_unbinned->Get("cemDio_mom/cemDio_mom");

  ws_binned->Print();
  ws_unbinned->Print();

  RooRealVar* mom_binned = ws_binned->var("mom");
  RooPlot* plot_binned = mom_binned->frame(RooFit::Range("fit"));

  RooAbsData* data_binned = ws_binned->data("data");
  data_binned->plotOn(plot_binned);

  RooRealVar* mom_unbinned = ws_unbinned->var("deentmom");
  RooPlot *plot_unbinned = mom_unbinned->frame();
  RooAbsData* data_unbinned = ws_unbinned->data("unbinned_data");
  data_unbinned->plotOn(plot_unbinned, RooFit::MarkerColor(kGreen));

  RooAbsPdf* pdf_binned = ws_binned->pdf("model");
  pdf_binned->plotOn(plot_binned);
  RooHist* mom_pull_binned = plot_binned->pullHist();
  pdf_binned->plotOn(plot_binned, RooFit::Components("cemLLmomEffResp"), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));
  pdf_binned->plotOn(plot_binned, RooFit::Components("dioPol58momEffResp"), RooFit::LineColor(kBlue), RooFit::LineStyle(kDashed));

  RooAbsPdf* pdf_unbinned = ws_unbinned->pdf("model");
  pdf_unbinned->plotOn(plot_unbinned);
  RooHist* mom_pull_unbinned = plot_unbinned->pullHist();
  pdf_unbinned->plotOn(plot_unbinned, RooFit::Components("cemLLmomEffResp"), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));
  pdf_unbinned->plotOn(plot_unbinned, RooFit::Components("dioPol58momEffResp"), RooFit::LineColor(kBlue), RooFit::LineStyle(kDashed));

  RooRealVar* NCe_binned = ws_binned->var("NCe");
  RooRealVar* NDio_binned = ws_binned->var("NDio");
  std::cout << "NCe_binned = " << NCe_binned->getValV() << " +/- " << NCe_binned->getError() << std::endl;
  std::cout << "NDio_binned = " << NDio_binned->getValV() << " +/- " << NDio_binned->getError() << std::endl;

  RooRealVar* NCe_unbinned = ws_unbinned->var("NCe");
  RooRealVar* NDio_unbinned = ws_unbinned->var("NDio");
  std::cout << "NCe_unbinned = " << NCe_unbinned->getValV() << " +/- " << NCe_unbinned->getError() << std::endl;
  std::cout << "NDio_unbinned = " << NDio_unbinned->getValV() << " +/- " << NDio_unbinned->getError() << std::endl;

  TCanvas* c = new TCanvas();
  TPad* pad1 = new TPad("", "", 0.0, 0.4, 1.0, 1.0);
  TPad* pad2 = new TPad("", "", 0.0, 0.0, 1.0, 0.4);
  pad1->Draw();
  pad2->Draw();
  
  pad1->SetLogy();
  pad1->cd();
  plot_binned->Draw();
  plot_unbinned->Draw("SAME");
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
  text << "N Ce binned = " << NCe_binned->getVal() << " #pm " << NCe_binned->getError() << " ; N Ce unbinned = " << NCe_unbinned->getVal() << " #pm " << NCe_unbinned->getError();
  latex->SetTextColor(kRed);
  latex->DrawLatexNDC(ndc_x, current_ndc_y, text.str().c_str());
  current_ndc_y += step_ndc_y;
  
  text.str("");
  text << std::fixed << std::setprecision(1);
  text << "N Dio binned = " << NDio_binned->getVal() << " #pm " << NDio_binned->getError() << " ; N Dio unbinned = " << NDio_unbinned->getVal() << " #pm " << NDio_unbinned->getError();
  latex->SetTextColor(kBlue);
  latex->DrawLatexNDC(ndc_x, current_ndc_y, text.str().c_str());
  current_ndc_y += step_ndc_y;


//  pad2->cd();
//  RooPlot* mom_pull_frame_binned = mom_binned->frame();
//  mom_pull_frame_binned->addPlotable(mom_pull_binned, "P");
//  mom_pull_frame_binned->Draw();    

//  pad2->cd();
//  RooPlot* mom_pull_frame_unbinned = mom_unbinned->frame();
//  mom_pull_frame_unbinned->addPlotable(mom_pull_unbinned, "P");
//  mom_pull_frame_unbinned->Draw();    
}
