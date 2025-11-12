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

#ifndef __NBODY_H__
#define __NBODY_H__

#include <magic/Math.h>
#include <magic/coord.h>
#include <magic/packarray.h>

const float cGravity = 6.67259E-11;

class BodyIniter;	// Local

// Coord can be either Coord3D or Coord2D - the both classes have identical operations
#define Coord Coord2D


//////////////////////////////////////////////////////////////////////////////
//                          ----           |                                //
//                          |   )          |                                //
//                          |---   __   ---| \   |                          //
//                          |   ) /  \ (   |  \  |                          //
//                          |___  \__/  ---|   \_/                          //
//                                            \_/                           //
//////////////////////////////////////////////////////////////////////////////

/** A body in an N-body system.
 *
 *  NOTE: The order of the private member attributes is very
 *  important!  NBody relies on the location of the mPosition and
 *  mForces. The class must also be otherwise "clean" - no inheritance
 *  of any kind, and it may not have a virtual table.
 *
 *  Also, the Coord2D objects may *not* have a virtual table!
 **/
class Body {
  public:

	/** Alters the current velocity according to the given force. */
	void			updateVelocity	(const Coord& force);

	/** Updates the position according to current velocity, with time
	 *  step h.
	 **/
	void			updatePosition	(float h);

	const Coord&	position		() const {return mPosition;}
	float			mass			() const {return mMass;}
	
	void			setMass			(float mass) {mMass = mass;}
	void			setPosition		(const Coord& value) {mPosition = value;}
	void			setVelocity		(const Coord& value) {mVelocity = value;}

	/** Calculates the force between this and the other body.
	 *
	 *  Distance between the bodies will be set to parameter r.
	 **/
	Coord			force			(const Body& other, float& r) const;

	/** Resets the forces affecting the body. */
	void			resetForces		() {mForces = Coord (0.0,0.0);}

	/** Accumulates a force vector. */
	void			addForce		(const Coord& force) {mForces += force;}

	/** Returns the total force affecting the body. */
	const Coord&	totalForce		() const {return mForces;}

	void			print			(OStream& out) const;
	
  private:
	// NOTE: THE ORDER OF THESE ATTRIBUTES MAY NOT BE ALTERED!

	/** Position of the body. */
	Coord	mPosition;

	/** Accumulated forces affecting this body. */
	Coord	mForces;

	float	mMass;
	Coord	mVelocity;
};



//////////////////////////////////////////////////////////////////////////////
//                       |   | ----           |                             //
//                       |\  | |   )          |                             //
//                       | \ | |---   __   ---| \   |                       //
//                       |  \| |   ) /  \ (   |  \  |                       //
//                       |   | |___  \__/  ---|   \_/                       //
//                                               \_/                        //
//////////////////////////////////////////////////////////////////////////////

/** N-body system, a sequential solution.
 *
 **/
class NBody {
  public:
					NBody				(const StringMap& params, MPEWindow& mpe);

	/** Initializes the bodies in the system with the given
	 *  initializer.
	 **/
	virtual void	init				(const BodyIniter& initer);

	/** Runs the system.
	 **/
	virtual void	run					(int iters, float h, int updateFreq);

  protected:
	/** Updates the positions of all bodies and draws them in
	 *  MPEWindow if updateView is true.
	 *
	 *  Parameter h is the time step.
	 *
	 *  Parameter plotCoords is a vector where coordinates from the
	 *  previous graphics update were stored.
	 **/
	void			updatePositions		(float h, bool updateView, PackArray<Coord>& plotCoords);

	/** Resets the forces affecting all bodies. */
	void			resetForces			();

	/** Calculates forces between two sets of bodies.
	 *
	 *  Parameter 'diagonal' tells that the sets refer to the same
	 *  bodies, and we can use the symmetric forces between the
	 *  bodies.
	 **/
	void			calculateForces		(PackArray<Body>& a, PackArray<Body>& b, bool diagonal);

	/** Updates the velocities of all bodies. */
	void			updateVelocities	(float h);
	
  protected:
	/** All local bodies, both resident and circulating. */
	PackArray<Body>		mBodies;

	/** Minimum distance for calculating the gravitational force. */
	float				mMinR;

	/** Center coordinates of the view. */
	Coord				mViewCenter;

	/** Radius of the view. */
	float				mViewRadius;

	/** Should we undraw the bodies or not? */
	bool				mUndraw;

	/** Density of the bodies: used for calculating the graphical
	 * representation of the body
	 **/
	float mDensity;
	
	/** Store the parameters for later use. */
	const StringMap&	mrParams;

  private:
	MPEWindow&			mrMPE;
};



//////////////////////////////////////////////////////////////////////////////
//             ----  o             |   | ----           |                   //
//             |   )     _         |\  | |   )          |                   //
//             |---  | |/ \   ___  | \ | |---   __   ---| \   |             //
//             | \   | |   | (   \ |  \| |   ) /  \ (   |  \  |             //
//             |  \  | |   |  ---/ |   | |___  \__/  ---|   \_/             //
//                            __/                          \_/              //
//////////////////////////////////////////////////////////////////////////////

/** Parallel ring-shaped N-body system.
 *
 **/
class RingNBody : public NBody {
  public:
					RingNBody	(const StringMap& params, MPEWindow& mpe, MPIInstance& mpi);
					~RingNBody	() {delete mpBodyVectorType;}

	/** Runs the system. */
	void			run			(int iters, float h, int updateFreq);
	
  private:
	MPIInstance&	mrMPI;

	/** Previous process in the process ring. */
	int				mPrev;

	/** Next process in the process ring. */
	int				mNext;

	/** MPI datatype for Body vectors. */
	MPIVector*		mpBodyVectorType;
};



//////////////////////////////////////////////////////////////////////////////
//             ----           |       ---       o                           //
//             |   )          |        |    _      |   ___                  //
//             |---   __   ---| \   |  |  |/ \  | -+- /   ) |/\             //
//             |   ) /  \ (   |  \  |  |  |   | |  |  |---  |               //
//             |___  \__/  ---|   \_/ _|_ |   | |   \  \__  |               //
//                               \_/                                        //
//////////////////////////////////////////////////////////////////////////////

/** Abstract body initializer baseclass.
 *
 *  Design Patterns: Visitor, Strategy.
 **/
class BodyIniter {
  public:
					BodyIniter		();
	
	virtual void	visit			(Body& body, int id, int size) const=0;
};

/** Body initializer that does completely random initialization within
 *  a value range.
 **/
class RandomIniter : public BodyIniter {
  public:
					RandomIniter	(const StringMap& params);

	/** Implementation. */
	virtual void	visit			(Body& body, int id, int size) const;
	
  private:
	Coord	mVelocityRange;
	Coord	mCorner1;
	Coord	mCorner2;
};

/** Body initializer that reads body parameters from config file.
 *
 **/
class PresetIniter : public BodyIniter {
  public:
					PresetIniter	(const StringMap& params);

	/** Implementation. */
	virtual void	visit			(Body& body, int id, int size) const;
	
  private:
	const StringMap&	mrParams;
};

#endif
