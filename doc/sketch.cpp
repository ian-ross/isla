#include <vector>
#include <numeric>
#include <string>
#include <boost/shared_ptr.hpp>
#include <netcdf.hh>

class Grid {
public:
  Grid(NcFile &infile);
  Grid(int nlat, double lat0, double dlat, int nlon, double lon0, double dlon);
  Grid(const Grid &other);

private:
  int nlat, nlon;
  double lat0, lon0;
  double dlat, dlon;
  std::vector<double> lats, lons;
};

typedef boost::shared_ptr<Grid> GridPtr;


template<typename T> class GridData {
public:
  GridData(GridPtr grid, T defval);
  GridData(NcFile &infile, std::string ncvar);
  GridData(const GridData &other);
  template<typename Functor> GridData(const GridData &other, Functor &process);
  ~GridData();

  // Data accessors.
  const T &operator() const(int row, int col);
  T &operator()(int row, int col);

  // Regrid data to a new resolution.
  void regrid(GridData<T> &to);

  // Process grid data in place: F => T fn(const T &x).
  template<typename F> void proc(F &fn);

  // Process grid data into new object: F => R fn(const T &x).
  template<typename R, typename F> void process(GridData<R> &to, F &fn);

  // Process grid data into new object of different resolution using
  // weighting: F => R fn(const std::vector<T> &xs, const std::vector<double> &ws).
  template<typename R, typename F> void wproc(GridData<R> &to, F &fn);

private:
  GridPtr grid;
  std::vector<T> data;
};


// Functor to offset values by a given offset.

struct Offset: public unary_function<double, double> {
  Offset(double indx) : dx(indx) { }
  double dx;
  double operator()(double x) { return x + off; }
};


// Count model levels to turn a water depth value into an integer
// depth mask.

struct LevelCounter: public unary_function<double, int> {
  LevelCounter(const std::vector<double> &levels) { }
  std::vector<double> levels;
  int operator()(double x) { }
};


// Count model levels to turn a water depth value into an integer
// depth mask.

struct FractionToMask: public unary_function<double, bool> {
  FractionToMask(double inthresh) : thresh(inthresh) { }
  double thresh;
  bool operator()(double x) { return x > thresh }
};


// Calculate weighted average of a vector of data given a vector of
// weights.

double weightedAverage(const std::vector<double> &xs,
                       const std::vector<double> &ws)
{
  double x = std::inner_product(xs.begin(), xs.end(), ws.begin(), 0.0);
  double w = std::accumulate(ws.begin(), ws.end(), 0.0);
  return x / w;
}


// Calculate land fraction within a model grid cell.

double landFraction(const std::vector<double> &xs,
                    const std::vector<double> &ws)
{
  double land = 0.0, total = 0.0;
  for (int i = 0; i < xs.size(); ++i) {
    if (xs[i] >= 0) land += ws[i];
    total += ws[i];
  }
  return land / total;
}


typedef std::pair<int, int> Pos;
typedef std::vector<Pos> Region;

struct Segment {
  int srow, erow;
  int scol, ecol;
};

class Island {
public:
  Island(std::string name, const Region &region);

  // Methods to add and remove points from region, triggering segment
  // recalculation.

  // Methods to access segment list.

private:
  std::string name;
  Region region;
  std::vector<Segment> segments;
};


// Calculate regions from mask -- flood fill algorithm.

void regions(const GridData<bool> &mask, std::vector<Region> &regions)
{
  GridData<bool> visited(mask.grid(), false);
  GridData<int> regionidx(mask.grid(), -1);

}


// Filter regions -- any larger than threshold are dropped.
void filterRegions(...)
{

}



int main(void)
{
  // Read input bathymetry.
  NcFile nc("in.nc");
  GridData<double> inbathy(nc, "bathy");

  // Offset input bathymetry by sea level offset.
  inbathy.process(Offset(SEA_LEVEL_OFFSET));

  // Regrid input bathymetry to model grid.
  GridPtr mgrid(new Grid(144, -89.375, 1.25, 288, 0.000, 1.25));
  GridData mbathy(mgrid, 0.0);
  inbathy.process(mbathy, weightedAverage);

  // Calculate model grid depthmask.
  GridData<int> mdepthmask(mgrid, 0);
  mbathy.process(mdepthmask, LevelCounter(MODEL_LEVELS));

  // Calculate model grid land fraction.
  GridData<double> mlandfrac(mgrid, 0.0);
  inbathy.process(mlandfrac, landFraction);

  // Calculate model land/sea mask.
  GridData<bool> mmask(mgrid, false);
  mlandfrac.process(mmask, FractionToMask(LAND_FRACTION_THRESHOLD));

  // Calculate regions and filter out any too large to be islands.
  std::vector<Region> regs;
  regions(mmask, regs);
  filterRegions(regions, MAX_ISLAND_SIZE);

  // Build islands.
  std::vector<Island> islands;
  for (std::vector<Region>::iterator it = regs.begin(); it != regs.end(); ++it)
    islands.push(Island(*it));
}
