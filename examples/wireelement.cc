#include "mpi++.h"
#include <stdio.h>
#include <magic/Math.h>
#include "wireelement.h"

///////////////////////////////////////////////////////////////////////////////
//           |   | o           ----- |                                       //
//           | | |        ___  |     |  ___         ___    _    |            //
//           | | | | |/\ /   ) |---  | /   ) |/|/| /   ) |/ \  -+-           //
//           | | | | |   |---  |     | |---  | | | |---  |   |  |            //
//            V V  | |    \__  |____ |  \__  | | |  \__  |   |   \           //
///////////////////////////////////////////////////////////////////////////////

void WireElement::update () {
	if (mEquation) {
		double leftTemp = mLeft? mLeft->mTemp : 0.0;
		double rightTemp = mRight? mRight->mTemp : 0.0;
		mNewTemp = mEquation->calc (mLeft? &leftTemp : NULL, this->mTemp, mRight? &rightTemp : NULL);
	}  else
		mNewTemp = ((mLeft? mLeft->mTemp:0) + (mRight? mRight->mTemp:0))
			/ (!!mLeft + !!mRight);
}



//////////////////////////////////////////////////////////////////////////////
//         ___                   ----- |                                    //
//        /   \                  |     |  ___         ___    _    |         //
//        |      __  |/|/| |/|/| |---  | /   ) |/|/| /   ) |/ \  -+-        //
//        |     /  \ | | | | | | |     | |---  | | | |---  |   |  |         //
//        \___/ \__/ | | | | | | |____ |  \__  | | |  \__  |   |   \        //
//////////////////////////////////////////////////////////////////////////////

void CommElement::update () {
	// Receive data from the neighbouring element
	String buffer;
	buffer.ensure (100);
	mMPI.world().recv (buffer, buffer.maxLen(), mNeighbourID);
	double mNeighbourTemp = buffer; // The value is there as plain ascii text

	// Now use it with the data from the other, ordinary neighbour to
	// update the temperature.
	if (mEquation) {
		double leftTemp = mLeft? mLeft->mTemp : mNeighbourTemp; // A little kludge
		double rightTemp = mRight? mRight->mTemp : mNeighbourTemp;
		mNewTemp = mEquation->calc (mLeft? &leftTemp : NULL, this->mTemp, mRight? &rightTemp : NULL);
	}  else
		mNewTemp = ((mLeft? mLeft->mTemp:0) + (mRight? mRight->mTemp:0)
					+ mNeighbourTemp) / (!!mLeft + !!mRight + 1);
}

void CommElement::update2 () {
	mTemp = mNewTemp;
	send (); // Send the new value to the neighbour
}

void CommElement::send () {
	// Use nonblocking send; the receiver uses blocking receive, which
	// synchronizes the computation in these two wire elements.
	//
	// NOTE: The temperature is sent as plain ascii text.
	mMPI.world().nbSend (mTemp, mNeighbourID);
}

///////////////////////////////////////////////////////////////////////////////
//       |   | o           -----                                             //
//       | | |        ___  |          ___               ___    _    |        //
//       | | | | |/\ /   ) |---  |/\  ___|  ___  |/|/| /   ) |/ \  -+-       //
//       | | | | |   |---  |     |   (   | (   \ | | | |---  |   |  |        //
//        V V  | |    \__  |     |    \__|  ---/ | | |  \__  |   |   \       //
//                                          __/                              //
///////////////////////////////////////////////////////////////////////////////

WireFragment::WireFragment (int len, MPIInstance& mpi)
		: mMPI (mpi) {

	if (len)
		make (len);
}

void WireFragment::make (int len) {
	int myID = mMPI.world().getRank ();
	int mpis = mMPI.world().size ();

	// Create elements
	for (int i=0; i<len; i++) {
		if (i==0) // First element
			if (myID==1) // First wire fragment
				mElements.add (new StaticElement (0.0));
			else // Later fragments connect to the previous fragments
				mElements.add (new CommElement (20.0, mMPI, myID-1));
		else
			if (i==len-1) // Last element
				if (myID==mpis-1) // Last wire fragment
					mElements.add (new StaticElement (100.0));
				else // Earlier fragments connect to the later fragments
					mElements.add (new CommElement (20.0, mMPI, myID+1));
			else // All other elements are ordinary
				mElements.add (new WireElement (20.0));
	}

	link ();
}

void WireFragment::link () {
	// Link the neighbouring elements together as a doubly-linked list
	for (int i=0; i<mElements.size; i++)
		mElements[i].link ((i>0)?     &mElements[i-1] : NULL,
						   (i<mElements.size-1)? &mElements[i+1] : NULL);
}

void WireFragment::print (FILE* out) const {
	fprintf (out, "Child %d has %d elements: ",
			mMPI.world().getRank (), mElements.size);

	// List elements
	for (int i=0; i<mElements.size; i++)
		if (dynamic_cast<const CommElement*>(&mElements[i]))
			fprintf (out, "C");
		else if (dynamic_cast<const WireElement*>(&mElements[i]))
			fprintf (out, "W");
		else if (dynamic_cast<const StaticElement*>(&mElements[i]))
			fprintf (out, "S");
			else
				fprintf (out, "?");
	fprintf (out, "\n");
	fflush (out);
}

void WireFragment::initComm () {
	// Order communicative elements to send their initial data to
	// other processes.
	for (int i=0; i<mElements.size; i++)
		if (CommElement* e = dynamic_cast<CommElement*>(&mElements[i]))
			e->send ();
}

void WireFragment::run (double epsilon, int maxcycles) {
	initComm ();

	// Run for at most maxcycles, or infinitely, if it is -1
	int cycle=0;
	for (;true;cycle++) {
		// Update the values once
		for (int i=0; i<mElements.size; i++)
			mElements[i].update ();

		// Update new temperature values as current
		bool under_epsilon=true;
		for (int i=0; i<mElements.size; i++) {
			// Check if any of the deltas is still greater than epsilon
			if (mElements[i].delta() > epsilon)
				under_epsilon = false;
			mElements[i].update2 ();
		}

		// Generate and send a report to the master

		// Report starts with T, if the delta is below epsilon,
		// otherwise with N
		String report = under_epsilon? "T" : "N";

		// Temperatures of the elements are listed in plain text
		for (int i=0; i<mElements.size; i++)
			report += format ("%g\n", mElements[i].temp());
		mMPI.world().nbSend (report, 0);

		// Get orders from master
		if (mMPI.world().recv (100, 0) == "terminate")
			break;
	}
}
