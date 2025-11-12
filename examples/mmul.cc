#include <stdio.h>
#include <magic/applic.h>
#include <magic/Matrix.h>
#include <netinet/in.h>

#include <mpi++.h>

class MatrixMultiplier : public Object {
  public:
	virtual	void	multiply	(const Matrix& a, const Matrix& b, Matrix& result) const=0;
};

class SequentialMMul : public MatrixMultiplier {
  public:
	virtual	void	multiply	(const Matrix& a, const Matrix& b, Matrix& result) const;
};

void SequentialMMul::multiply (const Matrix& a, const Matrix& b, Matrix& result) const {
	result.make (a.cols, b.rows);
	for (int r=0; r<result.rows; r++)
		for (int c=0; c<result.cols; c++) {
			double sum = 0.0;
			for (int k=0; k<a.rows; k++)
				sum += a.get(k,c) * b.get(r,k);
			result.get(r,c) = sum;
		}
}


Main () {
	MPIInstance mpi (mArgc, mArgv);

	int N=300;
	Matrix matrix (N,N);
	FILE* matrixIn = popen (format ("gunzip -c data/m%dx%d.txt.gz", N, N), "r");
	ASSERT (matrixIn);
	matrix.load (matrixIn);
	pclose (matrixIn);

	SequentialMMul mplier;
	Matrix c;
	printf ("Starting to multiply...\n");
	double t1 = mpi.time ();
	mplier.multiply (matrix, matrix, c);
	double t2 = mpi.time ();
	printf ("Multiplication ended.\n");
	printf ("Time difference = %g seconds\n", t2-t1);
}
