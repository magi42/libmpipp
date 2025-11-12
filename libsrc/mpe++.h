#ifndef __MPEPP_H__
#define __MPEPP_H__

#include "../config.h"

#ifndef ENABLE_MPE
#error "MPE must be enabled if it is used"
#else

#include "mpi++.h"
#define MPE_GRAPHICS 1
#include <mpe.h>

class mpe_error : public myexception {
  public:
	mpe_error (const char* msg) : myexception(msg) {}
};

template <class T>
class Rectangle {
  public:
	T x1, y1, x2, y2;
};

class MPEWindow {
  public:
					MPEWindow		(MPIComm& comm, int x, int y, int xsize, int ysize,
									 const char* xserver);

	/** Returns the number of colors in the window palette. */
	int				colors			();

	/** Adds the given RGB color into the palette and returns its
	 *  palette index.
	 **/
	MPE_Color		addRGBcolor		(int r, int g, int b);

	/** Creates a sequence of colors. The neighboring colors will be
	 *  close to each other.
	 *
	 *  color_arr The result parameter. Notice: will be changed.
	 *
	 *  ncolors Number of items reserved in the color_arr.
	 **/
	void			makeColorArray	(MPE_Color* color_arr, int ncolors);

	/** Draws a point at the given coordinates, with the given
	 *  color.
	 **/
	void			drawPoint		(int x, int y, MPE_Color color);

	/** Draws a filled rectangle.
	 *
	 *  @param x,y Position
	 *  @param w,h Size
	 *  @param color Color
	 **/
	void			fillRectangle	(int x, int y, int w, int h, MPE_Color color);

	void			fillCircle		(int x, int y, int r, MPE_Color color);

	/** Flushes all recent draw operations and actually draws them. */
	void			update			();

	/** Waits until the user selects a region in the window. */
	Rectangle<int>	getDragRegion	(int button, int ratio);

	/** Waits until the user clicks somewhere in the window with the
	 *  mouse.  Click coordinates are stored in *x and *y, if they are
	 *  given.
	 **/
	void			getMousePress	(int* button, int* x=NULL, int* y=NULL);
	
	bool			getMousePressed	(int button, int* x=NULL, int* y=NULL);

	MPIComm&		comm			() {return mComm;}
	
  protected:
	MPE_XGraph	mWin;
	MPIComm&	mComm;
};

#endif
#endif
