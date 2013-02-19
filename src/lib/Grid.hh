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
  Grid(const Grid &other);

  int nlat(void) const { return _lats.size(); }
  int nlon(void) const { return _lons.size(); }
  bool lats_reversed(void) const { return _lats_reversed; }
  bool lons_reversed(void) const { return _lons_reversed; }
  double lat(unsigned int i) const {
    if (i < nlat())
      return _lats[i];
    else
      throw std::out_of_range("latitude index out of range in Grid");
  }
  double lon(unsigned int i) const {
    if (i < nlon())
      return _lons[i];
    else
      throw std::out_of_range("longitude index out of range in Grid");
  }

private:
  std::vector<double> _lats, _lons;
  bool _lats_reversed, _lons_reversed;
};

typedef boost::shared_ptr<Grid> GridPtr;

#endif
