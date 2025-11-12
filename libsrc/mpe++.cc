#include "mpe++.h"

MPEWindow::MPEWindow (MPIComm& comm, int x, int y, int xsize, int ysize, const char* xserver)
		: mComm (comm) {
	int errcode;
	if ((errcode=MPE_Open_graphics (&mWin, comm.getCommTag(),
									(char*) xserver, x,y, xsize,ysize, 0)) != MPE_SUCCESS)
		throw mpe_error (format ("MPI_Open_graphics error in MPEWindow::MPEWindow: %s\n",
								 (CONSTR) comm.mpi().error(errcode)));
}

int MPEWindow::colors () {
	int errcode, ncolors=0;
	if ((errcode=MPE_Num_colors(mWin, &ncolors)) != MPE_SUCCESS)
		throw mpe_error (format ("MPE_Num_colors error in MPEWindow::colors: %s\n",
								 (CONSTR) mComm.mpi().error(errcode)));
	return ncolors;
}

MPE_Color MPEWindow::addRGBcolor (int r, int g, int b) {
	MPE_Color result;
	int errcode;
	if ((errcode=MPE_Add_RGB_color (mWin, r, g, b, &result)) != MPE_SUCCESS)
		throw mpe_error (format ("MPE_add_RGB_color error in MPEWindow::addRGBcolor: %s\n",
								 (CONSTR) mComm.mpi().error(errcode)));
	return result;
}

void MPEWindow::makeColorArray (MPE_Color* color_arr, int ncolors) {
	int errcode;
	if ((errcode=MPE_Make_color_array (mWin, ncolors, color_arr)) != MPE_SUCCESS)
		throw mpe_error (format ("MPE_makeColorArray error in MPEWindow::makeColorArray: %s\n",
								 (CONSTR) mComm.mpi().error(errcode)));
}

void MPEWindow::drawPoint (int x, int y, MPE_Color color) {
	int errcode;
	if ((errcode=MPE_Draw_point (mWin, x, y, color)) != MPE_SUCCESS)
		throw mpe_error (format ("MPE_Draw_point error in MPEWindow::drawPoint: %s\n",
								 (CONSTR) mComm.mpi().error(errcode)));
}

void MPEWindow::fillRectangle (int x, int y, int w, int h, MPE_Color color) {
	int errcode;
	if ((errcode=MPE_Fill_rectangle (mWin, x, y, w, h, color)) != MPE_SUCCESS)
		throw mpe_error (format ("MPE_Fill_rectangle error in MPEWindow::fillRectangle: %s\n",
								 (CONSTR) mComm.mpi().error(errcode)));
}

void MPEWindow::fillCircle (int x, int y, int r, MPE_Color color) {
	int errcode;
	if ((errcode=MPE_Fill_circle (mWin, x, y, r, color)) != MPE_SUCCESS)
		throw mpe_error (format ("MPE_Fill_circle error in MPEWindow::fillCircle: %s\n",
								 (CONSTR) mComm.mpi().error(errcode)));
}


void MPEWindow::update () {
	int errcode;
	if ((errcode=MPE_Update (mWin)) != MPE_SUCCESS)
		throw mpe_error (format ("MPE_Update error in MPEWindow::update: %s\n",
								 (CONSTR) mComm.mpi().error(errcode)));
}

Rectangle<int> MPEWindow::getDragRegion (int button, int ratio) {
	Rectangle<int> result;
	int errcode;
	if ((errcode=MPE_Get_drag_region (mWin, button, ratio, &result.x1, &result.y1,
									  &result.x2, &result.y2)) != MPE_SUCCESS)
		throw mpe_error (format ("MPE error in MPEWindow::getDragRegion: %s\n",
								 (CONSTR) mComm.mpi().error(errcode)));
	return result;
}

void MPEWindow::getMousePress (int* button, int* x=NULL, int* y=NULL) {
	int tmpx, tmpy; // Use these dummy spare variables if x and y were not given.
	int errcode;
	if ((errcode=MPE_Get_mouse_press (mWin, x?x:&tmpx, y?y:&tmpy, button)) != MPE_SUCCESS)
		throw mpe_error (format ("MPE error in MPEWindow::getMousePress: %s\n",
								 (CONSTR) mComm.mpi().error(errcode)));
}

bool MPEWindow::getMousePressed (int button, int* x=NULL, int* y=NULL) {
	int tmpx, tmpy; // Use these dummy spare variables if x and y were not given.
	int errcode;
	int result;
	if ((errcode=MPE_Iget_mouse_press (mWin, x?x:&tmpx, y?y:&tmpy, &button, &result)) != MPE_SUCCESS)
		throw mpe_error (format ("MPE error in MPEWindow::getMousePressed: %s\n",
								 (CONSTR) mComm.mpi().error(errcode)));
	return result;
}

