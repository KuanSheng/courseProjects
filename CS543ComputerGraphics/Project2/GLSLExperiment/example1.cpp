// Two-Dimensional Sierpinski Gasket       
// Generated using randomly selected vertices and bisection
#include "Angel.h"

//----------------------------------------------------------------------------
int width = 0;
int height = 0;
static float scaleDividor = 5000.0;
static int time=0;
const int fileNum=43;
static float relativeLength;

// remember to prototype
void generateGeometry( void );
void display( void );
void keyboard( unsigned char key, int x, int y );
void quad( int a, int b, int c, int d );
void colorcube(void);
void draw(void);
void readFile( char* fileName);
void readData(void);
void idle(void);
void initalizePosition( void );
void calmDown(void);
void nextPicture(void);
void updateBoundingBox(void);

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

// handle to program
GLuint program;
GLuint vColor;
GLuint buffer;
Angel::mat4 modelMatTemp;

using namespace std;

float xMin, xMax, yMin, yMax, zMin, zMax;
char* fileNames[fileNum]={"airplane.ply","ant.ply","apple.ply","balance.ply","beethoven.ply","big_atc.ply","big_dodge.ply",
"big_porsche.ply","big_spider.ply","canstick.ply","chopper.ply","cow.ply","dolphins.ply","egret.ply","f16.ply",
"footbones.ply","fracttree.ply","galleon.ply","hammerhead.ply","helix.ply","hind.ply","kerolamp.ply","ketchup.ply",
"mug.ply","part.ply","pickup_big.ply","pump.ply","pumpa_tb.ply","sandal.ply","saratoga.ply","scissors.ply","shark.ply",
"steeringweel.ply","stratocaster.ply","street_lamp.ply","teapot.ply","tennis_shoe.ply","tommygun.ply","trashcan.ply",
"turbine.ply","urn2.ply","walkman.ply","weathervane.ply"};
int index=0;
int NumVertices;
int numberOfVertices;
int numberOfPolygons;
point4 vertices[50000];
point4 points[50000];
point4 points2[50000]; //using for pulsing
point4 points3[40000]; //using for drawing normal
point4 points4[24];
vec4 tmp1, tmp2;
vec4 faceDirect[50000];
color4 colors[50000];
bool xPositiveMove=false;
bool yPositiveMove=false;
bool zPositiveMove=false;
bool xNegativeMove=false;
bool yNegativeMove=false;
bool zNegativeMove=false;
bool rotation=false;
float xAxis=0.0f;
float yAxis=0.0f;
float zAxis=0.0f;
float xTinnyMove=0.0f;
float yTinnyMove=0.0f;
float zTinnyMove=0.0f;
float theta=0.0f;
bool alternation=false;
bool pulsing=false;
bool drawingFaceNormal=false;
bool drawingBoundingBox=false;

color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};

void readFile( char* fileName)
{
	FILE * fp;
	char *pch;
	float x,y,z;

	xMin=100000.0;
	xMax=-100000.0;
	yMin=xMin;
	yMax=xMax;
	zMin=xMin;
	zMax=xMax;

	fp = fopen(fileName, "r");
	if (fp == NULL)
			exit(EXIT_FAILURE);
	char str[80];
	fscanf (fp,"%s",str);
	if(strcmp(str, "ply")!=0)
		exit(EXIT_FAILURE);
	fgets(str, sizeof(str), fp); //skip first line
	fgets(str, sizeof(str), fp); //skip second line
	fgets(str, sizeof(str), fp); //element vertex 758
	pch = strtok(str, " "); //element
	pch = strtok(NULL, " "); //vertex
	pch = strtok(NULL, " "); //get the number
	numberOfVertices = atoi(pch);
	fgets(str, sizeof(str), fp); //property float 32 x
	fgets(str, sizeof(str), fp); //property float 32 y
	fgets(str, sizeof(str), fp); //property float 32 z
	fgets(str, sizeof(str), fp); //element face 1140
	pch = strtok(str, " "); //element
	pch = strtok(NULL, " "); //face
	pch = strtok(NULL, " "); //get the number
	numberOfPolygons = atoi(pch);
	fgets(str, sizeof(str), fp); //property list uint8 int32 vertex_indices
	fgets(str, sizeof(str), fp); //end_header
	for(int i=0; i<numberOfVertices; i++){
		fscanf(fp, "%f %f %f", &x, &y, &z);
		vertices[i] = point4(x,  y,  z, 1.0);
		if(xMin>x)	xMin=x;
		if(yMin>y)	yMin=y;
		if(zMin>z)	zMin=z;
		if(xMax<x)	xMax=x;
		if(yMax<y)	yMax=y;
		if(zMax<z)	zMax=z;
	}
	xTinnyMove=(xMax-xMin)/scaleDividor;
	yTinnyMove=(yMax-yMin)/scaleDividor;
	zTinnyMove=(zMax-zMin)/scaleDividor;
	relativeLength = sqrt(pow(xMax-xMin, 2)+pow(yMax-yMin, 2)+pow(zMax-zMin, 2));

	//update the position to center
	Angel::mat4 modelMat = Angel::Translate(-(xMin+xMax)/2, -(yMin+yMax)/2, -(zMin+zMax)/2);
	for(int i=0; i<numberOfVertices; i++){
		vertices[i] = modelMat*vertices[i];
	}
	
	NumVertices=numberOfPolygons*3;
	int index=0;
	int triangle;
	int one,two,three;
	for(int i=0; i<numberOfPolygons; i++){
		fscanf(fp, "%d %d %d %d", &triangle, &one, &two, &three);
		if(triangle!=3)
			exit(EXIT_FAILURE);
		points[index] = vertices[one]; 
		index++; 
		points[index] = vertices[two];
		index++;
		points[index] = vertices[three]; 
		index++;

		//update face direct
		tmp1 = vertices[two] - vertices[one];
		tmp2 = vertices[three] - vertices[one];
		faceDirect[index-3] = normalize(cross(tmp1, tmp2));
		faceDirect[index-3].w = 0.0;
		faceDirect[index-2] = faceDirect[index-3];
		faceDirect[index-1] = faceDirect[index-2];

		//update this for drawing normals
		points3[2*i].x = (vertices[one].x+vertices[two].x+vertices[three].x)/3.0;
		points3[2*i].y = (vertices[one].y+vertices[two].y+vertices[three].y)/3.0;
		points3[2*i].z = (vertices[one].z+vertices[two].z+vertices[three].z)/3.0;
		points3[2*i].w = 1.0;
		points3[2*i+1] = points3[2*i] + faceDirect[index-3]*relativeLength*0.02;
	}

	//duplicate data
	for(int i=0; i<NumVertices; i++){
		points2[i]=points[i];
	}

	updateBoundingBox();
	//printf("%f %f %f %f %f %f\n", xMin, xMax, yMin, yMax, zMin, zMax);
    fclose(fp);
}

void generateGeometry( void )
{	
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );

	// Load shaders and use the resulting shader program
    program = InitShader( "vshader1.glsl", "fshader1.glsl" );
    glUseProgram( program );
     // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

    vColor = glGetAttribLocation( program, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );

	// sets the default color to clear screen
    glClearColor( 1.0, 1.0, 1.0, 1.0 ); // white background
}

void readData(){
	char filePath[30];
	strcpy (filePath,"ply_files\\");
	strcat (filePath,fileNames[index]);
	readFile( filePath );
}

void draw(void)
{
	glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
 
	glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
	 
	// change to GL_FILL
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	// draw functions should enable then disable the features 
	// that are specifit the themselves
	// the depth is disabled after the draw 
	// in case you need to draw overlays
	glEnable( GL_DEPTH_TEST );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	//printf("%d\n", NumVertices);
	glDisable( GL_DEPTH_TEST ); 

	if(drawingFaceNormal==true){
		glBufferData( GL_ARRAY_BUFFER, sizeof(points3) + sizeof(colors), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points3), points3 );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points3), sizeof(colors), colors );
 
		glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points3)) );
		
		glDrawArrays( GL_LINES, 0, numberOfPolygons*2);
	}

	if(drawingBoundingBox==true){
		glBufferData( GL_ARRAY_BUFFER, sizeof(points4) + sizeof(colors), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points4), points4 );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points4), sizeof(colors), colors );
 
		glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points4)) );
		
		glDrawArrays( GL_LINES, 0, 24);
	}
}

// this is where the drawing should happen
void display( void )
{
	// remember to enable depth buffering when drawing in 3d

	// TIP1: if you get bogged down with many matrix operations
	// or need to perform some complexed fitting or solving
	// it may be time to include some package like diffpac or lapack
	// http://www.netlib.org/lapack/#_lapack_for_windows
	// http://www.diffpack.com/

	// TIP2: if you feel that GLSL is too restrictive OpenCL or CUDA can be
	// used to generate images or other information, they support interface to OpenGL
	// http://www.khronos.org/registry/cl/

	// TIP3: prototype your operations in Matlab or Mathematica first to verify correct behavior
	// both programs support some sort of runtime interface to C++ programs

	// TIP4: check your graphics specs. you have a limited number of loop iterations, storage, registers, texture space etc.
	
	// TIP5: take a look at the "Assembly" generated from the opengl compilers, it might lead you to some optimizations
	// http://http.developer.nvidia.com/Cg/cgc.html

	// avoid using glTranslatex, glRotatex, push and pop
	// pass your own view matrix to the shader directly
	// refer to the latest OpenGL documentation for implementation details

	// PROTIP1: You can access the depth buffer value and screen location at each fragment
	// in the fragment shader, go wild

	// PROTIP2: Render stuff to texture, then run filters on the texture in a second pass to 
	// produce cool effects
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );     // clear the window

	// PROTIP4: Do not set the near and far plane too far appart!
	// depth buffers do not have unlimited resolution
	// surfaces will start to fight as they come nearer to each other
	// if the planes are too far appart (quantization errors :(   )

	// PROTIP5: If you must have extreme distances, use multiple render passes
	// which divide the entire scene into adjacent viewing volumes
	// render far away volumes first
	// also scale your objects appropriatly, dont use scales at the upper or lower bounds
	// of floating point precision

	// WARNING1: I believe Angel::transpose(...) does not transpose a mat4, but just returns
	// an identical matrix, can anyone verify this?
	Angel::mat4 perspectiveMat = Angel::Perspective((GLfloat)45.0, (GLfloat)width/(GLfloat)height, (GLfloat)0.01, (GLfloat) 999.0);

	float viewMatrixf[16];
	viewMatrixf[0] = perspectiveMat[0][0];viewMatrixf[4] = perspectiveMat[0][1];
	viewMatrixf[1] = perspectiveMat[1][0];viewMatrixf[5] = perspectiveMat[1][1];
	viewMatrixf[2] = perspectiveMat[2][0];viewMatrixf[6] = perspectiveMat[2][1];
	viewMatrixf[3] = perspectiveMat[3][0];viewMatrixf[7] = perspectiveMat[3][1];

	viewMatrixf[8] = perspectiveMat[0][2];viewMatrixf[12] = perspectiveMat[0][3];
	viewMatrixf[9] = perspectiveMat[1][2];viewMatrixf[13] = perspectiveMat[1][3];
	viewMatrixf[10] = perspectiveMat[2][2];viewMatrixf[14] = perspectiveMat[2][3];
	viewMatrixf[11] = perspectiveMat[3][2];viewMatrixf[15] = perspectiveMat[3][3];
	
	Angel::mat4 modelMat = Angel::identity();
	//modelMat = modelMat * Angel::Translate(0.0, 0.0, -2.0f) * Angel::RotateY(45.0f) * Angel::RotateX(35.0f);
	modelMat =  modelMat * Angel::Translate(0, 0, -relativeLength*1.35);
	modelMat =  modelMat * Angel::Translate(xAxis, yAxis, zAxis);
	modelMat =  modelMat * Angel::RotateY(theta);

	//printf("%f %f %f\n", xAxis, yAxis, zAxis);
	//Angel::Scale(2/(xMax-xMin), 2/(yMax-yMin), 2/(zMax-zMin)) *
	//printf("%f %f %f %f %f %f\n", xMin, xMax, yMin, yMax, zMin, zMax);
	float modelMatrixf[16];
	modelMatrixf[0] = modelMat[0][0];modelMatrixf[4] = modelMat[0][1];
	modelMatrixf[1] = modelMat[1][0];modelMatrixf[5] = modelMat[1][1];
	modelMatrixf[2] = modelMat[2][0];modelMatrixf[6] = modelMat[2][1];
	modelMatrixf[3] = modelMat[3][0];modelMatrixf[7] = modelMat[3][1];

	modelMatrixf[8] = modelMat[0][2];modelMatrixf[12] = modelMat[0][3];
	modelMatrixf[9] = modelMat[1][2];modelMatrixf[13] = modelMat[1][3];
	modelMatrixf[10] = modelMat[2][2];modelMatrixf[14] = modelMat[2][3];
	modelMatrixf[11] = modelMat[3][2];modelMatrixf[15] = modelMat[3][3];
	
	// set up projection matricies
	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
	glUniformMatrix4fv( modelMatrix, 1, GL_FALSE, modelMatrixf );
	GLuint viewMatrix = glGetUniformLocationARB(program, "projection_matrix");
	glUniformMatrix4fv( viewMatrix, 1, GL_FALSE, viewMatrixf);

	draw();
	glFlush(); // force output to graphics hardware
	// use this call to double buffer
	glutSwapBuffers();
	// you can implement your own buffers with textures
}

void idle(void){
	if(xPositiveMove==true)
		xAxis+=xTinnyMove;
	if(xNegativeMove==true)
		xAxis-=xTinnyMove;
	if(yPositiveMove==true)
		yAxis+=yTinnyMove;
	if(yNegativeMove==true)
		yAxis-=yTinnyMove;
	if(zPositiveMove==true)
		zAxis+=zTinnyMove*3.0;
	if(zNegativeMove==true)
		zAxis-=zTinnyMove*3.0;
	if(rotation==true){
		if(alternation==false)
			theta-=0.1;
		else
			theta+=0.1;
		
		if(theta>360.0){
			theta=0.0;
			alternation=false;
			nextPicture();
		}
		if(theta<-360.0){
			theta=0.0;
			alternation=true;
			nextPicture();
		}
	}
	if(pulsing==true){
		if(time>=0 && time<1000){
			for(int i=0; i<NumVertices; i++){
				//points[i]+=faceDirect[i]*0.01*sin(time);
				points[i]+=faceDirect[i]*0.00005*relativeLength;
			}
			time++;
		}
		
		if(time==1000)
			time=2000;

		if(time>1000 && time<=2000){
			for(int i=0; i<NumVertices; i++){
				points[i]-=faceDirect[i]*0.00005*relativeLength;
			}
			time--;
		}

		if(time==1000)
			time=0;
	}
		
	glutPostRedisplay();
}

void initalizePosition( void ){
	xAxis=0.0f;
	yAxis=0.0f;
	zAxis=0.0f;
	rotation=false;
	pulsing=false;
	calmDown();
}

void calmDown(void){
	xPositiveMove=false;
	yPositiveMove=false;
	zPositiveMove=false;
	xNegativeMove=false;
	yNegativeMove=false;
	zNegativeMove=false;
}

void nextPicture(void){
	if(index==fileNum-1)
		index=0;
	else
		index++;
	readData();
}

void updateBoundingBox(void){
	points4[0] = point4(xMin,  yMin,  zMin, 1.0);
	points4[1] = point4(xMax,  yMin,  zMin, 1.0);
	points4[2] = point4(xMin,  yMin,  zMin, 1.0);
	points4[3] = point4(xMin,  yMax,  zMin, 1.0);
	points4[4] = point4(xMin,  yMin,  zMin, 1.0);
	points4[5] = point4(xMin,  yMin,  zMax, 1.0);
	points4[6] = point4(xMax,  yMin,  zMin, 1.0);
	points4[7] = point4(xMax,  yMax,  zMin, 1.0);
	points4[8] = point4(xMax,  yMin,  zMin, 1.0);
	points4[9] = point4(xMax,  yMin,  zMax, 1.0);
	points4[10] = point4(xMax,  yMin,  zMax, 1.0);
	points4[11] = point4(xMin,  yMin,  zMax, 1.0);
	points4[12] = point4(xMax,  yMin,  zMax, 1.0);
	points4[13] = point4(xMax,  yMax,  zMax, 1.0);
	points4[14] = point4(xMax,  yMax,  zMax, 1.0);
	points4[15] = point4(xMax,  yMax,  zMin, 1.0);
	points4[16] = point4(xMax,  yMax,  zMin, 1.0);
	points4[17] = point4(xMin,  yMax,  zMin, 1.0);
	points4[18] = point4(xMin,  yMax,  zMax, 1.0);
	points4[19] = point4(xMin,  yMin,  zMax, 1.0);
	points4[20] = point4(xMin,  yMax,  zMin, 1.0);
	points4[21] = point4(xMin,  yMax,  zMax, 1.0);
	points4[22] = point4(xMax,  yMax,  zMax, 1.0);
	points4[23] = point4(xMin,  yMax,  zMax, 1.0);
	modelMatTemp = Angel::Translate(-(xMin+xMax)/2, -(yMin+yMax)/2, -(zMin+zMax)/2);
	for(int i=0; i<24; i++){
		points4[i] = modelMatTemp*points4[i];
		if(points4[i].w != 1.0)
			printf("%f\n", points4[i].w);
	}
}

// keyboard handler
void keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
	case 'e':
		drawingBoundingBox=!drawingBoundingBox;
		break;
	case 'm':
		drawingFaceNormal=drawingFaceNormal==false?true:false;
		break;
	case 'B':
		drawingBoundingBox=false;
		if(pulsing==true){
			pulsing=false;
			for(int i=0; i<NumVertices; i++)
				points[i]=points2[i];
		}else{
			pulsing=true;
			time=0;
		}
		break;
	case 'R':
		if(rotation==false){
			calmDown();
			rotation=true;
		}else
			rotation=false;
		break;
	case 'W':
		initalizePosition();
		break;
	case 'N':
		initalizePosition();
		nextPicture();
		glutPostRedisplay();
		break;
	case 'P':
		initalizePosition();
		if(index==0)
			index=fileNum-1;
		else
			index--;
		readData();
		glutPostRedisplay();
		break;
	case 'X':
		xNegativeMove=false;
		rotation=false;
		xPositiveMove=xPositiveMove==false?true:false;
		break;
	case 'x':
		xPositiveMove=false;
		rotation=false;
		xNegativeMove=xNegativeMove==false?true:false;
		break;
	case 'Y':
		yNegativeMove=false;
		rotation=false;
		yPositiveMove=yPositiveMove==false?true:false;
		break;
	case 'y':
		yPositiveMove=false;
		rotation=false;
		yNegativeMove=yNegativeMove==false?true:false;
		break;
	case 'Z':
		zNegativeMove=false;
		rotation=false;
		zPositiveMove=zPositiveMove==false?true:false;
		break;
	case 'z':
		zPositiveMove=false;
		rotation=false;
		zNegativeMove=zNegativeMove==false?true:false;
		break;
    case 033:
        exit( EXIT_SUCCESS );
        break;
    }
}

//----------------------------------------------------------------------------
// entry point
int main( int argc, char **argv )
{
	// init glut
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
	width = 512;
	height = 512;

	for(int i=0; i<50000; i++)
		colors[i]=vertex_colors[4];
    // If you are using freeglut, the next two lines will check if 
    // the code is truly 3.2. Otherwise, comment them out
    
	// should run a test here 
	// with different cases
	// this is a good time to find out information about
	// your graphics hardware before you allocate any memory
    glutInitContextVersion( 3, 1 );
    glutInitContextProfile( GLUT_CORE_PROFILE );

	// create window
	// opengl can be incorperated into other packages like wxwidgets, fltoolkit, etc.
    glutCreateWindow( "Color Cube" );

	// init glew
    glewInit();

	readData();
    generateGeometry();

	// assign handlers
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
	glutIdleFunc( idle );

	// should add menus
	// add mouse handler
	// add resize window functionality (should probably try to preserve aspect ratio)

	// enter the drawing loop
	// frame rate can be controlled with 
    glutMainLoop();
    return 0;
}
