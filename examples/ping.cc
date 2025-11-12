#include "mpi++.h"
#include <stdio.h>
#include <magic/applic.h>
#include <magic/Math.h>

#define MAX_SIZE 1000000

Main () {
	MPIInstance mpi (mArgc, mArgv);

	// Reread application parameters and put the into variables
	int msgMinlen	 = mParams[0];
	int msgIncrement = mParams[1];
	int msgMaxlen	 = mParams[2];
	int loops = mParamMap["loops"];

	Vector record ((msgMaxlen-msgMinlen)/msgIncrement+1);
	for (int i=0; i<record.size; i++)
		record[i] = 0.0;

	// Create a buffer, and allocate enough space for it.
	String buffer;
	buffer.reserve (msgMaxlen);

	if (mpi.world().getRank() == 0) {

		// Sender process

		printf ("Sender iterating for %d...%d +=%d (%d values)\n",
				msgMinlen, msgMaxlen, msgIncrement, record.size);

		for (int l=0; l<loops; l++) {
			fprintf (stderr, "\r% 5d\r", l);
			// Send packets of growing sizes
			for (int s=msgMinlen; s<=msgMaxlen; s+=msgIncrement) {
				buffer.len = s; // Force the buffer size; don't care about contents
				
				// Bounce the buffer from Echo, and measure time
				double t1 = mpi.time();
				mpi.world().send (buffer, 1);
				mpi.world().recv (buffer, buffer.len, 1);
				double t2 = mpi.time();
				
				// Record time difference in milliseconds
				record[(s-msgMinlen)/msgIncrement] += (t2-t1)*1000/2;
				//printf ("%d\t%1.6f\n", s, (t2-t1)*1000);
			}
		}

		// Display statistics
		fprintf(stderr, "\r     \r");
		for (int i=0; i<record.size; i++)
			printf ("%d\t%1.6f\n", msgMinlen+i*msgIncrement, record[i]/double(loops));

		// Send stop message to the Echo
		mpi.world().send ("stop", 1);
	} else {

		// Echo process
		
		while (true) {
			mpi.world().recv (buffer, MAX_SIZE, 0);

			if (buffer == "stop")
				break;

			mpi.world().send (buffer, 0);
		}
	}
 end:;
}
