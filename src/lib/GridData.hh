#ifndef _H_GRIDDATA_
#define _H_GRIDDATA_

#include <vector>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <ncFile.h>

class Grid {
public:
  Grid(netCDF::NcFile &infile);
  Grid(int nlat, double lat0, double dlat, int nlon, double lon0, double dlon);
  Grid(const Grid &other);

  int nlat(void) const { return _nlat; }
  int nlon(void) const { return _nlon; }
  double lat0(void) const { return _lat0; }
  double lon0(void) const { return _lon0; }
  double dlat(void) const { return _dlat; }
  double dlon(void) const { return _dlon; }
  double lat(unsigned int i) const {
    if (i < _nlat)
      return _lats[i];
    else
      throw std::out_of_range("latitude index out of range in Grid");
  }
  double lon(unsigned int i) const {
    if (i < _nlon)
      return _lons[i];
    else
      throw std::out_of_range("longitude index out of range in Grid");
  }
  bool lats_reversed(void) const { return _lats_reversed; }
  bool lons_reversed(void) const { return _lons_reversed; }

private:
  static void reverse(std::vector<double> &xs);

  int _nlat, _nlon;
  double _lat0, _lon0;
  double _dlat, _dlon;
  std::vector<double> _lats, _lons;
  bool _lats_reversed, _lons_reversed;
};

typedef boost::shared_ptr<Grid> GridPtr;

#endif
