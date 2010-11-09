WORKING=/devel/devel/Working
CC=gcc
CFLAGS=-c -fPIC -Wall -Wfatal-errors -g \
-I/home/johnu/src/include \
-I/home/johnu/src/CHOLMOD/Include \
-I/home/johnu/mosek/6/tools/platform/linux64x86/h
CXXFLAGS=-c -fPIC -Wall -g
LD=gcc
LDFLAGS=        -L. -L/home/johnu/src/lib \
	-L/home/johnu/mosek/6/tools/platform/linux64x86/bin \
	-L$(WORKING) \
	-lpymparserd -lpymoptimizerd -lpymrsd \
	-lconfig -lmosek64 -lpthread \
	-lm -lcholmod `pkg-config --libs x11 xext` -lGL -lGLU -lglut \
	-lpthread -lGLEW
OBJS=FiberEffectImp.o FiberEffectImpAll.o csparse.o FiberMain.o
TARGETS=$(WORKING)/rsnatd \
$(WORKING)/rsfltkd \
$(WORKING)/libpymoptimizerd.so \
$(WORKING)/libpymparserd.so \
$(WORKING)/libpymrsd.so
LIBS=-lm

OBJS_libpymparser=PymCmdLineParser.o TrajParser.o

OBJS_libpymoptimizer_c=PymBiped.o \
ExpBodyMoEq_real.o \
MathUtil.o \
MuscleFiber.o \
Optimize.o \
PymJointAnchor.o \
PymPodUtil.o \
PymuscleConfig.o \
RigidBody.o \
StateDependents.o \
SymbolicTensor.o \
ToSparse.o \
TripletMatrix.o \
dRdv_real.o \
fibDiffW.o
OBJS_libpymoptimizer_cxx=ConvexHull.o ConvexHullCapi.o

OBJS_libpymrs_c= image.o model.o quaternion.o pymrscore.o pymrsrender.o pymrsphysics.o
OBJS_libpymrs_cxx=src/PrsGraph.o src/PrsGraphCapi.o src/PrsGraphData.o

OBJS_rsnat_c=PymRs.o PhysicsThreadMain.o
OBJS_rsnat_cxx=

OBJS_rsfltk_c=
OBJS_rsfltk_cxx=rsfltk.o

all: $(TARGETS)

$(WORKING)/libpymparserd.so: $(OBJS_libpymparser)
	gcc -shared -Wl,-soname,libpymparserd.so.1 \
        -o libpymparserd.so.1.0.1 $(OBJS_libpymparser)
	mv libpymparserd.so.1.0.1 $(WORKING)
	-ln -s $(WORKING)/libpymparserd.so.1.0.1 $(WORKING)/libpymparserd.so
	-ln -s $(WORKING)/libpymparserd.so.1.0.1 $(WORKING)/libpymparserd.so.1
	-ln -s $(WORKING)/libpymparserd.so.1.0.1 $(WORKING)/libpymparserd.so.1.0

$(WORKING)/libpymoptimizerd.so: $(OBJS_libpymoptimizer_c) \
$(OBJS_libpymoptimizer_cxx)
	g++ -shared -Wl,-soname,libpymoptimizerd.so.1 \
        -o libpymoptimizerd.so.1.0.1 $(OBJS_libpymoptimizer_c) \
	$(OBJS_libpymoptimizer_cxx)
	mv libpymoptimizerd.so.1.0.1 $(WORKING)
	-ln -s $(WORKING)/libpymoptimizerd.so.1.0.1 $(WORKING)/libpymoptimizerd.so
	-ln -s $(WORKING)/libpymoptimizerd.so.1.0.1 $(WORKING)/libpymoptimizerd.so.1
	-ln -s $(WORKING)/libpymoptimizerd.so.1.0.1 $(WORKING)/libpymoptimizerd.so.1.0

$(WORKING)/libpymrsd.so: $(OBJS_libpymrs_c) \
$(OBJS_libpymrs_cxx)
	g++ -shared -Wl,-soname,libpymrsd.so.1 \
        -o libpymrsd.so.1.0.1 $(OBJS_libpymrs_c) \
	$(OBJS_libpymrs_cxx)
	mv libpymrsd.so.1.0.1 $(WORKING)
	-ln -s $(WORKING)/libpymrsd.so.1.0.1 $(WORKING)/libpymrsd.so
	-ln -s $(WORKING)/libpymrsd.so.1.0.1 $(WORKING)/libpymrsd.so.1
	-ln -s $(WORKING)/libpymrsd.so.1.0.1 $(WORKING)/libpymrsd.so.1.0

$(WORKING)/rsnatd: $(OBJS_rsnat_c) $(OBJS_rsnat_cxx)\
$(WORKING)/libpymparserd.so \
$(WORKING)/libpymoptimizerd.so $(WORKING)/libpymrsd.so
	g++ $(LDFLAGS) $(OBJS_rsnat_c) $(OBJS_rsnat_cxx) -o rsnatd
	mv rsnatd $(WORKING)

$(WORKING)/rsfltkd: $(OBJS_rsfltk_c) $(OBJS_rsfltk_cxx) $(WORKING)/libpymparserd.so \
$(WORKING)/libpymoptimizerd.so $(WORKING)/libpymrsd.so
	g++ $(LDFLAGS) $(OBJS_rsfltk_c) $(OBJS_rsfltk_cxx) \
	`fltk-config --ldflags --cxxflags --use-gl` -o rsfltkd
	mv rsfltkd $(WORKING)


clean:
	-rm `find . -name "*.o"`
	-rm $(TARGETS)
	-rm $(WORKING)/libpym*

#.o: %.c %.cpp
#	g++ $(CFLAGS) $< -o $@
