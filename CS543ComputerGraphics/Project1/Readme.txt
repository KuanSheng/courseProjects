When the program starts, it initializes GLUT, sets random numbers(for picking main drawing space), creates GLUT window, 
initializes GLEW, initializes GPU buffers, sets up shaders, assigns handlers and then go to main loop. 
This is the main architecture. 

Here comes the brief introduce to the handlers which implement functionality required by this project.

DISPLAY: 
display() -- This function handles all the drawing.  First it draws 10 thumbnails of provided polyline files at the top. 
Then it draws a picture in the main drawing space according to current system state. The initial state is 'p'. Function 
readAndDraw( ), gingerbreadMan(), and updateMainPanel() is called. 

readAndDraw is used to read a .dat file and draw the polyline as provided filename, drawing space parameters(such as start X point, 
start Y point, length, width). In this function it first read the file, then sets world window, and sets viewport according 
to aspect ratios and drawing space parameters. Then it draws the picture in use of glDrawArrays.

gingerbreadMan is used to draw the gingerbread man described in class.

updateMainPanel is used to draw the dots and lines that already modified by users in 'e', 'm', or 'd' mode. As these 
information is stored in a 2 dimensional int array, it just go over the array and draws it.

KEYBOARD INTERACTION:
keyboard() -- This function sets which state current system is as user presses the keyboard. For the viewing mode(p, t, g), 
it will clear the screen and calls the display() to redraw. Also before it calls display, the drawing space parameters are calculated.

keyboardUp() -- This function works with keyboard together to define the state when the user is pressing 'b' button. If keyboardUp() is 
called, then the user exit this state.

MOUSE INTERACTION:
myMouse() -- Implements most of the click interactions. First it judge the current state, and then does whatever required 
in that state. For example, if in state p, it first check the mouse position validation, then set which picture to be 
shown in the main drawing space. if in state e, the current mouse position is store in a 2 dimensional int array for 
updateMainPanel() to draw. If in state m or d, first calls findPoint(), then does related acts. Note as we have the 
problem of reshape, so before store the mouse position, and in the findPoint(), we need to converse the position into the 
dimension of (640, 480) which is set as initial length and initial width.

myMovedMouse() -- For dynamically shows the procedure of moving the target node and related lines. It just dynamically 
updates the node in the 2 dimensional int array.

RESHAPE:
myReshape: update the current length and width and calls the display(). In display(), viewport is properly set 
according to these parameters.

@author: Jiefeng He
  Email: jiefenghaspower@gmail.com
