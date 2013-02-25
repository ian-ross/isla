#include <cmath>
#include <algorithm>
#include "Grid.hh"
#include "ncDim.h"
#include "ncVar.h"

using namespace std;
using namespace netCDF;

Grid::Grid(NcFile &infile)
{
  // Extract dimension and variable information.
  multimap<string, NcDim> dims = infile.getDims();
  multimap<string, NcVar> vars = infile.getVars();

  // Check dimension names.
  string latname = "", lonname = "";
  if (dims.find("lat") != dims.end())      latname = "lat";
  if (dims.find("latitude") != dims.end()) latname = "latitude";
  if (dims.find("lon") != dims.end())       lonname = "lon";
  if (dims.find("longitude") != dims.end()) lonname = "longitude";
  if (latname == "" || lonname == "")
    throw domain_error("NetCDF file needs latitude and longitude dimension");

  // Check coordinate variable names.
  if (vars.find(latname) == vars.end() || vars.find(lonname) == vars.end())
    throw domain_error("netCDF file lacks appropriate coordinate variables");

  // Assume: latitude is degrees north, longitude degrees east.
  NcDim latdim = dims.find(latname)->second;
  NcVar latvar = vars.find(latname)->second;
  NcDim londim = dims.find(lonname)->second;
  NcVar lonvar = vars.find(lonname)->second;
  unsigned int nlat = latdim.getSize(), nlon = londim.getSize();

  // Read latitude and longitude variables.
  _lats_reversed = _lons_reversed = false;
  _lats.resize(nlat);  latvar.getVar(_lats.data());
  if (_lats[0] > _lats[1]) {
    reverse(_lats.begin(), _lats.end());
    _lats_reversed = true;
  }
  _lons.resize(nlon);  lonvar.getVar(_lons.data());
  if (_lons[0] > _lons[1]) {
    reverse(_lons.begin(), _lons.end());
    _lons_reversed = true;
  }
}

Grid::Grid(int nlat, double lat0, double dlat,
           int nlon, double lon0, double dlon) :
  _lats(nlat), _lons(nlon)
{
  for (int i = 0; i < nlat; ++i) _lats[i] = lat0 + i * dlat;
  for (int i = 0; i < nlon; ++i) _lons[i] = lon0 + i * dlon;
  if (_lats[0] > _lats[1]) {
    reverse(_lats.begin(), _lats.end());
    _lats_reversed = true;
  }
  if (_lons[0] > _lons[1]) {
    reverse(_lons.begin(), _lons.end());
    _lons_reversed = true;
  }
}

Grid::Grid(const Grid &other) :
  _lats(other._lats), _lons(other._lons)
{ }


// Radius of Earth in km.
const double REARTH = 6370.0;

double Grid::cellArea(int r, int c)
{
  double lat = _lats[r], lon = _lons[c];
  double dlon = _lons[1] - _lons[0], dlat;
  if (r == 0)
    dlat = (_lats[1] - _lats[0]) / 2 + (_lats[0] - (-90.0));
  else if (r == nlat() - 1)
    dlat = (_lats[nlat()-1] - _lats[nlat()-2]) / 2 + (90.0 - _lats[nlat()-1]);
  else dlat = (_lats[r + 1] - _lats[r - 1]) / 2;
  double dphi = dlon / 180.0 * M_PI, dtheta = dlat / 180.0 * M_PI;
  double theta = (90 - lat) / 180.0 * M_PI;
  return REARTH * REARTH * sin(theta) * dtheta * dphi;
}
