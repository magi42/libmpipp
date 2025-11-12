#include "mpi++.h"
#include <stdio.h>

void main (int argc, char** argv) {
	MPIInstance mpi (argc, argv);
	
	if (mpi.getRank() == 0) {
		mpi.send ("Hello, world!");
	} else {
        printf("received :%s:\n", (CONSTR) mpi.recv (20));
	}
}
