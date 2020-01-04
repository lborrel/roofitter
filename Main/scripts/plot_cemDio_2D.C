void plot_cemDio_2D(std::string filename) {

  TFile* file = new TFile(filename.c_str(), "READ");
  
  RooWorkspace* ws = (RooWorkspace*) file->Get("cemDio_2D/cemDio_2D");

  ws->Print();

  RooRealVar* mom = ws->var("mom");
  RooPlot* plot_mom = mom->frame(RooFit::Range("fit"));
  RooRealVar* t0 = ws->var("t0");
  RooPlot* plot_t0 = t0->frame(RooFit::Range("fit"));
  
  RooAbsData* data = ws->data("data");
  TH2F* hist = (TH2F*) data->createHistogram("hist2d", *ws->var("mom"), RooFit::YVar(*ws->var("t0")));
  TCanvas* c = new TCanvas();
  hist->Draw("COLZ");

  data->plotOn(plot_mom);
  data->plotOn(plot_t0);
  
  RooAbsPdf* pdf = ws->pdf("model");
  pdf->plotOn(plot_mom);
  pdf->plotOn(plot_t0);
//  RooHist* mom_pull = plot->pullHist();
  pdf->plotOn(plot_mom, RooFit::Components("cemLLmomEffResp"), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));
  pdf->plotOn(plot_mom, RooFit::Components("dioPol58momEffResp"), RooFit::LineColor(kBlue), RooFit::LineStyle(kDashed));
  pdf->plotOn(plot_t0, RooFit::Components("cemLLt0"), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));
  pdf->plotOn(plot_t0, RooFit::Components("dioPol58t0"), RooFit::LineColor(kBlue), RooFit::LineStyle(kDashed));

  RooRealVar* NCe = ws->var("NCe");
  RooRealVar* NDio = ws->var("NDio");
  std::cout << "NCe = " << NCe->getValV() << " +/- " << NCe->getError() << std::endl;
  std::cout << "NDio = " << NDio->getValV() << " +/- " << NDio->getError() << std::endl;

  TCanvas* c_mom = new TCanvas();
  TPad* pad1_mom = new TPad("", "", 0.0, 0.4, 1.0, 1.0);
  TPad* pad2_mom = new TPad("", "", 0.0, 0.0, 1.0, 0.4);
  pad1_mom->Draw();
  pad2_mom->Draw();
  
  pad1_mom->SetLogy();
  pad1_mom->cd();
  plot_mom->Draw();
/*  TLatex* latex = new TLatex();
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


  pad2->cd();
  RooPlot* mom_pull_frame = mom->frame();
  mom_pull_frame->addPlotable(mom_pull, "P");
  mom_pull_frame->Draw();    
*/

  TCanvas* c_t0 = new TCanvas();
  TPad* pad1_t0 = new TPad("", "", 0.0, 0.4, 1.0, 1.0);
  TPad* pad2_t0 = new TPad("", "", 0.0, 0.0, 1.0, 0.4);
  pad1_t0->Draw();
  pad2_t0->Draw();
  
//  pad1_t0->SetLogy();
  pad1_t0->cd();
  plot_t0->Draw();
/*  TLatex* latex = new TLatex();
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


  pad2->cd();
  RooPlot* mom_pull_frame = mom->frame();
  mom_pull_frame->addPlotable(mom_pull, "P");
  mom_pull_frame->Draw();    
*/
}
