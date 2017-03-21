/********************************************************************
**                                                                 **
** File   : app/classificator.cpp                                  **
** Authors: Viktor Richter                                         **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
********************************************************************/

#include "RsbClassificator.h"
#include <boost/program_options.hpp>
#include <boost/timer.hpp>
#include <fformation/GroupDetectorFactory.h>
#include <iostream>
#include <rsc/misc/SignalWaiter.h>

#if HAVE_GCO
#include "GraphCutsOptimization.h"
#endif

using fformation::GroupDetectorFactory;
using fformation::GroupDetector;
using fformation::Options;
using fformation::Option;

#if HAVE_GCO
GroupDetectorFactory &factory =
    GroupDetectorFactory::getDefaultInstance().addDetector(
        "gco", GraphCutsOptimization::creator());
#else
GroupDetectorFactory &factory = GroupDetectorFactory::getDefaultInstance();
#endif

static std::string getClassificators(std::string prefix) {
  std::stringstream str;
  str << prefix;
  str << " ( ";
  for (auto cl : factory.listDetectors()) {
    str << cl << " | ";
  }
  str << ")";
  return str.str();
}

int main(const int argc, const char **args) {
  boost::program_options::variables_map program_options;
  boost::program_options::options_description desc("Allowed options");
  desc.add_options()("help,h", "produce help message");
  desc.add_options()(
      "classificator,c",
      boost::program_options::value<std::string>()->default_value("list"),
      getClassificators(
          "Which classificator should be used for evaluation. Possible: ")
          .c_str());
  desc.add_options()("inscope,i",
                     boost::program_options::value<std::string>()->required(),
                     "The scope to listen for person hypotheses");
  desc.add_options()("outscope,o",
                     boost::program_options::value<std::string>()->required(),
                     "The scope to publish group assignments");
  desc.add_options()(
      "mdl,m", boost::program_options::value<double>()->default_value(40000),
      "The minimum description length prior.");
  desc.add_options()(
      "stride,s", boost::program_options::value<double>()->default_value(10),
      "The distance btw. a persons position and trnsactional space.");

  try {
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, args, desc),
        program_options);
    boost::program_options::notify(program_options);
  } catch (const std::exception &e) {
    std::cerr << "Error while parsing command line parameters:\n\t" << e.what()
              << "\n";
    std::cerr << desc << std::endl;
    return 1;
  }

  if (program_options.count("help")) {
    std::cout << desc << "\n";
    return 0;
  }

  auto config =
      factory.parseConfig(program_options["classificator"].as<std::string>());
  config.second.addIfMissing("mdl", program_options["mdl"].as<double>());
  config.second.addIfMissing("stride", program_options["stride"].as<double>());

  auto inscope = program_options["inscope"].as<std::string>();
  auto outscope = program_options["outscope"].as<std::string>();
  auto rsb_cl = RsbClassificator(factory.create(config.first, config.second),
                                 inscope, outscope);

  rsc::misc::initSignalWaiter();
  return rsc::misc::waitForSignal();
}
