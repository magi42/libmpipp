SUBDIRS = libsrc examples
#EXTRA_DIST = aux/install-sh aux/missing aux/mkinstalldirs aux/config.guess aux/config.sub aux/ltconfig aux/ltmain.sh
EXTRA_DIST =

pose:
	cd .. && \
	tar zcf mpi.tgz mpi && \
	scp mpi.tgz countess@poseidon.cs.abo.fi:. && \
	rm mpi.tgz
