#include "mpi++.h"


///////////////////////////////////////////////////////////////////////////////
//        |   | ----  --- ---                                                //
//        |\ /| |   )  |   |    _    ____  |   ___    _    ___   ___         //
//        | V | |---   |   |  |/ \  (     -+-  ___| |/ \  |   \ /   )        //
//        | | | |      |   |  |   |  \__   |  (   | |   | |     |---         //
//        |   | |     _|_ _|_ |   | ____)   \  \__| |   |  \__/  \__         //
///////////////////////////////////////////////////////////////////////////////

MPIInstance::MPIInstance (int& argc, char**& argv) {
	MPI_Init (&argc, &argv);
	mpWorld = new MPIComm (*this, MPI_COMM_WORLD);
	initBuffer (1024);
}

MPIInstance::~MPIInstance () {
	delete mpWorld;
    MPI_Finalize(); 
}

double MPIInstance::time () const {
	return MPI_Wtime();
}

void MPIInstance::initBuffer (int len) {
	mBuffer.ensure (len);
	MPI_Buffer_attach (mBuffer.getbuffer(), mBuffer.maxLen());
}

String MPIInstance::error (int errcode) const {
	char errorStr [MPI_MAX_ERROR_STRING];
	int errorlen;
	MPI_Error_string (errcode, errorStr, &errorlen);
	return String(errorStr);
}

void MPIRequest::wait () {
	MPI_Wait (&mRequest, &mComm.mpi().mMPIStatus);

	complete ();
}

bool MPIRequest::check () {
	int flag;
	MPI_Test (&mRequest, &flag, &mComm.mpi().mMPIStatus);
	if (flag)
		complete ();
	return flag;
}

void MPIRequest::complete () {
	// Always get the length info. I'm not sure about how much
	// overhead this makes. Might be really small.
	int len;
	MPI_Get_count (&mComm.mpi().mMPIStatus, MPI_CHAR, &len);

	// Ensure string termination (there is always room for one 0 in
	// MagiClib Strings).
	mBuffer.getbuffer()[mBuffer.len=len] = 0;
}



//////////////////////////////////////////////////////////////////////////////
//                  |   | ----  ---  ___                                    //
//                  |\ /| |   )  |  /   \                                   //
//                  | V | |---   |  |      __  |/|/| |/|/|                  //
//                  | | | |      |  |     /  \ | | | | | |                  //
//                  |   | |     _|_ \___/ \__/ | | | | | |                  //
//////////////////////////////////////////////////////////////////////////////

void MPIComm::send (void* buffer, int len, MPI_Datatype datatype, int receiver) {
	MPI_Send (buffer, len, datatype, receiver, 99, mCommTag);
}

void MPIComm::send (const String& buffer, int target) {
	// Use the more generic method
	send (buffer.getbuffer(), buffer.len, MPI_CHAR, target);
}

void MPIComm::nbSend (void* buffer, int len, MPI_Datatype datatype, int target) {
	MPI_Request request;
	int errcode;
	if ((errcode=MPI_Ibsend (buffer, len, datatype, target, 99, mCommTag, &request)) != MPI_SUCCESS)
		throw mpi_error (format ("Error in MPIComm::nbSend(,,,): %s\n",
								 (CONSTR) mpi().error(errcode)));
	MPI_Wait (&request, &mMPI.mMPIStatus);
}

void MPIComm::nbSend (const String& buffer, int target) {
	MPI_Request request;
	MPI_Ibsend (buffer.getbuffer(), buffer.len, MPI_CHAR, target, 99, mCommTag, &request);
	MPI_Wait (&request, &mMPI.mMPIStatus);
}

int MPIComm::recv (void* buffer, int maxlen, MPI_Datatype datatype, int source) {
	int errcode;
	if ((errcode=MPI_Recv (buffer, maxlen, datatype, source, 99, mCommTag, & mMPI.mMPIStatus)) != MPI_SUCCESS)
		throw mpi_error (format ("Error in MPIComm::recv(,,,): %s\n",
								 (CONSTR) mpi().error(errcode)));

	// Always get the length info. I'm not sure about how much
	// overhead this makes. Might be really small.
	int len;
	MPI_Get_count (&mMPI.mMPIStatus, datatype, &len);
	return len;
}

void MPIComm::recv (String& buffer, int maxlen, int source) {
	buffer.ensure (maxlen+1);

	// Use the more generic one to actually receive it
	int len = recv (buffer.getbuffer(), maxlen, MPI_CHAR, source);

	// Ensure string termination (there is always room for one \x00 in
	// MagiClib Strings).
	buffer.getbuffer()[buffer.len=len] = 0;
}

MPIRequest* MPIComm::nbRecv (String& buffer, int maxlen, int source) {
	buffer.ensure (maxlen+1);

	MPIRequest* request = new MPIRequest (*this, buffer);
	MPI_Irecv(buffer.getbuffer(), maxlen, MPI_CHAR, source, 99,
			 mCommTag, &request->mRequest);

	return request;
}

String MPIComm::recv (int maxlen, int sender) {
	String result;
	recv (result, maxlen, sender);
	return result;
}

void MPIComm::allReduce (const void* sendBuffer, void* recvBuffer, int count, const MPI_Datatype& datatype, const MPI_Op& op) {
	MPI_Allreduce (const_cast<void*>(sendBuffer), recvBuffer,
				   count, datatype, op, mCommTag);
}

void MPIComm::barrier () {
	MPI_Barrier (mCommTag);
}

int MPIComm::getRank () const {
	int myrank;
	MPI_Comm_rank (mCommTag, &myrank);
	return myrank;
}

int MPIComm::size () const {
	int size;
    MPI_Comm_size (mCommTag, &size);
	return size;
}



//////////////////////////////////////////////////////////////////////////////
//        |   | ----  --- ___                                               //
//        |\ /| |   )  |  |  \   ___   |   ___   |         --   ___         //
//        | V | |---   |  |   |  ___| -+-  ___| -+- \   | |  ) /   )        //
//        | | | |      |  |   | (   |  |  (   |  |   \  | |--  |---         //
//        |   | |     _|_ |__/   \__|   \  \__|   \   \_/ |     \__         //
//                                                   \_/                    //
//////////////////////////////////////////////////////////////////////////////

MPIVector::MPIVector (int count, int blocklength, int stride, MPI_Datatype oldtype) {
	make (count, blocklength, stride, oldtype);
}

void MPIVector::make (int count, int blocklength, int stride, MPI_Datatype oldtype) {
	MPI_Type_vector (count, blocklength, stride, oldtype, &mDatatype);
	MPI_Type_commit (&mDatatype);
}

