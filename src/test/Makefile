BOOST_CXXFLAGS=
NETCDF_CXXFLAGS=
NETCDF_LDFLAGS=
LIBS=-lnetcdf_c++4 -lnetcdf

CXXFLAGS_RELEASE=-O2 -DISLA_DEBUG
CXXFLAGS_DEBUG=-g -DISLA_DEBUG
CXXFLAGS_PROFILE=-g -fprofile-arcs -ftest-coverage
CXXFLAGS=-I.. $(CXXFLAGS_RELEASE) $(BOOST_CXXFLAGS) $(NETCDF_CXXFLAGS)

LDFLAGS=$(NETCDF_LDFLAGS)

PROGS=test_Grid test_GridData test_LoadMask

OBJS=$(addprefix obj/,$(SRCS:.cpp=.o))

all: $(PROGS) run_tests

$(PROGS): ../obj/Grid.o

%: %.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(LIBS)

run_tests:
	@for p in $(PROGS) ; \
	do \
	  if ./$$p > /dev/null ; \
	  then \
	    echo $$p: OK ; \
	  else \
	    echo $$p: FAIL ; \
	  fi ; \
	done

clean:
	rm -f *.o $(PROGS)
