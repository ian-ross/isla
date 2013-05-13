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
  lt_rev = ln_rev = false;
  lt.resize(nlat);  latvar.getVar(lt.data());
  if (lt[0] > lt[1]) {
    reverse(lt.begin(), lt.end());
    lt_rev = true;
  }
  ln.resize(nlon);  lonvar.getVar(ln.data());
  if (ln[0] > ln[1]) {
    reverse(ln.begin(), ln.end());
    ln_rev = true;
  }
}

Grid::Grid(int nlat, double lat0, double dlat,
           int nlon, double lon0, double dlon) :
  lt(nlat), ln(nlon)
{
  for (int i = 0; i < nlat; ++i) lt[i] = lat0 + i * dlat;
  for (int i = 0; i < nlon; ++i) ln[i] = lon0 + i * dlon;
  if (lt[0] > lt[1]) { reverse(lt.begin(), lt.end()); lt_rev = true; }
  if (ln[0] > ln[1]) { reverse(ln.begin(), ln.end()); ln_rev = true; }
}

Grid::Grid(vector<double> lats, int nlon, double lon0, double dlon) :
  lt(lats), ln(nlon)
{
  for (int i = 0; i < nlon; ++i) ln[i] = lon0 + i * dlon;
  if (lt[0] > lt[1]) { reverse(lt.begin(), lt.end()); lt_rev = true; }
  if (ln[0] > ln[1]) { reverse(ln.begin(), ln.end()); ln_rev = true; }
}


// Radius of Earth in km.
const double REARTH = 6370.0;

double Grid::cellArea(unsigned int r, unsigned int c)
{
  double lat = lt[r];
  double dlon = ln[1] - ln[0], dlat;
  if (r == 0)
    dlat = (lt[1] - lt[0]) / 2 + (lt[0] - (-90.0));
  else if (r == nlat() - 1)
    dlat = (lt[nlat()-1] - lt[nlat()-2]) / 2 + (90.0 - lt[nlat()-1]);
  else dlat = (lt[r + 1] - lt[r - 1]) / 2;
  double dphi = dlon / 180.0 * M_PI, dtheta = dlat / 180.0 * M_PI;
  double theta = (90 - lat) / 180.0 * M_PI;
  return REARTH * REARTH * sin(theta) * dtheta * dphi;
}
