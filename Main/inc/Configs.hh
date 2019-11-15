#ifndef Configs_hh_
#define Configs_hh_

#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/OptionalTable.h"

namespace roofitter {

  struct ParameterConfig {
    fhicl::Atom<std::string> name{fhicl::Name("name"), fhicl::Comment("Parameter name")};
    fhicl::OptionalAtom<double> value{fhicl::Name("value"), fhicl::Comment("Parameter value")};
    fhicl::OptionalAtom<double> minValue{fhicl::Name("minValue"), fhicl::Comment("Minimum parameter value")};
    fhicl::OptionalAtom<double> maxValue{fhicl::Name("maxValue"), fhicl::Comment("Maximum parameter value")};
  };

  struct FormulaConfig {
    fhicl::Atom<std::string> formula{fhicl::Name("formula"), fhicl::Comment("Formula string")};
    fhicl::Sequence< fhicl::Table<ParameterConfig> > parameters{fhicl::Name("parameters"), fhicl::Comment("List of parameters that appear in the formula with values and value ranges")};
  };

  struct PdfConfig {
    fhicl::Atom<std::string> name{fhicl::Name("name"), fhicl::Comment("Name of this PDF")};
    fhicl::Atom<std::string> formula{fhicl::Name("formula"), fhicl::Comment("PDF function in RooFit factory format")};
  };

}

#endif
