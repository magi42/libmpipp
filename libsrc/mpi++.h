/***************************************************************************
 *   copyright            : (C) 2000 by Marko Grönroos                     *
 *   email                : magi@iki.fi                                    *
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
 *
 * Here is defined the mpi++, a C++ interface to MPI.
 *
 **/

#ifndef __MPIPP_H__
#define __MPIPP_H__

#include <magic/object.h>
#include <magic/cstring.h>

#include <mpi.h>

class MPIInstance;
class MPIRequest;
class MPIComm;
class MPIDatatype;
class MPIVector;

/** A very simple exception class that passes an error message. For
 *  some reason my standard Exception class didn't work with
 *  MPI/MPE. Very strange.
 **/
class myexception : public exception {
	const String mMsg;
  public:
	myexception (const char* msg) : mMsg (msg) {}
	const char* what () const {return (CONSTR) mMsg;}
};

/** MPI exceptions. */
class mpi_error : public myexception {
  public:
	mpi_error (const char* msg) : myexception(msg) {}
};

#ifndef TAGTYPE_IS_INT
typedef int MPITAGTYPE;
#else
typedef _comm* MPITAGTYPE;
#endif



///////////////////////////////////////////////////////////////////////////////
//        |   | ----  --- ---                                                //
//        |\ /| |   )  |   |    _    ____  |   ___    _    ___   ___         //
//        | V | |---   |   |  |/ \  (     -+-  ___| |/ \  |   \ /   )        //
//        | | | |      |   |  |   |  \__   |  (   | |   | |     |---         //
//        |   | |     _|_ _|_ |   | ____)   \  \__| |   |  \__/  \__         //
///////////////////////////////////////////////////////////////////////////////

/** MPI process data; a singleton (although that is not explicit).
 *
 *  Design Patterns: Singleton.
 **/
class MPIInstance : public Object {
  public:
						MPIInstance		(int& argc, char**& argv);
						~MPIInstance	();

	/** Returns a time stamp. */
	double				time			() const;

	/** Returns the MPI_COMM_WORLD-communicator. */
	MPIComm&			world			() {return *mpWorld;}

	/** Returns MPI status data block. */
	const MPI_Status&	status			() const {return mMPIStatus;}

	/** Returns an MPI error code to a string. */
	String				error			(int errcode) const;

	/** Initializes the buffer to the given length.
	 **/
	void				initBuffer		(int len);
	
  protected:
	MPIComm*		mpWorld;
	MPI_Status		mMPIStatus;
	String			mBuffer;

	friend MPIComm;
	friend MPIRequest;
};



//////////////////////////////////////////////////////////////////////////////
//          |   | ----  --- ----                                            //
//          |\ /| |   )  |  |   )  ___              ___   ____  |           //
//          | V | |---   |  |---  /   )  __  |   | /   ) (     -+-          //
//          | | | |      |  | \   |---  /  \ |   | |---   \__   |           //
//          |   | |     _|_ |  \   \__  \__|  \__!  \__  ____)   \          //
//                                         |                                //
//////////////////////////////////////////////////////////////////////////////

/** Typically a pending non-blocking receive request.
 **/
class MPIRequest {
  public:
	/** Creates a pending request.
	 *
	 *  buff A reference to the buffer used for the request. For
	 *  example, if the request is a nbRecv, the data will be stored
	 *  in this buffer when it arrives.
	 **/
					MPIRequest		(MPIComm& comm, String& buff)
							: mComm (comm), mBuffer (buff) {}

	/** Waits until the request has been completed. */
	void			wait			();

	/** Checks if the request has been completed, and returns true if
	 *  it has, otherwise false.
	 **/
	bool			check			();
	
  protected:
	MPI_Request		mRequest;
	String&			mBuffer;
	MPIComm&		mComm;

	void 			complete		();

	friend MPIComm;
};



//////////////////////////////////////////////////////////////////////////////
//                  |   | ----  ---  ___                                    //
//                  |\ /| |   )  |  /   \                                   //
//                  | V | |---   |  |      __  |/|/| |/|/|                  //
//                  | | | |      |  |     /  \ | | | | | |                  //
//                  |   | |     _|_ \___/ \__/ | | | | | |                  //
//////////////////////////////////////////////////////////////////////////////

/** An MPI communicator object.
 **/
class MPIComm : public Object {
  public:

	/** Creates a communications channel with the given comm tag.
	 **/
					MPIComm			(MPIInstance& mpi, MPITAGTYPE commtag)
							: mMPI (mpi), mCommTag (commtag) {}
	
	/** Sends a message. Blocking. */
	void			send			(void* buffer, int len, MPI_Datatype datatype, int receiver);
	
	/** Sends a string buffer. Blocking. */
	void			send			(const String& buffer, int receiver);

	/** Sends a message. Blocking. */
	void			nbSend			(void* buffer, int len, MPI_Datatype datatype, int receiver);

	/** Sends a string buffer. Non-blocking. */
	void			nbSend			(const String& buffer, int receiver);

	/** Receives a message. Blocking. */
	int				recv			(void* buffer, int maxlen, MPI_Datatype datatype, int source);

	/** Receives a string buffer. Blocking. */
	void			recv			(String& buffer, int maxlen, int sender);

	/** Receives a message. Non-blocking.
	 *
	 *  Returns an MPIRequest object that can be used to wait()
	 *  or check() if the message has yet been received.
	 **/
	MPIRequest*		nbRecv			(String& buffer, int maxlen, int sender);

	/** Coating for the other recv. */
	String			recv			(int maxlen, int sender);

	/** Performs an operation with all processors. */
	void			allReduce		(const void* sendBuffer, void* recvBuffer, int count, const MPI_Datatype& datatype, const MPI_Op& op);

	/** Blocks the calling process until all processes in the comm
	 *  group have called this method.
	 *
	 *  Involves no data transmission.
	 **/
	void			barrier			();
	
	/** Returns the MPI process rank of the current process. */
	int				getRank			() const;

	/** Returns the COMM tag. */
	MPITAGTYPE		getCommTag		() const {return mCommTag;}

	/** Returns the number of processor in the given comm channel */
	int				size			() const;

	/** Returns the (well, singleton) MPI object. */
	MPIInstance&	mpi				() {return mMPI;}

  protected:
	MPIInstance&	mMPI;
	MPITAGTYPE		mCommTag;
};



//////////////////////////////////////////////////////////////////////////////
//        |   | ----  --- ___                                               //
//        |\ /| |   )  |  |  \   ___   |   ___   |         --   ___         //
//        | V | |---   |  |   |  ___| -+-  ___| -+- \   | |  ) /   )        //
//        | | | |      |  |   | (   |  |  (   |  |   \  | |--  |---         //
//        |   | |     _|_ |__/   \__|   \  \__|   \   \_/ |     \__         //
//                                                   \_/                    //
//////////////////////////////////////////////////////////////////////////////

class MPIDatatype : public Object {
  public:
	const MPI_Datatype&	getType		() const {return mDatatype;}

  protected:
	MPIDatatype	() {}
	MPI_Datatype	mDatatype;
  private:
};

class MPIVector : public MPIDatatype {
  public:
				MPIVector	() {}
				MPIVector	(int count, int blocklength, int stride, MPI_Datatype oldtype);
	void		make		(int count, int blocklength, int stride, MPI_Datatype oldtype);

  private:
};

#endif
