#include <mpi++.h>
//#include <mpe++.h>
#include <stdio.h>
#include <magic/applic.h>
#include <magic/Math.h>
#include "wireelement.h"

class StringEquation : public WireEquation {
  public:
	StringEquation (double old, double tau) : mOldX (old), mTau(tau) {}
	virtual double	calc	(const double* left, double x, const double* right) {
		if (mOldX == 0.0)
			mOldX == x;
		double result = 2*x-mOldX+mTau*mTau*((left?*left:0.0) - 2*x + (right?*right:0.0));
		mOldX = x;
		return result;
	}
  protected:
	double mOldX;
	double mTau;
};

class StringFragment : public WireFragment {
  public:
						StringFragment	(int len, MPIInstance& mpi) : WireFragment (0, mpi) {make (len);}
	virtual void		make			(int len);
};

void StringFragment::make (int len) {
	printf ("Making string fragment... \n");
	int myID = mMPI.world().getRank ();
	int mpis = mMPI.world().size ();

	// Create elements
	for (int i=0; i<len; i++) {
		// Calculate initial value for the element
		double j = (myID-1)*len+i;
		double y = sin (2*M_PI*j/double((mpis-1)*len));

		StringEquation* eq = new StringEquation (y, 1.0*0.3/1.0);

		// Insert right kind of element in the fragment
		if (i==0 && myID>1) // First element
			mElements.add (new CommElement (y, mMPI, myID-1, eq));
		else if (i==len-1 && myID<mpis-1) // Last element
			mElements.add (new CommElement (y, mMPI, myID+1, eq));
		else // All other elements are ordinary
			mElements.add (new WireElement (y, eq));
	}

	link ();
}

///////////////////////////////////////////////////////////////////////////////

Main () {
	MPIInstance mpi (mArgc, mArgv);

	//MPEWindow mpe (mpi.world(), 0, 0, 400, 400, NULL);

	if (mpi.world().getRank() == 0) {

		//
		// MASTER
		//
		
		String buffer;
		buffer.reserve (1024);

		// Get the command-line parameters
		int wirelen		= mParamMap["wirelen"];
		double epsilon	= mParamMap["epsilon"];
		int maxcycles	= mParamMap["maxcycles"];

		// Calculate the wire segment length for the children
		int childlen = wirelen / (mpi.world().size()-1);
		printf ("childlen=%d\n", childlen);

		// Send the segment length to the children
		int children = mpi.world().size ();
		for (int i=1; i<children; i++)
			mpi.world().send (format("%d %f %d", childlen, epsilon, maxcycles), i);
	
		// Iterate until all children have finished
		int t=0;
		for (bool finished = false; !finished; t++) {
			// Finish anyways after certain limit
			if (t>=maxcycles)
				finished = true;
			
			// Order children to continue or terminate
			for (int i=1; i<children; i++)
				mpi.world().send (finished? "terminate":"continue", i);
		}
		printf ("Master terminating after %d cycles.\n", t-1);
		
	} else {

		//
		// SLAVE
		//
		
		// Receive some parameters from the master
		String buffer = mpi.world().recv (100, 0);
		Array<String> pars;
		buffer.split (pars, ' ');
		int len			= pars[0]; // Segment length
		double epsilon	= pars[1]; // Termination limit
		int maxcycles	= pars[2]; // Maximum number if iterations

		printf ("Making slave...\n");
		StringFragment fragment (len, mpi);
		fragment.print ();
		fragment.run (0.0, maxcycles);
		printf ("Child %d finished\n", mpi.world().getRank ());
	}
}
