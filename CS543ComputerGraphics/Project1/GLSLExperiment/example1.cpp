// Starter program for HW 0. 
// Program draws a triangle. Study the program first
// Then modify the function generateGeometry to draw a two-Dimensional Sierpinski Gasket       
// Generated using randomly selected vertices and bisection

// Angel.h is homegrown include file that also includes glew and freeglut

#include "Angel.h"
#include <time.h>  

// Number of points in polyline
const int NumPoints = 5000;

//----------------------------------------------------------------------------

// remember to prototype
void generateGeometry( void );
void initGPUBuffers( void );
void shaderSetup( void );
void display( void );
void keyboard( unsigned char key, int x, int y );
void keyboardUp(unsigned char key, int x, int y);
void myMouse( int button, int state, int x, int y);
void myMovedMouse(int x, int y);
void readAndDraw( char* fileName, int startX, int startY, int l, int w);
void gingerbreadMan(int px, int py, int l, int w);
void findPoint(int x, int y);
void drawingModeRefresh( void );
void myReshape( int W, int H );
void updateMainPanel( void );

typedef vec2 point2;

using namespace std;

// Array for polyline
static int iniLen=640;
static int iniWidth=480;
static int len=iniLen;
static int width=iniWidth;
point2 points[NumPoints];
GLuint program;
GLuint ProjLoc;
int numPolygon=0, numPoint=0;
mat4 ortho;
char names[10][15]={"dino.dat", "birdhead.dat", "dragon.dat", "house.dat", "knight.dat", "rex.dat", "scene.dat", "usa.dat", "vinci.dat", "vinci.dat"};
int randNumb;
int randNumbs[6][6];
unsigned char myKey='p';
bool isBpressed=false;
unsigned char preKey='h';
int drawNumb=0;

point2 drawLine[2];
int groupNumb=0;
int index=0;
int count[50];
point2 drawLines[50][100];
int pointGroupNumb=-1;
int pointIndex=-1;
bool isPointFound=false;
bool foundToDelete=false;

void generateGeometry( void )
{
		// ***************** Important note ***************** //
	// please refer to OpenGL documentation before coding
	// many old functions have been depricated
	// though they will probably still work on most machines others
	// will require compatiability mode to be used

	// Specifiy the vertices for a triangle
	// for 3d points use vec3 and change your vPosition attribute appropriately

	/*
	points[0] = point2( -0.5, -0.5 );
	points[1] = point2( 0.0, 0.5 );
	points[2] = point2( 0.5, -0.5 );
	*/
}

void initGPUBuffers( void )
{
	    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW );
}

void shaderSetup( void )
{
	    // Load shaders and use the resulting shader program
    program = InitShader( "vshader1.glsl", "fshader1.glsl" );
    glUseProgram( program );

    // Initialize the vertex position attribute from the vertex shader
    GLuint loc = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( loc );
    glVertexAttribPointer( loc, 2, GL_FLOAT, GL_FALSE, 0,
                           BUFFER_OFFSET(0) );

	// sets the default color to clear screen
    glClearColor( 1.0, 1.0, 1.0, 1.0 ); // white background

}

void readAndDraw( char* fileName, int startX, int startY, int l, int w )
{
	FILE * fp;
	float x,y;
	float left, right, bottom, top;

	fp = fopen(fileName, "r");
	if (fp == NULL)
			exit(EXIT_FAILURE);
	if(strcmp(fileName, "dino.dat")==0)
	{
		left=0.0;
		right=640.0;
		bottom=0.0;
		top=480.0;
	}else{
		
		char str[80];
		fscanf (fp,"%s",str);
		while(strchr(str,'*')==NULL){
			fscanf(fp,"%s", str);
		}
		fscanf(fp, "%f %f %f %f", &left, &top, &right, &bottom);
	}
	
	ProjLoc = glGetUniformLocation( program, "Proj");
	ortho = Ortho2D(left, right, bottom, top);
	glUniformMatrix4fv( ProjLoc, 1, GL_FALSE, ortho);
	float R=(right-left)/(top-bottom);
	float r=l*1.0/w; //Note: if use r=1/w then will have the problem of losing digits after decimal!

	if(R>r){
		glViewport(startX, startY, l, l/R);
	}
	else if(R<r){
		
		glViewport(startX, startY, w*R, w);
	}
	else{
		glViewport(startX, startY, l, w);
	}

	fscanf(fp, "%d", &numPolygon);
	for(int i=0; i<numPolygon; i++){
		fscanf(fp, "%d", &numPoint);
		for(int j=0; j<numPoint; j++){
			fscanf(fp, "%f %f", &x, &y);
			points[j]=point2(x, y);
		}

		glBufferData( GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW );
		glDrawArrays( GL_LINE_STRIP, 0, numPoint);	
	}
	glFlush();
    fclose(fp);
}

void gingerbreadMan(int px, int py, int l, int w)
{
	int M=40;
	int L=3;
    int qx, qy;

	ProjLoc = glGetUniformLocation( program, "Proj");
	ortho = Ortho2D(0.0, 640.0, 0.0, 480.0);
	double R=640.0/480.0;
	glUniformMatrix4fv( ProjLoc, 1, GL_FALSE, ortho);

	for(int i=0; i<5000; i++){
		qx=M*(1+L*2)-py+abs(px-L*M);
		qy=px;
		points[i]=point2(qx, qy);
		px=qx;
		py=qy;
	}

	float r=l*1.0/w;
	if(R>r)
		glViewport(0, 0, l, l/R);
	else if(R<r)
		glViewport(0, 0, w*R, w);
	else
		glViewport(0, 0, l, w);
	glBufferData( GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW );
	glDrawArrays( GL_POINTS, 0, 5000);	
	
	glFlush();
}

//----------------------------------------------------------------------------
// this is where the drawing should happen
void
display( void )
{
	// TIP 1: remember to enable depth buffering when drawing in 3d

	// TIP 2: If you want to be sure your math functions are right, 
	// prototype your operations in Matlab or Mathematica first to verify correct behavior
	// both programs support some sort of runtime interface to C++ programs

	// TIP3: check your graphics specs. you have a limited number of loop iterations, storage, registers, texture space etc.
	

	// TIP4: avoid using glTranslatex, glRotatex, push and pop
	// pass your own view matrix to the shader directly
	// refer to the latest OpenGL documentation for implementation details
	
    //glClear( GL_COLOR_BUFFER_BIT );     // clear the window
	// GL_LINE_STRIP   GL_POLYGON
    //glDrawArrays( GL_LINE_LOOP, 0, NumPoints );    // draw the points
	//glDrawArrays( GL_POINTS, 0, NumPoints );
	int lenTemp, widthTemp;

	for(int i=0; i<10; i++){
		//glViewport(64*i, 432, 64, 48);
		lenTemp=len/10.0;
		widthTemp=lenTemp*(480.0/640.0);
		//printf("%d %d\n", lenTemp, widthTemp);
		readAndDraw(names[i], lenTemp*i, width-widthTemp, lenTemp, widthTemp);
	}

	switch(myKey)
	{		
		case 't':
			lenTemp=len/10.0;
			widthTemp=lenTemp*(480.0/640.0);
			widthTemp=(width-widthTemp)/6.0;
			lenTemp=len/6.0;
			for(int i=0; i<6; i++)
				for(int j=0; j<6; j++)
				{
					readAndDraw(names[randNumbs[i][j]], lenTemp*i, widthTemp*j, lenTemp, widthTemp);
				}
			break;
		case 'e':
			updateMainPanel();
			break;
		case 'm':
			updateMainPanel();
			break;
		case 'd':
			updateMainPanel();
			break;
		case 'g':
			gingerbreadMan(115, 121, len, width-widthTemp);
			break;
		default:
			//glViewport(0, 0, 640, 432);
			//printf("%d %d\n", len, width-widthTemp);
			readAndDraw(names[randNumb], 0, 0, len, width-widthTemp);
	}

	preKey=myKey;
}

//----------------------------------------------------------------------------

// keyboard handler
void
keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
	case 'p':
		myKey='p';
		glClear( GL_COLOR_BUFFER_BIT );
		display();
		break;
	case 'g':
		myKey='g';
		glClear( GL_COLOR_BUFFER_BIT );
		display();
		break;
	case 'b':
		isBpressed=true;
		break;
	case 'e':
		drawNumb=0;
		groupNumb=-1;
		pointGroupNumb=-1;
		foundToDelete=false;
		isPointFound=false;
		myKey='e';
		glClear( GL_COLOR_BUFFER_BIT );
		display();
		break;
	//State e is the initial drawing state and states (m and d) are only accessible from state e. 
	case 'm':
		if(preKey=='e' || preKey=='d' || preKey=='m'){
			myKey='m';
		}
		break;
	case 'd':
		if(preKey=='e' || preKey=='d' || preKey=='m')
			myKey='d';
		break;
	case 't':
		myKey='t';
		for(int i=0; i<6; i++)
			for(int j=0; j<6; j++)
				randNumbs[i][j]=rand()%9;
		glClear( GL_COLOR_BUFFER_BIT );
		display();
		break;
    case 033:
        exit( EXIT_SUCCESS );
        break;
    }
}

void keyboardUp(unsigned char key, int x, int y)
{
    if(key=='b')
		isBpressed=false;
}

void findPoint(int x, int y)
{
	int tempLen, tempWidth;
	float R=iniLen*1.0/iniWidth;
    float r=len*1.0/width;
	y=width-y;
	if(R>r){
		tempLen=len;
		tempWidth=len/R;
	}
	else if(R<r){
		tempLen=width*R;
		tempWidth=width;
	}else{
		tempLen=len;
		tempWidth=width;
	}

	x=x*1.0/tempLen*iniLen;
	y=y*1.0/tempWidth*iniWidth;
	//printf("\n*************(%d, %d)*******************\n", x, y);

	for(int i=0; i<=groupNumb; i++)
		for(int j=0; j<=count[i]; j++)  //for(int j=0; j<=count[groupNumb]; j++)  finally find this bug!!!
		{
			//printf("(%f, %f) ", drawLines[i][j].x, drawLines[i][j].y);
			if(pow(x-drawLines[i][j].x+0.0, 2.0)+pow(y-drawLines[i][j].y+0.0, 2.0) <100)
			{
				pointGroupNumb=i;
				pointIndex=j;
				if(myKey=='m')
					isPointFound=true;
				else if(myKey=='d')
					foundToDelete=true;
				//printf("\nPoint (%d, %d) has been found!\n", x, y);
				return;
			}
		}
		//printf("\n");
	pointGroupNumb=-1;
	pointIndex=-1;
}

void myMouse( int button, int state, int x, int y)
{
	int lenTemp, widthTemp;
	lenTemp=len/10;
	widthTemp=lenTemp*(480.0/640);

	if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN)
	{
		switch(myKey)
		{
			case 'p':
				if(y<=widthTemp)
					randNumb=x/lenTemp;
				glClear( GL_COLOR_BUFFER_BIT );
				break;
			case 'e':
				if(drawNumb<100 && y>widthTemp){
					if(drawNumb==0)
					{
						groupNumb=0;
						index=-1;
						for(int i=0; i<50; i++)
							count[i]=0;
					}
					int tempLen, tempWidth;
					float R=iniLen*1.0/iniWidth;
					float r=len*1.0/width;
					y=width-y;
					if(R>r){
						tempLen=len;
						tempWidth=len/R;
					}
					else if(R<r){
						tempLen=width*R;
						tempWidth=width;
					}else{
						tempLen=len;
						tempWidth=width;
					}

					x=x*1.0/tempLen*iniLen;
					y=y*1.0/tempWidth*iniWidth;
					if(isBpressed==true){

						index=0;
						drawLines[++groupNumb][0]=point2(x, y);

						float R=iniLen*1.0/iniWidth;
						float r=len*1.0/width;

						if(R>r){
							glViewport(0, 0, len, len/R);
						}
						else if(R<r){
							glViewport(0, 0, width*R, width);
						}
						else{
							glViewport(0, 0, len, width);
						}
						ProjLoc = glGetUniformLocation( program, "Proj");
						ortho = Ortho2D(0.0, iniLen+0.0, 0.0, iniWidth+0.0);
						glUniformMatrix4fv( ProjLoc, 1, GL_FALSE, ortho);
						glBufferData( GL_ARRAY_BUFFER, sizeof(drawLines[groupNumb]), &drawLines[groupNumb], GL_STATIC_DRAW );
						glDrawArrays( GL_POINTS, 0, 1 );	
						glFlush();

					}else{

						drawLines[groupNumb][++index]=point2(x, y);
						count[groupNumb]=index;

						if(drawNumb==0)
						{
							float R=iniLen*1.0/iniWidth;
							float r=len*1.0/width;

							if(R>r){
								glViewport(0, 0, len, len/R);
							}
							else if(R<r){
								glViewport(0, 0, width*R, width);
							}
							else{
								glViewport(0, 0, len, width);
							}
							ProjLoc = glGetUniformLocation( program, "Proj");
							ortho = Ortho2D(0.0, iniLen+0.0, 0.0, iniWidth+0.0);
							glUniformMatrix4fv( ProjLoc, 1, GL_FALSE, ortho);
							glBufferData( GL_ARRAY_BUFFER, sizeof(drawLines[groupNumb]), &drawLines[groupNumb], GL_STATIC_DRAW );
							glDrawArrays( GL_POINTS, 0, 1 );	
							glFlush();
						}
					}
					drawNumb++;
				}
				break;
			case 'm':
				findPoint(x, y);
				break;
			case 'd':
				findPoint(x, y);
				if(foundToDelete==true)
				{
					for(int i=pointIndex; i<count[pointGroupNumb];i++)
					{
						drawLines[pointGroupNumb][i]=drawLines[pointGroupNumb][i+1];
					}
					count[pointGroupNumb]=count[pointGroupNumb]-1;;
					pointGroupNumb=-1;
				}
				drawingModeRefresh();
				break;
		}
	}
	if(button==GLUT_LEFT_BUTTON && state==GLUT_UP)
	{
		switch(myKey)
		{
			case 'm':
				isPointFound=false;
				if(pointGroupNumb!=-1)
				{
					pointGroupNumb=-1;
					pointIndex=-1;
				}
				break;
		}
	}
}

void updateMainPanel()
{
	point2 temp[2];
	float R=iniLen*1.0/iniWidth;
	float r=len*1.0/width;

	if(R>r){
		glViewport(0, 0, len, len/R);
	}
	else if(R<r){
		glViewport(0, 0, width*R, width);
	}
	else{
		glViewport(0, 0, len, width);
	}

	ProjLoc = glGetUniformLocation( program, "Proj");
	ortho = Ortho2D(0.0, iniLen+0.0, 0.0, iniWidth+0.0);
	glUniformMatrix4fv( ProjLoc, 1, GL_FALSE, ortho);
	for(int i=0; i<=groupNumb; i++)
		for(int j=1; j<=count[i]; j++)
		{
			temp[0]=drawLines[i][j-1];
			temp[1]=drawLines[i][j];
			glBufferData( GL_ARRAY_BUFFER, sizeof(temp), temp, GL_STATIC_DRAW );
			glDrawArrays( GL_LINES, 0, 2 );	
		}
	foundToDelete=false;
	glFlush();
}

void drawingModeRefresh()
{
	glClear( GL_COLOR_BUFFER_BIT );
	display();
	updateMainPanel();
}

void myMovedMouse(int x, int y)
{
	if(isPointFound==true)
	{
		int tempLen, tempWidth;
		float R=iniLen*1.0/iniWidth;
		float r=len*1.0/width;
		y=width-y;
		if(R>r){
			tempLen=len;
			tempWidth=len/R;
		}
		else if(R<r){
			tempLen=width*R;
			tempWidth=width;
		}else{
			tempLen=len;
			tempWidth=width;
		}

		x=x*1.0/tempLen*iniLen;
		y=y*1.0/tempWidth*iniWidth;
		drawLines[pointGroupNumb][pointIndex]=point2(x, y);
		drawingModeRefresh();
	}
}

void myReshape( int W, int H )
{
	len=W;
	width=H;
	display();
}

//----------------------------------------------------------------------------
// entry point
int
main( int argc, char **argv )
{
	// init glut
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_SINGLE );
    glutInitWindowSize( len, width );

	srand (time(NULL));
	randNumb=rand()%9;
	for(int i=0; i<6; i++)
		for(int j=0; j<6; j++)
			randNumbs[i][j]=rand()%9;
    // If you are using freeglut, the next two lines will check if 
    // the code is truly 3.2. Otherwise, comment them out
    //glutInitContextVersion( 3, 3 );
    //glutInitContextProfile( GLUT_CORE_PROFILE );

	// create GLUT window for drawing
    glutCreateWindow( "Project One: Jiefeng He" );

	// init glew
    glewInit();
	
    initGPUBuffers( );
    shaderSetup( );

	// assign handlers
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
	// should add menus
	//glutCreateMenu(myMenu);  //lecture02_p1
	// add mouse handler
	glutMouseFunc(myMouse);
	glutMotionFunc(myMovedMouse);
	glutReshapeFunc(myReshape);
	glutKeyboardUpFunc( keyboardUp );

	// enter the drawing loop
	// frame rate can be controlled with 
    glutMainLoop();
    return 0;
}
