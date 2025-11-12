#include "fdgrid.h"
#include <math.h>

FDGridSegment::FDGridSegment (int N, MPIInstance& mpi)
		: mrMPI (mpi), mN (N),
		  mProcessorGrid (int (sqrt(double(mpi.world().size()))+.0001)) {
	int mpis = mrMPI.world().size();
	int q = int (sqrt(double(mpis))+.0001);

	ASSERTWITH (q*q == mpis, "Must have q^2 processors");

	int id = mrMPI.world().getRank ();

	// Create the data matrix plus ghost point (2 rows and columns)
	// for communication with neighbour processes.
	mRow0 = int (double(N)/double(q)*mProcessorGrid.row(id)+0.0001);
	mRow1 = int (double(N)/double(q)*(mProcessorGrid.row(id)+1)+0.0001)-1;
	mCol0 = int (double(N)/double(q)*mProcessorGrid.column(id)+0.0001);
	mCol1 = int (double(N)/double(q)*(mProcessorGrid.column(id)+1)+0.0001)-1;
	mMatrix.make (mRow1-mRow0+1+2, mCol1-mCol0+1+2);
	printf ("Processor %d handles matrix segment from (%d,%d) to (%d,%d)\n",
			id, mRow0, mCol0, mRow1, mCol1);
	fflush (stdout);

	// Create communication data types
	mColumnVectorType.make (mRow1-mRow0+1, 1, mMatrix.cols, MPI_DOUBLE);
}

void FDGridSegment::init () {
	for (int i=0; i<mMatrix.rows; i++)
		for (int j=0; j<mMatrix.cols; j++)
			TRYWITH (
				mMatrix.get (i,j) = initPoint (i+mRow0-1, j+mCol0-1),
				format ("Error while initializing point (%d,%d) with processor %d",
						i,j, mrMPI.world().getRank ())
				);
}

void FDGridSegment::execute (int maxcycles, double epsilon) {
	init ();
	
	for (mCycle=0; mCycle<maxcycles; mCycle++) {
		// We may terminate unless there is a delta larger than the epsilon
		int mayTerminate = true;

		startOfCycle ();
		
		// Update. Calculate every value in the matrix EXCEPT the
		// border values, which are either received by communication,
		// or constants.
		for (int r=1; r<mMatrix.rows-1; r++)
			for (int c=1; c<mMatrix.cols-1; c++) {
				// Compute the new value by the absolute coordinates
				double newvalue = compute (r+mRow0-1, c+mCol0-1);

				// We do not want to terminate if the delta > epsilon
				if (fabs(mMatrix.get(r,c) - newvalue) > epsilon)
					mayTerminate = false;

				// Update the new value
				mMatrix.get(r,c) = newvalue;
			}

		endOfCycle ();

		////////////////////////////////////////
		// Exchange data with neighbouring matrices

		MPIComm& comm = mrMPI.world();
		int id = comm.getRank();

		// Send to up and receive from down
		comm.send (&mMatrix.get(1,1), mMatrix.cols-2, MPI_DOUBLE, mProcessorGrid.upPid(id));
		comm.recv (&mMatrix.get(mMatrix.rows-1,1), mMatrix.cols-2, MPI_DOUBLE, mProcessorGrid.downPid(id));
		// Send to down and receive from up
		comm.send (&mMatrix.get(mMatrix.rows-2,1), mMatrix.cols-2, MPI_DOUBLE, mProcessorGrid.downPid(id));
		comm.recv (&mMatrix.get(0,1), mMatrix.cols-2, MPI_DOUBLE, mProcessorGrid.upPid(id));
		// Send to left and receive from right
		comm.send (&mMatrix.get(1,1), 1, mColumnVectorType.getType(), mProcessorGrid.leftPid(id));
		comm.recv (&mMatrix.get(1,mMatrix.cols-1), 1, mColumnVectorType.getType(), mProcessorGrid.rightPid(id));
		// Send to right and receive from left
		comm.send (&mMatrix.get(1,mMatrix.cols-2), 1, mColumnVectorType.getType(), mProcessorGrid.rightPid(id));
		comm.recv (&mMatrix.get(1,0), 1, mColumnVectorType.getType(), mProcessorGrid.leftPid(id));

		// Check for termination every 10th cycle, 
		if (!(cycle()%10)) {
			int mayAllTerminate;
			comm.allReduce (&mayTerminate, &mayAllTerminate, 1, MPI_INT, MPI_LAND);
			if (mayAllTerminate) {
				if (id==0)
					printf ("Converged after %d iterations.\n", cycle());
				break;
			}
		}
	}
}
