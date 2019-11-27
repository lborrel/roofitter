void plot_cemDioCrvRpc_mom(std::string filename) {

  TFile* file = new TFile(filename.c_str(), "READ");
  
  RooWorkspace* ws = (RooWorkspace*) file->Get("cemDioCrvRpc_mom/cemDioCrvRpc_mom");

  ws->Print();

  RooRealVar* mom = ws->var("mom");
  RooPlot* plot = mom->frame(RooFit::Range("fit"));
  
  RooAbsData* data = ws->data("data");
  data->plotOn(plot);
  
  RooAbsPdf* pdf = ws->pdf("model");
  pdf->plotOn(plot);
  RooHist* mom_pull = plot->pullHist();
  pdf->plotOn(plot, RooFit::Components("cemLLmomEffResp"), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));
  pdf->plotOn(plot, RooFit::Components("dioPol58momEffResp"), RooFit::LineColor(kBlue), RooFit::LineStyle(kDashed));
  pdf->plotOn(plot, RooFit::Components("crvFlatmomEffResp"), RooFit::LineColor(kMagenta), RooFit::LineStyle(kDashed));
  pdf->plotOn(plot, RooFit::Components("rpcmomEffResp"), RooFit::LineColor(kGreen), RooFit::LineStyle(kDashed));

  RooRealVar* NCe = ws->var("NCe");
  RooRealVar* NDio = ws->var("NDio");
  RooRealVar* NCrv = ws->var("NCrv");
  RooRealVar* NRpc = ws->var("NRpc");
  //  RooAbsPdf* Rmue = ws->pdf("Rmue");
  std::cout << "NCe = " << NCe->getValV() << " +/- " << NCe->getError() << std::endl;
  std::cout << "NDio = " << NDio->getValV() << " +/- " << NDio->getError() << std::endl;
  std::cout << "NCrv = " << NCrv->getValV() << " +/- " << NCrv->getError() << std::endl;
  std::cout << "NRpc = " << NRpc->getValV() << " +/- " << NRpc->getError() << std::endl;
  //  std::cout << "Rmue = " << Rmue->getVal() << std::endl;

  TCanvas* c = new TCanvas();
    c->Divide(1, 2);
    c->GetPad(1)->SetLogy();
    c->cd(1);
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
  text << "N DIO = " << NDio->getVal() << " #pm " << NDio->getError();
  latex->SetTextColor(kBlue);
  latex->DrawLatexNDC(ndc_x, current_ndc_y, text.str().c_str());
  current_ndc_y += step_ndc_y;

  text.str("");
  text << std::fixed << std::setprecision(1);
  text << "N CRV = " << NCrv->getVal() << " #pm " << NCrv->getError();
  latex->SetTextColor(kMagenta);
  latex->DrawLatexNDC(ndc_x, current_ndc_y, text.str().c_str());
  current_ndc_y += step_ndc_y;

  text.str("");
  text << std::fixed << std::setprecision(1);
  text << "N RPC = " << NRpc->getVal() << " #pm " << NRpc->getError();
  latex->SetTextColor(kGreen);
  latex->DrawLatexNDC(ndc_x, current_ndc_y, text.str().c_str());
  current_ndc_y += step_ndc_y;

    c->cd(2);
    RooPlot* mom_pull_frame = mom->frame();
    mom_pull_frame->addPlotable(mom_pull, "P");
    mom_pull_frame->Draw();    
    //    RooPlot* new_plot = mom->frame(RooFit::Range(-5, 5));
    //    res->plotOn(new_plot);
    //    new_plot->Draw();
}
