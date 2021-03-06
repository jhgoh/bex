#include "include/Random.h"
#include "include/Utility.h"

#include <cmath>

using namespace std;

Random::Random(const unsigned int seed)
{
  rnd_.seed(seed);
  rnd_min_ = rnd_.min();
  rnd_width_ = rnd_.max()-rnd_min_;
}

double Random::shift(const double x, const double min, const double max) const
{
  return x*(max-min) + min;
}

double Random::uniform(const double min, const double max)
{
  return shift(rand(), min, max);
}

double Random::ramp(const double min, const double max)
{
  return shift(sqrt(rand()), min, max);
}

void Random::sphere(const double r, double& x, double& y, double& z)
{
  double rr = 1e9;
  while ( rr > 1 or rr < 0.01 )
  {
    x = rand();
    y = rand();
    z = rand();
    rr = x*x + y*y + z*z;
  }
  const double scale = r/sqrt(rr);
  x *= scale;
  y *= scale;
  z *= scale;
}

int Random::pick(const int min, const int max)
{
  return int(rand()*(max+1-min));
}

int Random::pickFromCHist(const std::vector<double>& v)
{
  // Check validity of CDF : is there any phase space?
  if ( v.back() <= 0 )
  {
    std::cerr << "Invalid CDF, empty phase space\n";
    return -1;
  }
  // Check validity of CDF : is it monolothic array?
  for ( int i=0, n=v.size()-1; i<n; ++i )
  {
    if ( v[i] > v[i+1] )
    {
      std::cerr << "Invalid CDF, array is not monolothic\n";
      return -1;
    }
  }

  const double x = uniform(0, v.back());

  return findNearest(x, v);
}

int Random::pickFromHist(const std::vector<double>& v)
{
  std::vector<double> cdf(v.size()+1);
  cdf[0] = 0;
  // Make it CDF
  for ( int i=0, n=v.size(); i<n; ++i )
  {
    cdf[i+1] = cdf[i]+v[i];
  }
  return pickFromCHist(cdf);
}

double Random::curve(const std::vector<std::pair<double, double> >& points, const double xmin, const double xmax)
{
  // Make CDF
  // FIXME : xmin is not applied. to be implemented
  // FIXME : xmax is not true xmax. to be updated
  std::vector<double> cdf(1);
  cdf.reserve(points.size());
  cdf[0] = 0;
  double sumArea = 0;
  for ( int i=1, n=points.size(); i<n; ++i )
  {
    const double x0 = points[i-1].first;
    const double y0 = max(0., points[i-1].second);
    const double x1 = points[i].first;
    const double y1 = max(0., points[i].second);
    const double dx = x1-x0;
    if ( x1 > xmax )
    {
      const double dxAtEnd = xmax-x0;
      const double yAtEnd = dxAtEnd/dx*(y1-y0)+y0;
      const double area = dxAtEnd*(yAtEnd+y0)/2;
      sumArea += area;
      cdf.push_back(sumArea);
      break;
    }

    const double area = dx*(y1+y0)/2;
    sumArea += area;
    cdf.push_back(sumArea);
  }

  // Generate by inverse method
  const double y = uniform(0, sumArea);
  const size_t index = findNearest(y, cdf);

  const double x0 = points[index].first;
  const double x1 = points[index+1].first;
  const double y0 = cdf[index];
  const double y1 = cdf[index+1];
  const double dy = y1-y0;

  // Special case if zero prob. in this range
  if ( std::abs(dy) < 1e-12  ) return x0;
  const double invSlope = (x1-x0)/dy;

  return invSlope*(y-y0) + x0;

}

