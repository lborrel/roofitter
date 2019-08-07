#ifndef Component_hh_
#define Component_hh_

namespace TrkAnaAnalysis {

  class Component {

  public:
    Component(const mu2e::SimpleConfig& config, std::string name) :
      name(name)
    {
    }

    std::string name;
    RooAbsPdf* pdf;
  };
}

#endif
