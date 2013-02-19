#include <iostream>
#include <cmath>
#include "GridData.hh"

using namespace std;
using namespace netCDF;

int main(void)
{
  try {
    NcFile nc("std_mask.nc", NcFile::read);
    GridPtr gr(new Grid(nc));

    // Read land/sea mask.
    GridData<bool> mask(gr, nc, "mask");

    // Check count of land points.
    int nmask = 0;
    for (int r = 0; r < gr->nlat(); ++r)
      for (int c = 0; c < gr->nlon(); ++c)
        if (mask(r, c)) ++nmask;
    cout << "nlat=" << gr->nlat() << " nlon=" << gr->nlon()
         << " points=" << gr->nlat() * gr->nlon() << endl;
    cout << " nmask=" << nmask << endl;
    assert(nmask == 14298);
  } catch (exception &e) {
    cout << "EXCEPTION: " << e.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
