#include <cmath>
#include "GridData.hh"
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
    throw domain_error("netCDF file needs a latitude and a longitude dimension");

  // Check coordinate variable names.
  if (vars.find(latname) == vars.end() || vars.find(lonname) == vars.end())
    throw domain_error("netCDF file lacks appropriate coordinate variables");

  // Assume: latitude is degrees north, longitude degrees east.
  NcDim latdim = dims.find(latname)->second;
  NcVar latvar = vars.find(latname)->second;
  NcDim londim = dims.find(lonname)->second;
  NcVar lonvar = vars.find(lonname)->second;
  _nlat = latdim.getSize();
  _nlon = londim.getSize();

  // Read latitude and longitude variables.
  _lats_reversed = _lons_reversed = false;
  _lats.resize(_nlat);  latvar.getVar(_lats.data());
  if (_lats[0] > _lats[1]) { reverse(_lats); _lats_reversed = true; }
  _lat0 = _lats[0];     _dlat = _lats[1] - _lats[0];
  _lons.resize(_nlon);  lonvar.getVar(_lons.data());
  if (_lons[0] > _lons[1]) { reverse(_lons); _lons_reversed = true; }
  _lon0 = _lons[0];     _dlon = _lons[1] - _lons[0];

  // Check for even spacing.
  for (int i = 1; i < _nlat; ++i)
    if (fabs((_lats[i] - _lats[i-1]) - _dlat) > 1.0E-6)
      throw domain_error("netCDF latitude variable is not evenly spaced");
  for (int i = 1; i < _nlon; ++i)
    if (fabs((_lons[i] - _lons[i-1]) - _dlon) > 1.0E-6)
      throw domain_error("netCDF longitude variable is not evenly spaced");
}

Grid::Grid(int inlat, double ilat0, double idlat,
           int inlon, double ilon0, double idlon) :
  _nlat(inlat), _lat0(ilat0), _dlat(idlat),
  _nlon(inlon), _lon0(ilon0), _dlon(idlon),
  _lats(inlat), _lons(inlon)
{
  for (int i = 0; i < _nlat; ++i) _lats[i] = _lat0 + i * _dlat;
  for (int i = 0; i < _nlon; ++i) _lons[i] = _lon0 + i * _dlon;
}

Grid::Grid(const Grid &other) :
  _nlat(other._nlat), _lat0(other._lat0), _dlat(other._dlat),
  _nlon(other._nlon), _lon0(other._lon0), _dlon(other._dlon),
  _lats(other._lats), _lons(other._lons)
{ }

void Grid::reverse(vector<double> &xs)
{
  double tmp;
  int n = xs.size();
  for (int i = 0; i < n / 2; ++i) {
    tmp = xs[i];
    xs[i] = xs[n-1-i];
    xs[n-1-i] = tmp;
  }
}
