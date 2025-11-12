MPIPATH = @MPIDIR@
MPI_LD = -L$(MPIPATH)/lib -lmpe
MPIRUN = $(MPIPATH)/bin/mpirun -v
CC = $(MPIPATH)/bin/mpiCC

###############################################################################

bin_PROGRAMS = ping wire @MPE_EXAMPLES@ mmul
EXTRA_PROGRAMS = wibstring heat nbody

ping_SOURCES = ping.cc
ping_LDADD = -lmagic -lapp -lmpipp -L../libsrc -L$(libdir)

wire_SOURCES = wireelement.cc wire.cc
wire_LDADD = -lmagic -lapp -lmpipp -L../libsrc -L$(libdir)

wibstring_SOURCES = wireelement.cc wibstring.cc
wibstring_LDADD = -lmagic -lapp -lmpipp -L../libsrc -L$(libdir)

heat_SOURCES = fdgrid.cc heat.cc
heat_LDADD =   -lmagic -lX11 -lapp -L../libsrc -L$(libdir) -L/usr/X11R6/lib -lmpipp $(MPI_LD) -lmagic

mmul_SOURCES = mmul.cc
mmul_LDADD =  -lmagic -lapp -L../libsrc -L$(libdir) -lmpipp

nbody_SOURCES = nbody.cc
nbody_LDADD =  -lmagic -lX11 -lapp -L../libsrc -L$(libdir) -L/usr/X11R6/lib -lmpipp $(MPI_LD) -lmagic

###############################################################################
# General parameters

include_HEADERS = wireelement.h fdgrid.h nbody.h
INCLUDES = -I$(includedir) -I../libsrc -I/home/magi/c/include @MPI_INCLUDE@
CXXFLAGS = -g $(CPPFLAGS) -O9

###############################################################################
# Compiling and running

ping: ping.o
	$(CC) -o ping ping.o $(ping_LDADD)

runping:
	export MPI_USEP4SSPORT=yes
	export MPI_P4SSPORT=1234
	$(MPIRUN) -np 2 ping 10 10 100 -loops=1
#$(MPIPATH)/bin/serv_p4 -port=1234  

wire: wire.o wireelement.o
	$(MPIPATH)/bin/mpiCC -o wire wire.o wireelement.o $(wire_LDADD)

runwire:
	$(MPIRUN) -np 4 wire -wirelen=75 -epsilon=0.1 -maxcycles=500

wibstring: wibstring.o wireelement.o
	$(MPIPATH)/bin/mpiCC -o wibstring wibstring.o wireelement.o $(wibstring_LDADD)

runwibstring:
	$(MPIRUN) -np 4 wibstring -wirelen=75 -epsilon=0.1 -maxcycles=500

heat: heat.o fdgrid.o
	$(MPIPATH)/bin/mpiCC -o heat heat.o fdgrid.o $(heat_LDADD)

runheat:
	$(MPIRUN) -np 4 heat

mmul: mmul.o FORCE
	$(MPIPATH)/bin/mpiCC -o mmul mmul.o $(mmul_LDADD)

runmmul:
	$(MPIRUN) -np 1 mmul

nbody: nbody.o FORCE
	$(MPIPATH)/bin/mpiCC -o nbody nbody.o $(nbody_LDADD)

runnbody:
	$(MPIRUN) -np 2 nbody

FORCE:
