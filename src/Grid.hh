#ifndef _H_GRID_
#define _H_GRID_

#include <vector>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <ncFile.h>

class Grid {
public:
  Grid(netCDF::NcFile &infile);
  Grid(int nlat, double lat0, double dlat, int nlon, double lon0, double dlon);
  Grid(std::vector<double> inlat, int nlon, double lon0, double dlon);
  Grid(const Grid &other) : lt(other.lt), ln(other.ln) { }

  int nlat(void) const { return lt.size(); }
  int nlon(void) const { return ln.size(); }
  double cellArea(int r, int c);
  bool lats_reversed(void) const { return lt_rev; }
  bool lons_reversed(void) const { return ln_rev; }
  const std::vector<double> lats(void) const { return lt; }
  const std::vector<double> lons(void) const { return ln; }
  double lat(unsigned int i) const {
    if (i < nlat())
      return lt[i];
    else
      throw std::out_of_range("latitude index out of range in Grid");
  }
  double lon(unsigned int i) const {
    if (i < nlon())
      return ln[i];
    else
      throw std::out_of_range("longitude index out of range in Grid");
  }

private:
  std::vector<double> lt, ln;
  bool lt_rev, ln_rev;
};

typedef boost::shared_ptr<Grid> GridPtr;

#endif
