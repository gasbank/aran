CC=gcc
CFLAGS=-c -fPIC -Wall -Wfatal-errors -g \
-I/home/johnu/src/include \
-I/home/johnu/src/CHOLMOD/Include \
-I/home/johnu/mosek/6/tools/platform/linux64x86/h
CXXFLAGS=-c -fPIC -Wall
LD=gcc
LDFLAGS=        -L. -L/home/johnu/src/lib \
	-L/home/johnu/mosek/6/tools/platform/linux64x86/bin \
	-lpymparserd -lpymoptimizerd -lpymrsd \
	-lconfig -lmosek64 -lpthread \
	-lm -lcholmod `pkg-config --libs x11 xext` -lGL -lGLU -lglut \
	-lpthread -lGLEW
OBJS=FiberEffectImp.o FiberEffectImpAll.o csparse.o FiberMain.o
TARGETS=rsnatd rsfltkd libpymoptimizerd.so.1.0.1 libpymparserd.so.1.0.1
LIBS=-lm

OBJS_libpymparser=PymCmdLineParser.o TrajParser.o

OBJS_libpymoptimizer_c=Biped.o \
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

OBJS_libpymrs_c= camera.o image.o model.o quaternion.o
OBJS_libpymrs_cxx=src/PrsGraph.o src/PrsGraphCapi.o src/PrsGraphData.o

OBJS_rsnat_c=PymRs.o PhysicsThreadMain.o
OBJS_rsnat_cxx=

OBJS_rsfltk_c=
OBJS_rsfltk_cxx=rsfltk.o

all: $(TARGETS)

libpymparserd.so: $(OBJS_libpymparser)
	gcc -shared -Wl,-soname,libpymparserd.so.1 \
        -o libpymparserd.so.1.0.1 $(OBJS_libpymparser)

libpymoptimizerd.so: $(OBJS_libpymoptimizer_c) $(OBJS_libpymoptimizer_cxx)
	g++ -shared -Wl,-soname,libpymoptimizerd.so.1 \
        -o libpymoptimizerd.so.1.0.1 $(OBJS_libpymoptimizer_c) \
	$(OBJS_libpymoptimizer_cxx)

libpymrsd.so: $(OBJS_libpymrs_c) $(OBJS_libpymrs_cxx)
	g++ -shared -Wl,-soname,libpymrsd.so.1 \
        -o libpymrsd.so.1.0.1 $(OBJS_libpymrs_c) \
	$(OBJS_libpymrs_cxx)

rsnatd: $(OBJS_rsnat_c) $(OBJS_rsnat_cxx) libpymparserd.so \
libpymoptimizerd.so libpymrsd.so
	g++ $(LDFLAGS) $(OBJS_rsnat_c) $(OBJS_rsnat_cxx) -o rsnatd

rsfltkd: $(OBJS_rsfltk_c) $(OBJS_rsfltk_cxx) libpymparserd.so \
libpymoptimizerd.so libpymrsd.so
	g++ $(LDFLAGS) $(OBJS_rsfltk_c) $(OBJS_rsfltk_cxx) \
	`fltk-config --ldflags --cxxflags --use-gl` -o rsfltkd

clean:
	rm `find . -name "*.o"`
	rm $(TARGETS)

#.o: %.c %.cpp
#	g++ $(CFLAGS) $< -o $@
