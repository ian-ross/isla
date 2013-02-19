#include <iostream>
#include <cmath>
#include "Grid.hh"

using namespace std;
using namespace netCDF;

bool fuzzeq(double x, double y) { return fabs(x - y) < 1E-6; }

int main(void)
{
  try {
    NcFile nc("test_grid.nc", NcFile::read);
    Grid file_grid(nc);
    cout << "nlat=" << file_grid.nlat()
         << "  nlon=" << file_grid.nlon() << endl;
    cout << "lats" << (file_grid.lats_reversed() ? " (R): " : ": ");
    for (int i = 0; i < file_grid.nlat(); ++i) cout << file_grid.lat(i) << " ";
    cout << endl;
    cout << "lons" << (file_grid.lons_reversed() ? " (R): " : ": ");
    for (int i = 0; i < file_grid.nlon(); ++i) cout << file_grid.lon(i) << " ";
    cout << endl;
    assert(file_grid.nlat() == 144);
    assert(file_grid.nlon() == 288);
    assert(fuzzeq(file_grid.lon(0), 0.0));
    assert(fuzzeq(file_grid.lon(1) - file_grid.lon(0), 1.25));
    assert(fuzzeq(file_grid.lat(0), -89.375));
    assert(fuzzeq(file_grid.lat(1) - file_grid.lat(0), 1.25));
  } catch (exception &e) {
    cout << "EXCEPTION: " << e.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
