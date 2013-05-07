BOOST_CXXFLAGS=
NETCDF_CXXFLAGS=
NETCDF_LDFLAGS=
LIBS=-lnetcdf_c++4 -lnetcdf

WX_CXXFLAGS=$(shell wx-config --cxxflags)
WX_LDFLAGS=$(shell wx-config --libs core,base,html)

CXXFLAGS_RELEASE=-O2 -DISLA_DEBUG
CXXFLAGS_DEBUG=-g -DISLA_DEBUG
CXXFLAGS_PROFILE=-g -fprofile-arcs -ftest-coverage
CXXFLAGS=$(CXXFLAGS_RELEASE) $(WX_CXXFLAGS) $(BOOST_CXXFLAGS) $(NETCDF_CXXFLAGS)

LDFLAGS=$(WX_LDFLAGS) $(NETCDF_LDFLAGS)

PROG=isla
HELPFILE=help/isla.htb

SRCS=isla.cpp \
     IslaFrame.cpp \
     IslaModel.cpp \
     IslaCompute.cpp \
     IslaCanvas.cpp \
     IslaPreferences.cpp \
     Dialogues.cpp \
     Grid.cpp

OBJS=$(addprefix obj/,$(SRCS:.cpp=.o))

all: obj $(PROG) help test

install: obj $(PROG) help
	mkdir -p ../install
	mkdir -p ../install/bin
	mkdir -p ../install/share/isla
	install $(PROG) ../install/bin
	install $(HELPFILE) ../install/share/isla

obj:
	if [ ! -d obj ]; then mkdir obj ; fi

isla: $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

.PHONY: test
test:
	cd test && $(MAKE)

.PHONY: help
help:
	cd help && $(MAKE)

depend:
	makedepend -Y -pobj/ -- $(CXXFLAGS) -- $(SRCS) 2> /dev/null

clean:
	rm -f docmgr $(OBJS)

obj/%.o: %.cpp
	$(COMPILE.cpp) -o $@ $<
