
// A scroll bar is a composit element that consists of a display frame which
//   contains three buttons. Two buttons are positioned at either end of the
//   bar and the third formw the "thumb" within the scroll bar.


//******************************************************
// Function: xwsWinCreateScrollBar - Create a scroll bar
// Returned: 0 if normal or negative error code if error
//******************************************************

;This function uses the Pascal calling sequence

long xwsWinCreateScrollBar(
	WIN   *parent,		// Parent window
	long   xpos,		// X position
	long   ypos,		// Y position
	long   xsize,		// X size (width)
	long   ysize,		// Y size (height)
	PAR   *parms,		// Parameter structure
	long (*event)(WIN *win, long arg1, long arg2, long arg3, long arg3),
						// Pointer to caller's event function
	long   evmask,		// Event mask bits
	EDB   *edb,			// Pointer to caller's environment data block
	WIN  **pwin)		// Pointer to location to receive address of
						//   the WIN created

{







}
