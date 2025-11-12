#include "mpistream.h"

MPIIOStream::MPIIOStream (MPIComm& comm, int buffsize)
		: MPIStream (comm) {
	mBuffer.reserve (buffsize);
}


MPIIOStream& MPIIOStream::operator<<	(const char* str) {
	printf ("%s", str);
}

MPIIOStream& MPIIOStream::operator<<	(int i) {
	printf ("%d", i);
}

MPIIOStream& MPIIOStream::operator<<	(long i) {
	printf ("%d", i);
}

MPIIOStream& MPIIOStream::operator<<	(double i) {
	printf ("%g", i);
}
