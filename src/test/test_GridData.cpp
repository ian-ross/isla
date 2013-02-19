#include <iostream>
#include <cmath>
#include "GridData.hh"

using namespace std;
using namespace netCDF;

bool fuzzeq(double x, double y) { return fabs(x - y) < 1E-6; }

bool is_point_land(int depth) { return depth < 0; }

int main(void)
{
  try {
    NcFile nc("test_grid.nc", NcFile::read);
    GridPtr gr(new Grid(nc));

    // Read floating point data.
    GridData<double> bath(gr, nc, "bathorig");

    // Read integer data stored as floating point (implicit casting).
    GridData<int> depthmask(gr, nc, "depthmask");

    // Calculate model land/sea mask.
    GridData<bool> mask(gr, false);
    depthmask.process(mask, GridData<int>::IsMissing(depthmask));

    // Check count of land points two ways.
    int ndepth = 0, nmask = 0;
    for (int r = 0; r < gr->nlat(); ++r)
      for (int c = 0; c < gr->nlon(); ++c) {
        if (mask(r, c)) ++nmask;
        if (depthmask.is_missing(depthmask(r, c))) ++ndepth;
      }
    cout << "nlat=" << gr->nlat() << " nlon=" << gr->nlon()
         << " points=" << gr->nlat() * gr->nlon() << endl;
    cout << "ndepth=" << ndepth << " nmask=" << nmask << endl;
    assert(ndepth == nmask);
  } catch (exception &e) {
    cout << "EXCEPTION: " << e.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
