### Windows OpenGL test program to check 10 bit support ###

Creates a window that shown a black to white gradient. The lower
half is forced to 8 bit precision, while no such limitation is
set on the top half.

The opengl context will be initialized to 10 bit color, so if both
the monitor and graphics card support it, banding will be seen in the
bottom half of the window and a smooth gradient will be seen in the
top half.
    
Note that you must make sure that no dithering is enabled.

