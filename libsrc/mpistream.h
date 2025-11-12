#include <magic/Stream.h>
#include "mpi++.h"

/** For packing&unpacking data into a single send buffer.
 **/
class MPIBuffer {
  public:

							MPIIOStream	(MPIComm& comm, int buffsize=1024)
									: MPIStream (comm) {}

	/** */
	virtual MPIIOStream&	sendTo		(int rank) {mTarget=rank; return *this;}

	/** */
	virtual MPIIOStream&	flush		() {return *this;}

	const String&			buffer		() const {return mBuffer;}
	
	virtual MPIIOStream&	operator<<	(const char* str);
	virtual MPIIOStream&	operator<<	(int i);
	virtual MPIIOStream&	operator<<	(long i);
	virtual MPIIOStream&	operator<<	(double i);

  protected:
	int		mTarget;
	String	mBuffer;
};


/*
class MPIStream {
	MPIComm&	mrMPIComm;
  public:
							MPIStream	(MPIComm& comm) : mrMPI (comm) {}
};

class MPIIOStream : public iostream, public MPIStream {
  public:

							MPIIOStream	(MPIComm& comm, int buffsize=1024)
									: MPIStream (comm) {}

	virtual MPIIOStream&	sendTo		(int rank) {mTarget=rank; return *this;}
	virtual MPIIOStream&	flush		() {return *this;}
	
	// Implementations
	virtual MPIIOStream&	operator<<	(const char* str);
	virtual MPIIOStream&	operator<<	(int i);
	virtual MPIIOStream&	operator<<	(long i);
	virtual MPIIOStream&	operator<<	(double i);

  protected:
	int		mTarget;
	String	mBuffer;
};
*/
/*
class MPIIStream : public istream, public MPIStream  {
  public:
							MPIIStream	(MPIInstance& mpi) : MPIStream (mpi) {}
};

class MPIIOStream : public MPIOStream, virtual public MPIIStream {
  public:
							MPIIOStream	(MPIInstance& mpi)
									: MPIIStream (mpi), MPIOStream (mpi) {}
};
*/
