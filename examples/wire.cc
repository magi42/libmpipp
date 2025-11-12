#include <mpi++.h>
#include <stdio.h>
#include <magic/applic.h>
#include "wireelement.h"

Main () {
	MPIInstance mpi (mArgc, mArgv);

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
	
		// We use gnuplot to plot the data vector during the run
		FILE* gnuplot = popen ("gnuplot", "w");

		// Iterate until all children have finished
		int t=0;
		for (bool finished = false; !finished; t++) {
			
			// Order Gnuplot to plot the wire data for this cycle from
			// its stdin
			fprintf (gnuplot, "plot '-' t '%d' w l\n", t);

			// Collect data from the children
			finished = true;
			for (int i=1; i<children; i++) {
				// Receive wire fragment report from child
				mpi.world().recv (buffer, buffer.maxLen(), i);

				// Check if the child wants to terminate
				if (buffer[0] == 'N')
					finished = false;
					
				// Give the values to Gnuplot, and create an empty
				// hole between the fragments with a newline
				fprintf (gnuplot, "%s\n", ((CONSTR) buffer)+1);
			}

			// Finish Gnuplot data
			fprintf (gnuplot, "EOF\n");
			fflush (gnuplot);

			if (finished)
				printf ("All children under delta epsilon -> terminating\n");

			// Finish anyways after certain limit
			if (t>=maxcycles)
				finished = true;

			// Order children to continue or terminate
			for (int i=1; i<children; i++)
				mpi.world().send (finished? "terminate":"continue", i);
		}
		fclose (gnuplot);
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
	
		WireFragment fragment (len, mpi);
		fragment.print ();
		fragment.run (epsilon, maxcycles);
		printf ("Child %d finished\n", mpi.world().getRank ());
	}
}
