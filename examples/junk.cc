			// Create nice spectrum palette with 254 colors + black
			//MPE_Color defcolors[] = {MPE_WHITE, MPE_BLACK, MPE_RED, MPE_YELLOW, MPE_GREEN, MPE_CYAN, MPE_BLUE, MPE_MAGENTA, MPE_AQUAMARINE, MPE_FORESTGREEN, MPE_ORANGE, MPE_MAROON, MPE_BROWN, MPE_PINK, MPE_CORAL, MPE_GRAY};

MPE_Color colors [256];
			for (int i=0; i<win.colors(); i++)
				colors[i] = defcolors[i];
			win.makeColorArray (colors+win.colors(), 256-win.colors());

/*
			for (int i=0; i<64; i++) // Red to yellow
				colors[i] = win.addRGBcolor (255, i*4, 0);
			fprintf (stderr, "Colors=%d\n", win.colors ());
			for (int i=0; i<64; i++) // Yellow to green
				colors[i+64] = win.addRGBcolor (255-i*4, 255, 0);
			fprintf (stderr, "Colors=%d\n", win.colors ());
			for (int i=0; i<64; i++) // Yellow to green
				colors[i+128] = win.addRGBcolor (0, 255, i*4);
			fprintf (stderr, "Colors=%d\n", win.colors ());
			for (int i=0; i<64; i++) // Yellow to green
				colors[i+196] = win.addRGBcolor (0, 255-i*4, 255);
			fprintf (stderr, "Colors=%d\n", win.colors ());
			colors[255] = MPE_BLACK;
			*/

###############################################################################

void RingNBody::run (int iters, float h, int updateFreq) {
	PackArray<Coord> plotCoords (mBodies.size); // Holds previous plot coordinates
	sout.autoflush (true);
	
	// A group of external bodies
	Array<PackArray<Body> > otherBodies;
	otherBodies.add (new PackArray<Body> (mBodies.size));
	otherBodies.add (new PackArray<Body> (mBodies.size));

	int ringSize = mrMPI.world().size();
	for (int iter=0; iter<iters; iter++) {
		mrMPI.world().barrier ();

		// Calculate forces (symmetrically)
		resetForces ();

		sout.printf ("Process %d, iteration %d\n", mrMPI.world().getRank(), iter);
	
		// We first calculate forces between local bodies, so we copy the
		// local bodies to the external array
		otherBodies[iter%2].shallowCopy (mBodies);
		
		for (int i=0; i<ringSize; i++) {
			// Calculate forces between local and external bodies
			//sout.printf ("Process %d calculating forces for step %d:", mrMPI.world().getRank(), i);
			calculateForces (mBodies, otherBodies[i%2], i);
			for (int a=0; a<mBodies.size; a++)
				for (int b=a+(i?0:1); b<mBodies.size; b++)
					sout.printf ("Process %d, iter=%d, step=%d: (%f,%f)*%f  vs  (%f,%f)*%f\n",
								 mrMPI.world().getRank(), iter, i,
								 mBodies[a].position().x, mBodies[a].position().y, mBodies[a].mass(), 
								 otherBodies[i%2][b].position().x, otherBodies[i%2][b].position().y, otherBodies[i%2][b].mass());
			
			// Send external bodies forward in the ring
			//sout.printf ("Sending buffer\n");
			mrMPI.world().nbSend (otherBodies[i%2].data, 1, mpBodyVectorType->getType(), mNext);
			
			// Receive external bodies from previous node in the ring
			//sout.printf ("Receiving buffer\n");
			mrMPI.world().recv (otherBodies[(i+1)%2].data, 1, mpBodyVectorType->getType(), mPrev);
		}
		//sout.printf ("Ring revolved\n");

		// Copy the forces calculated in other processes
		for (int i=0; i<mBodies.size; i++)
			mBodies[i].addForce (otherBodies[ringSize%2][i].totalForce());

		// Update velocity of the bodies
		updateVelocities (h);

		updatePositions (h, !(iter%updateFreq), plotCoords);
	}
}
