#ifndef __WIREELEMENT_H__
#define __WIREELEMENT_H__

#include <magic/object.h>
#include <magic/pararr.h>
#include <math.h>

class AnyElement;

class WireEquation {
	int mDummy;
  public:
	WireEquation () {}
	virtual double	calc	(const double* left, double x, const double* right) {return x;}
};

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//             _               ----- |                                       //
//            / \    _         |     |  ___         ___    _    |            //
//           /   \ |/ \  \   | |---  | /   ) |/|/| /   ) |/ \  -+-           //
//           |---| |   |  \  | |     | |---  | | | |---  |   |  |            //
//           |   | |   |   \_/ |____ |  \__  | | |  \__  |   |   \           //
//                        \_/                                                //
///////////////////////////////////////////////////////////////////////////////

class AnyElement {
  public:
	AnyElement (double temp=20) : mTemp (temp) {}

	double	temp	() const {return mTemp;}

	void	link	(const AnyElement* left, const AnyElement* right) {
		mLeft = left;
		mRight = right;
	}

	/** Updates the temperature. The temperature change does not
	 *  actualize before update2().
	 *
	 *  Must be overloaded.
	 **/
	virtual void	update	() {MUST_OVERLOAD}

	/** Returns the temperature change after previous update.
	 *
	 *  Must be overloaded by immediate inheritors.
	 *  
	 *  OBSERVE! The method is not valid after the update2() call!
	 **/
	virtual double	delta	() const {MUST_OVERLOAD}
	
	/** Updates the new calculated temperature as current.
	 *
	 *  Must be overloaded.
	 **/
	virtual void	update2	() {MUST_OVERLOAD}
	
	double	mTemp;
	double  mNewTemp;
  protected:
	const AnyElement* mLeft;
	const AnyElement* mRight;
};



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//       ----               o       ----- |                                  //
//      (      |   ___   |     ___  |     |  ___         ___    _    |       //
//       ---  -+-  ___| -+- | |   \ |---  | /   ) |/|/| /   ) |/ \  -+-      //
//          )  |  (   |  |  | |     |     | |---  | | | |---  |   |  |       //
//      ___/    \  \__|   \ |  \__/ |____ |  \__  | | |  \__  |   |   \      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

class StaticElement : public AnyElement {
  public:
	StaticElement (double temp) : AnyElement (temp) {}

	/** Dummy; static elements can't be modified. */
	virtual void	update	() {}
	virtual void	update2	() {}
	virtual double	delta	() const {return 0.0;}
  protected:
};



///////////////////////////////////////////////////////////////////////////////
//           |   | o           ----- |                                       //
//           | | |        ___  |     |  ___         ___    _    |            //
//           | | | | |/\ /   ) |---  | /   ) |/|/| /   ) |/ \  -+-           //
//           | | | | |   |---  |     | |---  | | | |---  |   |  |            //
//            V V  | |    \__  |____ |  \__  | | |  \__  |   |   \           //
///////////////////////////////////////////////////////////////////////////////

class WireElement : public AnyElement {
  public:
					WireElement (double temp, WireEquation* equation=NULL) : AnyElement (temp), mEquation(equation) {}
	
	virtual void	update		();
	virtual double	delta		() const {return fabs (mTemp-mNewTemp);}
	virtual void	update2		() {mTemp = mNewTemp;}

  protected:
	WireEquation*	mEquation;
};



//////////////////////////////////////////////////////////////////////////////
//         ___                   ----- |                                    //
//        /   \                  |     |  ___         ___    _    |         //
//        |      __  |/|/| |/|/| |---  | /   ) |/|/| /   ) |/ \  -+-        //
//        |     /  \ | | | | | | |     | |---  | | | |---  |   |  |         //
//        \___/ \__/ | | | | | | |____ |  \__  | | |  \__  |   |   \        //
//////////////////////////////////////////////////////////////////////////////

/** Comm element acts just like an ordinary WireElement, except that
 *  it gets its values by communication.
 **/
class CommElement : public WireElement {
  public:
	CommElement	(double temp, MPIInstance& mpi, int neighbour, WireEquation* equation=NULL)
			: WireElement (temp, equation), mMPI (mpi), mNeighbourID (neighbour) {}
	
	virtual void	update	();
	virtual void	update2	();
	
	/** Sends the data to the neighbouring CommElement in neighbouring
	 *  WireFragment, over the MPI interface.
	 *
	 *  Used by the CommElement itself autonomously, and also by the
	 *  WireFragment, to send the initial value.
	**/
	void			send	();

  protected:
	MPIInstance&	mMPI;
	int				mNeighbourID;
	double			mNeighbourTemp;

};



///////////////////////////////////////////////////////////////////////////////
//       |   | o           -----                                             //
//       | | |        ___  |          ___               ___    _    |        //
//       | | | | |/\ /   ) |---  |/\  ___|  ___  |/|/| /   ) |/ \  -+-       //
//       | | | | |   |---  |     |   (   | (   \ | | | |---  |   |  |        //
//        V V  | |    \__  |     |    \__|  ---/ | | |  \__  |   |   \       //
//                                          __/                              //
///////////////////////////////////////////////////////////////////////////////

class WireFragment : public Object {
  public:
						WireFragment	(int len, MPIInstance& mpi);

	virtual void		make			(int len);

	/** Links the neighbouring elements together as a doubly-linked
	 *  list. Any reimplementation of make must call this!
	 **/
	void				link			();

	/** Prints out the data in the fragment.
	 **/
	void				print			(FILE* out=stdout) const;

	/** Shakes hands with the neighbouring fragments (CommElements
	 *  exchange their values.
	 **/
	void				initComm		();

	/** Runs the wire.
	 *
	 *  %param epsilon Minimum delta for the elements. The run
	 *  terminates ONLY after all fragments have their delta below
	 *  this value. Of course, if this is 0.0, it never terminates.
	 *
	 *  %param maxcycles Maximum number of cycles. Value -1 causes to
	 *  run forever, unless terminated by the epsilon parameter.
	 **/
	void				run				(double epsilon=0.01, int maxcycles=-1);
	
  protected:
	Array<AnyElement>	mElements;
	MPIInstance&		mMPI;
};
#endif
