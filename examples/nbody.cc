/***************************************************************************
    copyright            : (C) 2000 by Marko Grönroos
    email                : magi@iki.fi
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
 *
 **/

#include <mpi++.h>
#include <mpe++.h>
#include <magic/applic.h>
#include "nbody.h"
#include <unistd.h> // sleep()

//////////////////////////////////////////////////////////////////////////////
//                          ----           |                                //
//                          |   )          |                                //
//                          |---   __   ---| \   |                          //
//                          |   ) /  \ (   |  \  |                          //
//                          |___  \__/  ---|   \_/                          //
//                                            \_/                           //
//////////////////////////////////////////////////////////////////////////////

Coord Body::force (const Body& other, float& r) const {
	Coord result;
	r = mPosition.dist (other.mPosition);
	result = (other.mPosition-mPosition)/(r*r*r) * other.mMass * mMass;
	return result;
}

void Body::updateVelocity (const Coord& force) {
	mVelocity += force/mMass;
}

void Body::updatePosition (float h) {
	mPosition += mVelocity * h;
}

void Body::print (OStream& out) const {
	printf ("pos=(%f,%f), delta=(%f,%f), m=%f\n",
			mPosition.x, mPosition.y, mVelocity.x, mVelocity.y, mMass);
}


//////////////////////////////////////////////////////////////////////////////
//                       |   | ----           |                             //
//                       |\  | |   )          |                             //
//                       | \ | |---   __   ---| \   |                       //
//                       |  \| |   ) /  \ (   |  \  |                       //
//                       |   | |___  \__/  ---|   \_/                       //
//                                               \_/                        //
//////////////////////////////////////////////////////////////////////////////

NBody::NBody (const StringMap& params, MPEWindow& mpe) : mrParams(params), mrMPE(mpe) {
	mBodies.make (int(params["NBody.n"]));
	mMinR = float(params["NBody.r"]);

	mViewCenter = Coord (float(params["viewCenter.x"]), float(params["viewCenter.y"]));
	mViewRadius = float(params["viewCenter.r"]);
	mUndraw = int(params["undraw"]);
	mDensity = int(params["NBody.density"]);
}

void NBody::init (const BodyIniter& initer) {
	for (int i=0; i<mBodies.size; i++)
		initer.visit (mBodies[i], i, mBodies.size);
}

void NBody::run (int iters, float h, int updateFreq) {
	PackArray<Coord> plotCoords (mBodies.size); // Holds previous plot coordinates
	
	for (int i=0; i<iters; i++) {

		// Update position of the bodies. Not done on the first
		// iteration, because of the leapfrog method.
		if (i>0)
			updatePositions (h, !(i%updateFreq), plotCoords);

		// Calculate forces (symmetrically)
		resetForces ();
		calculateForces (mBodies, mBodies, true);

		// Update velocity of the bodies
		updateVelocities (i? h : h/2);
	}
}

void NBody::resetForces () {
	for (int i=0; i<mBodies.size; i++)
		mBodies[i].resetForces ();
}

void NBody::calculateForces (PackArray<Body>& a, PackArray<Body>& b, bool diagonal) {
	float r;
	for (int i=0; i<a.size; i++) {
		// Iterate only half of the a*b matrix
		for (int j=diagonal? i+1:0; j<b.size; j++) {
			// Compute force
			Coord force = a[i].force (b[j], r);

			// Forces apply only if the bodies are further than the
			// minimum distance
			if (r>mMinR) {
				a[i].addForce (force);
				b[j].addForce (-force); // Symmetric force
			}
		}
	}
}

void NBody::updateVelocities (float h) {
	for (int i=0; i<mBodies.size; i++) {
		Coord force = mBodies[i].totalForce() * cGravity * h;
		mBodies[i].updateVelocity (force);
	}
}

void NBody::updatePositions (float h, bool updateView, PackArray<Coord>& plotCoords) {
	for (int a=0; a<mBodies.size; a++) {
		// Calculate drawing radius
		float rf = pow(3000*mBodies[a].mass()/(4*mDensity*M_PI), 1.0/3);
		int r = int (rf/mViewRadius+0.5);
		if (r==0)
			r=1;

		// Undraw the old picture
		if (updateView && mUndraw)
			mrMPE.fillCircle (plotCoords[a].x, plotCoords[a].y, r, MPE_WHITE);
		
		// Update body position
		mBodies[a].updatePosition (h);
		
		// Draw the new picture
		if (updateView) {
			plotCoords[a] = Coord (200+200*mBodies[a].position().x/mViewRadius,
								   200+200*mBodies[a].position().y/mViewRadius);
			mrMPE.fillCircle (plotCoords[a].x, plotCoords[a].y, r, mrMPE.comm().getRank()? MPE_BLACK:MPE_BLUE);
		}
	}
	if (updateView)
		mrMPE.update ();
}



//////////////////////////////////////////////////////////////////////////////
//             ----  o             |   | ----           |                   //
//             |   )     _         |\  | |   )          |                   //
//             |---  | |/ \   ___  | \ | |---   __   ---| \   |             //
//             | \   | |   | (   \ |  \| |   ) /  \ (   |  \  |             //
//             |  \  | |   |  ---/ |   | |___  \__/  ---|   \_/             //
//                            __/                          \_/              //
//////////////////////////////////////////////////////////////////////////////

RingNBody::RingNBody (const StringMap& params, MPEWindow& mpe, MPIInstance& mpi) : NBody (params, mpe), mrMPI (mpi) {
	ASSERTWITH (!(mBodies.size%2), "N for RingNBody system must be divisible by 2.");
	
	// Determine the next and previous process id in the process ring
	mNext = (mpi.world().getRank()+1) % mpi.world().size();
	mPrev = (mpi.world().getRank()-1+mpi.world().size()) % mpi.world().size();

	// Create MPI vector type for Body arrays. Note that the order of
	// the member variables in the Body class is crucial!
	mpBodyVectorType = new MPIVector (mBodies.size, 5, sizeof (Body)/sizeof(float), MPI_FLOAT);
}

void RingNBody::run (int iters, float h, int updateFreq) {
	PackArray<Coord> plotCoords (mBodies.size); // Holds previous MPE plot coordinates
	sout.autoflush (true);

	// Create a double-buffered container for circulating bodies
	Array<PackArray<Body> > circulatingBodies;
	circulatingBodies.add (new PackArray<Body> (mBodies.size)); // Create buffer #0
	circulatingBodies.add (new PackArray<Body> (mBodies.size)); // Create buffer #1

	int ringSize = mrMPI.world().size();
	for (int iter=0; iter<iters; iter++) {
		// Update positions of the bodies. Use ½h on the first
		// iteration, to implement leapfrog method.
		updatePositions (iter? h:h/2, !(iter%updateFreq), plotCoords);

		resetForces ();

		// Copy the local bodies into circulating bodies.
		circulatingBodies[0].shallowCopy (mBodies);

		for (int i=0; i<ringSize; i++) {
			// Calculate forces diagonally between resident and
			// circulating bodies. On the step=0, the circulating
			// bodies are local.
			calculateForces (mBodies, circulatingBodies[i%2], true);
			
			// Send circulating bodies forward in the ring
			mrMPI.world().nbSend (circulatingBodies[i%2].data, 1, mpBodyVectorType->getType(), mNext);
			
			// Receive circulating bodies from previous node in the
			// ring. In the last step, we receive back the local
			// circulating bodies, which we sent out in the step=0.
			mrMPI.world().recv (circulatingBodies[(i+1)%2].data, 1, mpBodyVectorType->getType(), mPrev);
		}

		// Add the forces from the circulated local bodies to resident
		// bodies
		for (int i=0; i<mBodies.size; i++)
			mBodies[i].addForce (circulatingBodies[ringSize%2][i].totalForce());

		// Update velocities of the bodies
		updateVelocities (h);
	}
}



//////////////////////////////////////////////////////////////////////////////
//             ----           |       ---       o                           //
//             |   )          |        |    _      |   ___                  //
//             |---   __   ---| \   |  |  |/ \  | -+- /   ) |/\             //
//             |   ) /  \ (   |  \  |  |  |   | |  |  |---  |               //
//             |___  \__/  ---|   \_/ _|_ |   | |   \  \__  |               //
//                               \_/                                        //
//////////////////////////////////////////////////////////////////////////////

BodyIniter::BodyIniter () {
	srandom (time(0)+getpid()*100);
}

RandomIniter::RandomIniter (const StringMap& params) {
	mCorner1		= Coord (params["RandomIniter.upperleft.x"],
							 params["RandomIniter.upperleft.y"]);
	mCorner2		= Coord (params["RandomIniter.lowerright.x"],
							 params["RandomIniter.lowerright.y"]);
	mVelocityRange	= Coord (params["RandomIniter.velocityRange.x"],
							 params["RandomIniter.velocityRange.y"]);
}

/*virtual*/ void RandomIniter::visit (Body& body, int id, int size) const {
	body.setPosition (Coord(mCorner1.x+fabs(mCorner1.x-mCorner2.x)*frnd(),
							mCorner1.y+fabs(mCorner1.y-mCorner2.y)*frnd()));
	float x = frnd()*M_PI*2;
	body.setVelocity (Coord (mVelocityRange.x*sin(x),
							 mVelocityRange.y*cos(x)));
	body.setMass (1.0E2); // Always 100kg. TODO: read from parameters
}

PresetIniter::PresetIniter (const StringMap& params) : mrParams (params) {
}

/*virtual*/ void PresetIniter::visit (Body& body, int id, int size) const {
	const String& body_s = mrParams[format("PresetIniter.body%d", id)];
	Array<String> fields;
	body_s.split (fields, ',');
	
	body.setPosition (Coord (float(fields[0]), float(fields[1])));
	body.setVelocity (Coord (float(fields[2]), float(fields[3])));
	body.setMass (float(fields[4]));

	body.print (sout);
}



///////////////////////////////////////////////////////////////////////////////
//                            |   |       o                                  //
//                            |\ /|  ___      _                              //
//                            | V |  ___| | |/ \                             //
//                            | | | (   | | |   |                            //
//                            |   |  \__| | |   |                            //
///////////////////////////////////////////////////////////////////////////////

Main () {
	sout.printf ("sizeof(Body)=%d, sizeof(float)=%d\n", sizeof(Body), sizeof(float));
	
	// Read application parameters from file
	readConfig ("/home/magi/c/opinnot/mpi/examples/nbody.cfg");
	mParamMap.failByThrow ();

	MPIInstance mpi (mArgc, mArgv);
	MPEWindow mpe (mpi.world(), 0, 0, 400, 400, NULL);

	// Create the N-body system
	NBody* system;
	if (paramMap()["system"] == "NBody")
		system = new NBody (paramMap(), mpe);
	else if (paramMap()["system"] == "RingNBody")
		system = new RingNBody (paramMap(), mpe, mpi);
	else
		exit (1);

	// Initialize the system
	BodyIniter* initer;
	if (paramMap()["initer"] == "RandomIniter")
		initer = new RandomIniter (paramMap());
	else if (paramMap()["initer"] == "PresetIniter")
		initer = new PresetIniter (paramMap());
	else
		exit (1);
	system->init (*initer);

	// Run the system
	system->run (int(paramMap()["iters"]), float(paramMap()["h"]), int(paramMap()["update"]));
	
	printf ("Done.\n");
}
