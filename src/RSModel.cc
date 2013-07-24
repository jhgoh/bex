#include "include/RSModel.h"
#include "include/Utility.h"

#include <iostream>

using namespace std;
using physics::Pi;

RSModel::RSModel(const ConfigReader& cfg):
  AbsModel(cfg)
{
  name_ += ":RSModel";

  if ( nDim_ != 5 )
  {
    cerr << "!! RSModel: We do not support RS Blackhole model at " <<  nDim_ << " dimension\n";
    cerr << "!!          Changing dimension to 5D\n";
    nDim_ = 5;
    cfg_.set("dimension", nDim_);

    kn_ = physics::kn[nDim_-4];
    kn2_ = kn_*kn_;

    if ( mLossType_ == MassLossType::YOSHINO )
    {
      loadYoshinoDataTable(); // Reload mass loss data table since nDim_ is altered
    }

    if ( formFactorType_ == FormFactorType::YOSHINO )
    {
      // Same code in AbsModel.cc
      std::vector<double> maxBValues;
      std::ifstream fin("data/yoshino/max_b.data");
      fin >> maxBValues;
      bMax_ = physics::r0ToRs(nDim_, maxBValues[nDim_-4]);
    }
    else if ( formFactorType_ == FormFactorType::FIOP )
    {
      bMax_ = 2.*pow(1.+(nDim_-2.)*(nDim_-2.)/4., -1./(nDim_-3.));
    }
    kn_ = physics::kn[nDim_-4];
    kn2_ = kn_*kn_;
    formFactor_ = kn2_*physics::Pi*bMax_*bMax_;
  }

  prodWeights_ = cfg_.get<std::vector<double> >("prodWeights");
  // wGG = C_gg
  rs_wGG_ = prodWeights_[0];
  // wBG = C_gQ + C_gd
  rs_wBG_ = prodWeights_[2] + prodWeights_[8];
  // wBB = C_QQ + C_Qd + C_dd
  rs_wBB_ = prodWeights_[1] + prodWeights_[7] + prodWeights_[6];

}

double RSModel::calculatePartonWeight(const double m, const PDF& pdf1, const PDF& pdf2)
{
  const double rssqr = m/mD_/mD_/mD_; // We will do 5D only
  const double u = m*m/s_;
  const double weightParton = rssqr*(1./massMax_-1./massMin_)*m*u*log(u);

  // Consider suppression factor due to profile in extra dimension
  // Assume partons are not polarized; 50% left and 50% right handed
  double weightProfile = 0;
  weightProfile += rs_wGG_*pdf1(21)*pdf2(21); // gluon+gluon
  weightProfile += 2*rs_wBG_*(pdf1(21)*pdf2(5) + pdf1(5)*pdf2(21)); // gluon+bottom (and bbar)
  weightProfile += 4*rs_wBB_*pdf1(5)*pdf2(5); // bottom+bottom (and bbar)

  return weightParton*weightProfile;
}

void RSModel::selectParton(const PDF& pdf1, const PDF& pdf2, Particle& parton1, Particle& parton2)
{
  double weights[] = {
    rs_wGG_*pdf1(21)*pdf2(21),
    2*rs_wBG_*(pdf1(21)*pdf2(5) + pdf1(5)*pdf2(21)),
    4*rs_wBB_*pdf1(5)*pdf2(5)
  };
}
