#ifndef _H_GRIDDATA_
#define _H_GRIDDATA_

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>
#include "ncFile.h"
#include "ncVar.h"
#include "ncVarAtt.h"
#include "ncDim.h"
#include "ncType.h"

#include "Grid.hh"

template<class T> class GridData {
public:
  GridData(GridPtr grid, T def) :
    _g(grid), _data(grid->nlat() * grid->nlon(), def), _has_missing(false) { }
  GridData(GridPtr grid, netCDF::NcFile &infile, std::string ncvar);
  GridData(const GridData &other) :
    _g(other._g), _data(other._data),
    _has_missing(other._has_missing), _missing_val(other._missing_val) { }
  template<typename F> GridData(const GridData &other, F fn) :
    _g(other._g), _data(other._data.size()),
    _has_missing(other._has_missing), _missing_val(other._missing_val) {
    other.process(*this, fn);
  }
  ~GridData() { }
  const GridData &operator=(const GridData &other) {
    if (this != &other) {
      _g = other._g;
      _data = other._data;
      _has_missing = other._has_missing;
      _missing_val = other._missing_val;
    }
    return *this;
  }
  const GridData &operator=(T val) {
    fill(_data.begin(), _data.end(), val);
    return *this;
  }

  // Access grid.
  GridPtr grid(void) const { return _g; }

  // Access data as flat vector.
  std::vector<T> &data(void) { return _data; }
  const std::vector<T> &data(void) const { return _data; }

  // Check for missing value.
  bool is_missing(T val) const { return _has_missing && val == _missing_val; }

  // Data accessors.
  typename std::vector<T>::const_reference operator()(int r, int c) const {
    return _data[r * _g->nlon() + c];
  }
  typename std::vector<T>::reference operator()(int r, int c) {
    return _data[r * _g->nlon() + c];
  }

  // Process grid data in place: F => T fn(const T &x)
  template<typename F> void process(F fn) {
    std::transform(_data.begin(), _data.end(), _data.begin(), fn);
  }

  // Process grid data into new object: F => R fn(const T &x)
  template<typename R, typename F> void process(GridData<R> &to, F fn) {
    if (_g != to.grid())
      throw std::domain_error("grid mismatch in GridData::process");
    std::transform(_data.begin(), _data.end(), to.data().begin(), fn);
  }

  // Missing data detection function class.
  class IsMissing {
  public:
    IsMissing(GridData<T> &d) : _d(d) { }
    bool operator()(T v) { return _d.is_missing(v); }
  private:
    GridData<T> &_d;
  };

  // Type conversion utilities.
  template<typename F> class Convert {
  public:
    Convert() { }
    T operator()(F v) { return static_cast<T>(v); }
  };
  template<typename F>
  static T convert(const std::vector<F> &in, std::vector<T> &out) {
    std::transform(in.begin(), in.end(), out.begin(), Convert<F>());
  }

private:
  GridPtr _g;
  std::vector<T> _data;
  bool _has_missing;
  T _missing_val;
};


// Constructor to read data from a NetCDF file.

template<typename T> GridData<T>::GridData
(GridPtr grid, netCDF::NcFile &infile, std::string ncvar) :
  _g(grid), _data(_g->nlon() * _g->nlat()), _has_missing(false)
{
  // Find variable definition.
  if (infile.getVars().find(ncvar) == infile.getVars().end())
    throw std::domain_error(std::string("NetCDF variable name '") +
                            ncvar + " not found");
  netCDF::NcVar var = infile.getVars().find(ncvar)->second;

  // Check variable definition.
  int nlat = grid->nlat(), nlon = grid->nlon();
  if (var.getDimCount() != 2 ||
      (var.getDim(0).getName() != "lat" &&
       var.getDim(0).getName() != "latitude") ||
      var.getDim(0).getSize() != nlat ||
      (var.getDim(1).getName() != "lon" &&
       var.getDim(1).getName() != "longitude") ||
      var.getDim(1).getSize() != nlon)
    throw std::domain_error(std::string("NetCDF variable '") +
                            ncvar + "' is not on lat/lon grid");

  // Read data in one go at original type and cast to output type.
  int n = nlat * nlon;
  switch (var.getType().getId()) {
  case netCDF::NcType::nc_BYTE:
  case netCDF::NcType::nc_CHAR: {
    std::vector<char> tdata(n);  var.getVar(tdata.data());
    convert(tdata, _data);  break;
  }
  case netCDF::NcType::nc_SHORT: {
    std::vector<short> tdata(n);  var.getVar(tdata.data());
    convert(tdata, _data);  break;
  }
  case netCDF::NcType::nc_INT: {
    std::vector<int> tdata(n);  var.getVar(tdata.data());
    convert(tdata, _data);  break;
  }
  case netCDF::NcType::nc_FLOAT: {
    std::vector<float> tdata(n);  var.getVar(tdata.data());
    convert(tdata, _data);  break;
  }
  case netCDF::NcType::nc_DOUBLE: {
    std::vector<double> tdata(n);  var.getVar(tdata.data());
    convert(tdata, _data);  break;
  }
  case netCDF::NcType::nc_UBYTE: {
    std::vector<unsigned char> tdata(n);  var.getVar(tdata.data());
    convert(tdata, _data);  break;
  }
  case netCDF::NcType::nc_USHORT: {
    std::vector<unsigned short> tdata(n);  var.getVar(tdata.data());
    convert(tdata, _data);  break;
  }
  case netCDF::NcType::nc_UINT: {
    std::vector<unsigned int> tdata(n);  var.getVar(tdata.data());
    convert(tdata, _data);  break;
  }
  default:
    throw std::domain_error("Invalid NetCDF type for grid data");
  }

  // Reverse coordinates if required.
  T tmp;
  if (grid->lats_reversed()) {
    for (int r = 0; r < nlat / 2; ++r)
      for (int c = 0; c < nlon; ++c) {
        tmp = (*this)(r, c);
        (*this)(r, c) = (*this)(nlat - r - 1, c);
        (*this)(nlat - r - 1, c) = tmp;
      }
  }
  if (grid->lons_reversed()) {
    for (int r = 0; r < nlat; ++r)
      for (int c = 0; c < nlon / 2; ++c) {
        tmp = (*this)(r, c);
        (*this)(r, c) = (*this)(r, nlon - c - 1);
        (*this)(r, nlon - c - 1) = tmp;
      }
  }

  // Deal with missing values.
  std::map<std::string, netCDF::NcVarAtt> atts = var.getAtts();
  netCDF::NcVarAtt missing_att;
  if (atts.find("missing_value") != atts.end())
    missing_att = atts.find("missing_value")->second;
  else if (atts.find("_FillValue") != atts.end())
    missing_att = atts.find("_FillValue")->second;
  if (!missing_att.isNull()) {
    _has_missing = true;
    int n = missing_att.getAttLength();
    if (n != 1)
      throw std::domain_error
        (std::string("Invalid NetCDF missing value for variable '") +
         ncvar + "'");
    switch (missing_att.getType().getId()) {
    case netCDF::NcType::nc_BYTE:
    case netCDF::NcType::nc_CHAR: {
      char mval;  missing_att.getValues(&mval);
      _missing_val = Convert<char>()(mval);  break;
    }
    case netCDF::NcType::nc_SHORT: {
      short mval;  missing_att.getValues(&mval);
      _missing_val = Convert<short>()(mval);  break;
    }
    case netCDF::NcType::nc_INT: {
      int mval;  missing_att.getValues(&mval);
      _missing_val = Convert<int>()(mval);  break;
    }
    case netCDF::NcType::nc_FLOAT: {
      float mval;  missing_att.getValues(&mval);
      _missing_val = Convert<float>()(mval);  break;
    }
    case netCDF::NcType::nc_DOUBLE: {
      double mval;  missing_att.getValues(&mval);
      _missing_val = Convert<double>()(mval);  break;
    }
    case netCDF::NcType::nc_UBYTE: {
      unsigned char mval;  missing_att.getValues(&mval);
      _missing_val = Convert<unsigned char>()(mval);  break;
    }
    case netCDF::NcType::nc_USHORT: {
      unsigned short mval;  missing_att.getValues(&mval);
      _missing_val = Convert<unsigned short>()(mval);  break;
    }
    case netCDF::NcType::nc_UINT: {
      unsigned int mval;  missing_att.getValues(&mval);
      _missing_val = Convert<unsigned int>()(mval);  break;
    }
    default:
      throw std::domain_error("Invalid NetCDF type for grid data");
    }
  }
}

#endif
