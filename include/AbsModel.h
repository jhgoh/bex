#ifndef AbsModel_H
#define AbsModel_H

#include <fstream>

#include "include/ConfigReader.h"
#include "include/PDFInterface.h"
#include "include/Random.h"

class AbsModel
{
public:
  AbsModel(const ConfigReader& cfg);
  virtual ~AbsModel();

  struct FormFactorType { enum X { YOSHINO, FIOP, END }; };
  struct MassLossType { enum X { YOSHINO, CONST, END }; };

  void calculateCrossSection();
  virtual double calculatePartonWeight(const double m, const PDF& pdf1, const PDF& pdf2) = 0;
  double getCrossSection();
  double getCrossSectionError();
  double getWeightMax() { return weightMax_; };

  virtual void beginJob();
  virtual void endJob();
  virtual void event();

protected:
  void loadYoshinoDataTable();

protected:
  bool isValid_;
  std::string name_;

  std::ofstream fout_;

  ConfigReader cfg_;
  Random* rnd_;
  PDFInterface* pdf_;

  int beamId1_, beamId2_;
  int nDim_;
  double beamEnergy_;
  double massMin_, massMax_;
  double mD_;

  double weightMax_;
  double xsec_, xsecErr_;

  const static int nXsecIter_ = 100000;

  // Cached variables for convenience
  int formFactorType_, mLossType_;
  double s_;
  double formFactor_;
  std::vector<std::pair<double, double> > mLossTab_;
};

#endif

