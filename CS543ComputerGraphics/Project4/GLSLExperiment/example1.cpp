// Two-Dimensional Sierpinski Gasket       
// Generated using randomly selected vertices and bisection

#ifdef USER_INPUT
// Include the C++ iostreams classes for use when collecting user input
#include <iostream>
#endif 

#include "Angel.h"
#include "bmpread.h"
#include <time.h>
#include <stack>
#include <complex>
typedef std::complex<float>  Complex;

using namespace std;

//----------------------------------------------------------------------------
int width = 0;
int height = 0;

bmpread_t bitmap;

// remember to prototype
void generateGeometry( void );
void display( void );
void keyboard( unsigned char key, int x, int y );
void readLSYSFile( int index);
void processLsystem(int index);
void readPlyFile( char* fileName);
void drawCylinder(float x, float y, float z, float angleX, float angleY, float angleZ);
void drawSphere(float x, float y, float z);
void scaleC(void);
void scaleS(void);
void drawTree(int index);
void drawGround(void);
void drawForest(void);
void drawCar(void);
void drawLamp(void);
void drawBeethoven(void);
int generateRandom(int left, int right);
void loadEnvPic(void);
void quad(int a, int b, int c, int d);
void cube(void);
void drawCube(void);

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

typedef struct
{
        int len;
		int iter;
		float rotX;
		float rotY;
		float rotZ;
		char start[15000];
		char f[30];
		char x[15];
       
}LSYS;

LSYS lsys[5];
char* lsysNames[]={"lsys1.txt", "lsys2.txt", "lsys3.txt", "lsys4.txt", "lsys5.txt"};
float xMin, xMax, yMin, yMax, zMin, zMax;
float relativeLength;
int numberOfSphereVertices;
int numberOfSpherePolygons;
int numberOfCylinderVertices;
int numberOfCylinderPolygons;
int numberOfVertices;
int numberOfPolygons;
point4 sphereVertices[200];
point4 spherePoints[1000];
point4 sphereColors[1000];
point4 cylinderVertices[50];
point4 cylinderPoints[200];
point4 cylinderColors[200];
point4 vertices[7000];
point4 vNormals[7000];
point4 normals[41000];
int address[41000];
point4 points[41000];
point4 colors[41000];
point4 cubePoints[36];
point4 cubeColors[36];
point4 cubeNormals[36];

float theta=0.0f;
point4 newPoint=point4(0.0,  0.0,  0.0, 1.0);
point4 eye = point4(0.0, 10.0, 35.0, 1.0);
point4 light = point4(-30.0, 30, 50, 1.0);
point4 oldPoint=newPoint;
float viewMatrixf[16];
float modelMatrixf[16];
char systemState='A';
bool shadowOn = false;
bool grassOn = false;
bool fogOn = false;
bool reflectionOn = false;
bool refractionOn = false;
bool randomOn = false;
mat4 viewMat = LookAt( eye, vec4(0,0,0,1), vec4(0,1,0,0));
vec4 tmp1, tmp2, tmp3;

static  GLuint  texture = 0;
float fog = 3.0;
int treePosition[2];
bool drawNewTree = true;

// handle to program
GLuint program;
GLuint vTexCoord;
GLuint vao, vao1;
GLuint buffer, buffer1;
GLuint tex;
GLuint texMapLocation;

// RGBA olors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
	color4( 0.0, 1.0, 1.0, 1.0 ),   // cyan
    color4( 1.0, 1.0, 1.0, 1.0 )  // white
};

point4 groundVertices1[6]={
	point4(-200, -0.01, -200,1),
	point4(-200, -0.01, 200,1),
	point4( 200, -0.01, 200,1),
	point4( 200, -0.01, 200,1),
	point4( 200, -0.01,-200,1),
	point4(-200, -0.01,-200,1)
};

const float tile = 16.0f;
vec2 groundTexture[6]={
	vec2(0.0, 0.0),
	vec2(0.0, tile),
	vec2(tile, tile),
	vec2(tile, tile),
	vec2(tile, 0.0),
	vec2(0.0, 0.0)
};


void generateGeometry( void )
{	
    // Create a vertex array object
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    program = InitShader( "vshader1.glsl", "fshader1.glsl" );
    glUseProgram( program );

    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer);

	glGenVertexArrays( 1, &vao1 );
    glBindVertexArray( vao1 );
	glGenBuffers( 1, &buffer1 );
    glBindBuffer( GL_ARRAY_BUFFER, buffer1);

	// sets the default color to clear screen
    glClearColor( 1.0, 1.0, 1.0, 1.0 ); // white background

	glUniform1f( glGetUniformLocation(program, "fog"), fog);
	//glUniform1f( glGetUniformLocation(program, "reflect"), reflect);
}

//----------------------------------------------------------------------------
// this is where the drawing should happen
void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );     // clear the window

	Angel::mat4 perspectiveMat = Angel::Perspective((GLfloat)75.0, (GLfloat)width/(GLfloat)height, (GLfloat)0.01, (GLfloat) 999.0);

	GLuint viewMatrix = glGetUniformLocationARB(program, "projection_matrix");
    glUniformMatrix4fv( viewMatrix, 1, GL_TRUE, perspectiveMat);
			
	drawForest();

	glFlush(); // force output to graphics hardware
    // use this call to double buffer
    glutSwapBuffers();
}

void drawForest(){
	int randNumb;

	GLuint vTextOn = glGetUniformLocation( program, "textOn");
	glUniform1i(vTextOn, false);

	/*if(drawNewTree){
		treePosition[0]=rand()%5;
		while(treePosition[0]==(randNumb=rand()%5))
			;
		treePosition[1]=randNumb;
	}*/
	

	drawTree(1);
	drawTree(3);
	drawCar();
	drawLamp();
	drawBeethoven();
	drawCube();

	glUniform1i(vTextOn, true);

	drawGround();
}

// keyboard handler
void keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
    case 033:
        exit( EXIT_SUCCESS );
        break;
	case 'A':
		systemState='A';
		grassOn = !grassOn;
		glutPostRedisplay();
		break;
	case 'D':
		{systemState='D';
		shadowOn = !shadowOn;
		glutPostRedisplay();
		break;}
	case 'F':
		systemState='F';
		fogOn = !fogOn;  // no use
		if(fog>2.0)
			fog = 1.0;
		else
			fog = fog > 0.5 ? 0.0 : 1.0;
		glUniform1f( glGetUniformLocation(program, "fog"), fog);
		glutPostRedisplay();
		break;
	case 'T':
		systemState='T';
		reflectionOn = !reflectionOn;
		refractionOn = false;
		glutPostRedisplay();
		break;
	case 'V':
		systemState = 'V';
		refractionOn = !refractionOn;
		reflectionOn = false;
		glutPostRedisplay();
		break;
	case 'K':
		systemState = 'K';
		randomOn = !randomOn;
		reflectionOn = false;
		refractionOn = false;
		glutPostRedisplay();
		break;
    }

	if(systemState!='F'){
		fog = 3.0;
		glUniform1f( glGetUniformLocation(program, "fog"), fog);
	}
}

//----------------------------------------------------------------------------
// entry point
int main( int argc, char **argv )
{
	// init glut
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 1024, 1024 );
	width = 1024;
	height = 1024;
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
    glutCreateWindow( "Project 4 by Jiefeng He" );

	// init glew
    glewInit();

	srand (time(NULL));

	for(int i=0; i<5;i++){
		readLSYSFile(i);
		processLsystem(i);
	}

	loadEnvPic();
	readPlyFile("sphere.ply");
	readPlyFile("cylinder.ply");

	int randColor=rand()%6;

    point4 color=(vertex_colors[4]+vertex_colors[1])/2.0;
	for(int i=0; i<41000; i++){
		colors[i] = color;
	}

	for(int i=0; i<1000; i++){
		sphereColors[i]=vertex_colors[randColor];
	}

	for(int i=0; i<200; i++){
		cylinderColors[i]=vertex_colors[randColor];
	}

    color=(vertex_colors[3]+vertex_colors[5])/2.0;
	for(int i=0; i<36; i++){
			cubeColors[i] = color;
	}

	cube();

    generateGeometry();

	// assign handlers
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
	// should add menus
	// add mouse handler
	// add resize window functionality (should probably try to preserve aspect ratio)

	// enter the drawing loop
	// frame rate can be controlled with 
    glutMainLoop();
    return 0;
}

void readLSYSFile( int index){
	FILE * fp;
	char *pch;

	fp=fopen(lsysNames[index], "r");
	
	if(fp==NULL)
		exit(EXIT_FAILURE);
	char str[150];
	fgets(str, sizeof(str), fp);
	while(str[0]=='#'){
		fgets(str, sizeof(str), fp);
	}

	//len
	pch = strtok(str, " :");
	pch = strtok(NULL, " :");
	lsys[index].len = atoi(pch);

	//iteration
	fgets(str, sizeof(str), fp);
	pch = strtok(str, " :");
	pch = strtok(NULL, " :");
	lsys[index].iter = atoi(pch);

	//rotation
	fgets(str, sizeof(str), fp);
	pch = strtok(str, " :");
	pch = strtok(NULL, " :");
	lsys[index].rotX = atof(pch);
	pch = strtok(NULL, " :");
	lsys[index].rotY = atof(pch);
	pch = strtok(NULL, " :");
	lsys[index].rotZ = atof(pch);

	//start
	fgets(str, sizeof(str), fp);
	pch = strtok(str, " :");
	pch = strtok(NULL, " :");
	strcpy (lsys[index].start, pch);

	if(pch[0]=='F'){
		//just F
		fgets(str, sizeof(str), fp);
		pch = strtok(str, " :");
		pch = strtok(NULL, " :");
		strcpy (lsys[index].f, pch);
	}else{
		//first X
		fgets(str, sizeof(str), fp);
		pch = strtok(str, " :");
		pch = strtok(NULL, " :");
		strcpy (lsys[index].x, pch);

		//second F
		fgets(str, sizeof(str), fp);
		pch = strtok(str, " :");
		pch = strtok(NULL, " :");
		strcpy (lsys[index].f, pch);
	}

	fclose(fp);
}

void processLsystem(int index){
	char buf[15000];
	char *bufPointer, *startPointer,*fPointer, *xPointer;
	while(lsys[index].iter>0){
		lsys[index].iter--;
		bufPointer=buf;
		startPointer=lsys[index].start;
		while(*startPointer!='\0'&&*startPointer!='\n'){
			if(*startPointer=='X'){
				xPointer=lsys[index].x;
				while(*xPointer!='\0'&&*xPointer!='\n')
					*bufPointer++=*xPointer++;
			}
			else if(*startPointer=='F'){
				fPointer=lsys[index].f;
				while(*fPointer!='\0'&&*fPointer!='\n')
					*bufPointer++=*fPointer++;
			}else{
				*bufPointer++=*startPointer;
			}
			startPointer++;
		}
		*bufPointer='\0';
		strcpy(lsys[index].start, buf);
		//printf("%d\n", strlen(lsys[index].start));
	}
}

void drawSphere(float x, float y, float z) {
    Angel::mat4 modelMat = Angel::Translate(x, y, z);
	modelMat = Angel::RotateY(90) * modelMat;

	if(systemState!='D'||systemState!='F'){
		GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
		glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, viewMat * modelMat );

		int randColor=rand()%6;

		for(int i=0; i<1000; i++){
			sphereColors[i] = (sphereColors[i] + vertex_colors[randColor])/2;
		}

		glBindBuffer( GL_ARRAY_BUFFER, buffer);
		glBufferData( GL_ARRAY_BUFFER, sizeof(spherePoints) + sizeof(sphereColors), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(spherePoints), spherePoints );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(spherePoints), sizeof(sphereColors), sphereColors );
 
		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );
 
		GLuint vColor;
		vColor = glGetAttribLocation( program, "vColor" ); 
		glEnableVertexAttribArray( vColor );
		glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(spherePoints)) );
         
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glEnable( GL_DEPTH_TEST );
		glDrawArrays( GL_TRIANGLES, 0,numberOfSpherePolygons*3 );
		glDisable( GL_DEPTH_TEST ); 
	}

	if( shadowOn ){
		Angel::mat4 m = Angel::identity();
		m[3][1] = -1.0/(light.y);
		m[3][3] = 0.0;

		GLuint vShadowOn = glGetUniformLocation( program, "shadowOn");
		glUniform1i(vShadowOn, true);

		m = viewMat * Angel::Translate(light.x, light.y, light.z) * m * Angel::Translate(-light.x, -light.y, -light.z);
		m = m * modelMat;

		GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
		glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, m );

		glBindBuffer( GL_ARRAY_BUFFER, buffer);
		glBufferData( GL_ARRAY_BUFFER, sizeof(spherePoints) + sizeof(sphereColors), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(spherePoints), spherePoints );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(spherePoints), sizeof(sphereColors), sphereColors );
 
		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );
 
		GLuint vColor;
		vColor = glGetAttribLocation( program, "vColor" ); 
		glEnableVertexAttribArray( vColor );
		glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(spherePoints)) );
         
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glEnable( GL_DEPTH_TEST );
		glDrawArrays( GL_TRIANGLES, 0,numberOfSpherePolygons*3 );
		glDisable( GL_DEPTH_TEST ); 

		glUniform1i(vShadowOn, false);
	}
}

void drawCylinder(float x, float y, float z, float angleX, float angleY, float angleZ){
    Angel::mat4 modelMat = Angel::identity();
	modelMat =  modelMat * Angel::Translate(x, y, z);
	modelMat =  modelMat * Angel::RotateX(angleX);
	modelMat = modelMat * Angel::RotateZ(angleZ);
	modelMat = modelMat * Angel::RotateY(angleY);
	modelMat = Angel::RotateY(90) * modelMat;

	if(systemState!='D'||systemState!='F'){
		GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
		glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, viewMat * modelMat );

		int randColor=rand()%6;

		for(int i=0; i<200; i++){
			cylinderColors[i] = (cylinderColors[i] + vertex_colors[randColor])/2;
		}
		glBindBuffer( GL_ARRAY_BUFFER, buffer);
		glBufferData( GL_ARRAY_BUFFER, sizeof(cylinderPoints) + sizeof(cylinderColors), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(cylinderPoints), cylinderPoints );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(cylinderPoints), sizeof(cylinderColors), cylinderColors );
 
		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );
 
		GLuint vColor;
		vColor = glGetAttribLocation( program, "vColor" ); 
		glEnableVertexAttribArray( vColor );
		glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(cylinderPoints)) );
         
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glEnable( GL_DEPTH_TEST );
		glDrawArrays( GL_TRIANGLES, 0,numberOfCylinderPolygons*3 );
		glDisable( GL_DEPTH_TEST ); 
	}

	if( shadowOn ){
		Angel::mat4 m = Angel::identity();
		m[3][1] = -1.0/(light.y);
		m[3][3] = 0.0;

		GLuint vShadowOn = glGetUniformLocation( program, "shadowOn");
		glUniform1i(vShadowOn, true);

		m = viewMat * Angel::Translate(light.x, light.y, light.z) * m * Angel::Translate(-light.x, -light.y, -light.z);
		m = m * modelMat;

		GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
		glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, m );

		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData( GL_ARRAY_BUFFER, sizeof(cylinderPoints) + sizeof(cylinderColors), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(cylinderPoints), cylinderPoints );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(cylinderPoints), sizeof(cylinderColors), cylinderColors );
 
		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );
 
		GLuint vColor;
		vColor = glGetAttribLocation( program, "vColor" ); 
		glEnableVertexAttribArray( vColor );
		glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(cylinderPoints)) );
         
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glEnable( GL_DEPTH_TEST );
		glDrawArrays( GL_TRIANGLES, 0,numberOfCylinderPolygons*3 );
		glDisable( GL_DEPTH_TEST );

		glUniform1i(vShadowOn, false);
	}
}

void readPlyFile( char* fileName){
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
    if(fp == NULL)
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
	
	if(strcmp(fileName, "sphere.ply")==0){
		numberOfSphereVertices=numberOfVertices;
		numberOfSpherePolygons=numberOfPolygons;
	}
	else if(strcmp(fileName, "cylinder.ply")==0){
		numberOfCylinderVertices=numberOfVertices;
		numberOfCylinderPolygons=numberOfPolygons;
	}

    for(int i=0; i<numberOfVertices; i++){
        fscanf(fp, "%f %f %f", &x, &y, &z);
		if(strcmp(fileName, "sphere.ply")==0)
			sphereVertices[i] = point4(x,  y,  z, 1.0);
		else if(strcmp(fileName, "cylinder.ply")==0)
			cylinderVertices[i] = point4(x,  y,  z, 1.0);
		else
			vertices[i] = point4(x,  y,  z, 1.0);

        if(xMin>x)        xMin=x;
        if(yMin>y)        yMin=y;
        if(zMin>z)        zMin=z;
        if(xMax<x)        xMax=x;
        if(yMax<y)        yMax=y;
        if(zMax<z)        zMax=z;
    }
    relativeLength = sqrt(pow(xMax-xMin, 2)+pow(yMax-yMin, 2)+pow(zMax-zMin, 2));

	if(strcmp(fileName, "sphere.ply")==0){
		scaleS();
	}
	else if(strcmp(fileName, "cylinder.ply")==0){
		scaleC();
	}else{
		//update the position to lowest level
		Angel::mat4 modelMat = Angel::Translate(-(xMin+xMax)/2, -yMin, -(zMin+zMax)/2);
		for(int i=0; i<numberOfVertices; i++){
			vertices[i] = modelMat*vertices[i];
		}
	}

	int index=0;
    int triangle;
    int one,two,three;
	for(int i=0; i<7000; i++)
		vNormals[i]=vec4(0.0, 0.0, 0.0, 0.0);
	
    for(int i=0; i<numberOfPolygons; i++){
        fscanf(fp, "%d %d %d %d", &triangle, &one, &two, &three);
        if(triangle!=3)
                exit(EXIT_FAILURE);

		if(strcmp(fileName, "sphere.ply")==0){
			spherePoints[index] = sphereVertices[one]; 
			index++; 
			spherePoints[index] = sphereVertices[two];
			index++;
			spherePoints[index] = sphereVertices[three]; 
			index++;
		}
		else if(strcmp(fileName, "cylinder.ply")==0){
			cylinderPoints[index] = cylinderVertices[one]; 
			index++; 
			cylinderPoints[index] = cylinderVertices[two];
			index++;
			cylinderPoints[index] = cylinderVertices[three]; 
			index++;
		}else{
			points[index] = vertices[one]; 
			index++; 
			points[index] = vertices[two]; 
			index++;
			points[index] = vertices[three];  
			index++;

			tmp1 = vertices[two] - vertices[one];
			tmp2 = vertices[three] - vertices[one];
			normals[index-3] = normalize(cross(tmp1, tmp2));
			normals[index-3].w = 0.0;
			normals[index-2] = normals[index-3];
			normals[index-1] = normals[index-2];
			/*tmp1 = vertices[two] - vertices[one];
			tmp2 = vertices[three] - vertices[one];
			tmp3 = normalize(cross(tmp1, tmp2));
			vNormals[one] += tmp3;
			vNormals[two] += tmp3;
			vNormals[three] += tmp3;
			address[index-3] = one;
			address[index-2] = two;
			address[index-1] = three;*/
		}   
	}

	/*for(int i=0; i<numberOfVertices; i++)
		vNormals[i] = normalize(vNormals[i]);

	for(int i=0; i<numberOfPolygons*3; i++)
		normals[i]= vNormals[address[i]];*/

	////normalize at last in order to consider their area
	//for(int i=0; i<numberOfVertices; i++){
	//	normals[i] = normalize(normals[i]);
	//}

	printf("%f %f %f %f %f %f\n", xMin, xMax, yMin, yMax, zMin, zMax);
	fclose(fp);
}

void scaleC(void){
	Angel::mat4 modelMat = Angel::RotateX(90.0f);
	modelMat = modelMat * Angel::Scale(1/1.41421/8, 1/1.41421/8, 1/2.0);
    for(int i=0; i<numberOfCylinderVertices; i++){
		cylinderVertices[i] = modelMat*cylinderVertices[i];
    }
}

void scaleS(void){
	Angel::mat4 modelMat = Angel::Scale(1/8.0, 1/8.0, 1/8.0);
	for(int i=0; i<numberOfSphereVertices; i++){
		sphereVertices[i] = modelMat*sphereVertices[i];
    }
}

int generateRandom(int left, int right){
	return left+rand()%(right-left+1);
}

void drawTree(int index){
	stack<point4> points;
	float angleX=0.0f;
	float angleY=0.0f;
	float angleZ=0.0f;
	stack<float> angleXs;
	stack<float> angleYs;
	stack<float> angleZs;
	point4 direction=point4(0.0, 1.0, 0.0, 0.0);

	if(index==0)
		//newPoint=point4(generateRandom(30, 200), 0.0, generateRandom(-45, 45), 1.0);
		newPoint=point4(40, 0.0, 34, 1.0);
	else if(index==1)
		//newPoint=point4(generateRandom(50, 100), 0.0, generateRandom(-45, 45), 1.0);
		newPoint=point4(58, 0.0, -15, 1.0);
	else if(index==2)
		//newPoint=point4(generateRandom(-10, 30), 0.0, generateRandom(-15, 15), 1.0);
		newPoint=point4(5, 0.0, 11, 1.0);
	else if(index==3)
		//newPoint=point4(generateRandom(-10, 30), 0.0, generateRandom(-15, 15), 1.0);
		newPoint=point4(13, 0.0, -18, 1.0);
	else 
		//newPoint=point4(generateRandom(200, 300), 0.0, generateRandom(-150, 150), 1.0);
		newPoint=point4(350, 0.0, 7, 1.0);

	drawSphere(newPoint.x, newPoint.y, newPoint.z);
	char *startPointer=lsys[index].start;
	
	while(*startPointer){
		switch(*startPointer){
			case '+':
				angleX+=lsys[index].rotX;
				break;
			case '-':
				angleX-=lsys[index].rotX;
				break;
			case '&':
				angleY+=lsys[index].rotY;
				break;
			case '^':
				angleY-=lsys[index].rotY;
				break;
			case '\\':
				angleZ+=lsys[index].rotZ;
				break;
			case '/':
				angleZ-=lsys[index].rotZ;
				break;
			case '|':
				angleX+=180;
				angleY+=180;
				angleZ+=180;
				break;
			case '[':
				points.push(newPoint);
				angleXs.push(angleX);
				angleYs.push(angleY);
				angleZs.push(angleZ);
				break;
			case ']':
				newPoint=points.top();
				angleX=angleXs.top();
				angleY=angleYs.top();
				angleZ=angleZs.top();
				points.pop();
				angleXs.pop();
				angleYs.pop();
				angleZs.pop();
				break;
			case 'F':
				oldPoint=newPoint;
				direction=Angel::RotateZ(angleZ)*Angel::RotateY(angleY)*Angel::RotateX(angleX)*point4(0.0, lsys[index].len+0.0, 0.0, 0.0);
				newPoint = oldPoint + direction;
				drawSphere(newPoint.x, newPoint.y, newPoint.z);
				drawCylinder((newPoint.x+oldPoint.x)/2, (newPoint.y+oldPoint.y)/2, (newPoint.z+oldPoint.z)/2, angleX, angleY, angleZ);
				break;
			default:
				break;
		}
		startPointer++;
	}
}

void loadEnvPic(void){

	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

	if(!bmpread("nvposx.bmp", 0, &bitmap))
		{
			fprintf(stderr, "%s:error loading bitmap file\n", "nvposx.bmp");
			exit(1);
		}
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X , 0, GL_RGB, bitmap.width, bitmap.height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap.rgb_data );
	bmpread_free(&bitmap);
	if(!bmpread("nvnegx.bmp", 0, &bitmap))
		{
			fprintf(stderr, "%s:error loading bitmap file\n", "nvnegx.bmp");
			exit(1);
		}
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X , 0, GL_RGB, bitmap.width, bitmap.height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap.rgb_data );
	bmpread_free(&bitmap);
	if(!bmpread("nvposy.bmp", 0, &bitmap))
		{
			fprintf(stderr, "%s:error loading bitmap file\n", "nvposy.bmp");
			exit(1);
		}
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y , 0, GL_RGB, bitmap.width, bitmap.height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap.rgb_data );
	bmpread_free(&bitmap);
	if(!bmpread("nvnegy.bmp", 0, &bitmap))
		{
			fprintf(stderr, "%s:error loading bitmap file\n", "nvnegy.bmp");
			exit(1);
		}
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y , 0, GL_RGB, bitmap.width, bitmap.height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap.rgb_data );
	bmpread_free(&bitmap);
	if(!bmpread("nvposz.bmp", 0, &bitmap))
		{
			fprintf(stderr, "%s:error loading bitmap file\n", "nvposz.bmp");
			exit(1);
		}
	glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z , 0, GL_RGB, bitmap.width, bitmap.height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap.rgb_data );
	bmpread_free(&bitmap);
	if(!bmpread("nvnegz.bmp", 0, &bitmap))
		{
			fprintf(stderr, "%s:error loading bitmap file\n", "nvnegz.bmp");
			exit(1);
		}
	glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z , 0, GL_RGB, bitmap.width, bitmap.height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap.rgb_data );
	bmpread_free(&bitmap);

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);    
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);    
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	texMapLocation = glGetUniformLocation(program, "texMap");
	glUniform1i(texMapLocation, 1);
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	/*glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points)+sizeof(normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals);

	GLuint loc2 = glGetAttribLocation(program, "Normal");
	glEnableVertexAttribArray(loc2);*/
}

void drawGround(void){

	if(grassOn){
		if(!bmpread("grass.bmp", 0, &bitmap))
		{
			fprintf(stderr, "%s:error loading bitmap file\n", "grass.bmp");
			exit(1);
		}
	}else{
		if(!bmpread("stones.bmp", 0, &bitmap))
		{
			fprintf(stderr, "%s:error loading bitmap file\n", "stones.bmp");
			exit(1);
		}
	}

		glActiveTexture( GL_TEXTURE0 );
		glGenTextures( 1, &texture );
		glBindTexture( GL_TEXTURE_2D, texture );

		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, bitmap.width, bitmap.height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap.rgb_data );
		bmpread_free(&bitmap);

		Angel::mat4 modelMat = Angel::Translate(0.0, 0.0, 0.0);
		modelMat *= Angel::RotateY(-45.0f);
		modelMat = Angel::RotateY(90) * modelMat;

		glBindBuffer( GL_ARRAY_BUFFER, buffer1);
		glBufferData( GL_ARRAY_BUFFER,sizeof(groundVertices1) + sizeof(groundTexture), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(groundVertices1), groundVertices1 );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(groundVertices1), sizeof(groundTexture), groundTexture );

		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );

		GLuint vTexCoord = glGetAttribLocation( program, "vTexCoord" ); 
		glEnableVertexAttribArray( vTexCoord );
		glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(sizeof(groundVertices1))  );

		glUniform1i( glGetUniformLocation(program, "texture"), 0 );
		glBindTexture(GL_TEXTURE_2D, texture);

		glClearColor(1.0, 1.0, 1.0, 1.0);

		GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
	    glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, viewMat * modelMat );

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable( GL_DEPTH_TEST );
		glDrawArrays( GL_TRIANGLES, 0, 6  );
	
		glDisable( GL_DEPTH_TEST ); 
	//}else{
	//	int index;
	//	const int scale = 101;
	//	point4 groundVertices[scale][scale];
	//	for(int i=-200; i<=200; i+=4)
	//		for(int j=-200; j<=200; j+=4){
	//			//use the two dimensional array as an matrix ground
	//			groundVertices[i/4+scale/2][j/4+scale/2]=point4(i, 0, j, 1.0);
	//		}
	//
	//	index=0;
	//	point4 color=vertex_colors[6];
	//	for(int p=0; p<scale; p++)
	//		for(int q=0; q<scale-1; q++){
	//			groundPoints[index]=groundVertices[p][q];
	//			groundColors[index++]=color;
	//			groundPoints[index]=groundVertices[p][q+1];
	//			groundColors[index++]=color;
	//		}
	//	for(int q=0; q<scale; q++)
	//		for(int p=0; p<scale-1; p++){
	//			groundPoints[index]=groundVertices[p][q];
	//			groundColors[index++]=color;
	//			groundPoints[index]=groundVertices[p+1][q];
	//			groundColors[index++]=color;
	//		}
	//	//printf("Total point number for ground: %d\n", index);

	//	Angel::mat4 modelMat = Angel::Translate(0.0, 0.0, 0.0);
	//	modelMat *= Angel::RotateY(-45.0f);
	//	modelMat = Angel::RotateY(90) * modelMat;

	//	drawMode = 0;
	//	glUniform1i( glGetUniformLocation(program, "DrawMode"), drawMode);

	//	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
	//	glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, viewMat * modelMat );

	//	glBufferData( GL_ARRAY_BUFFER, sizeof(groundPoints) + sizeof(groundColors), NULL, GL_STATIC_DRAW );
	//	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(groundPoints), groundPoints );
	//	glBufferSubData( GL_ARRAY_BUFFER, sizeof(groundPoints), sizeof(groundColors), groundColors );
 //
	//	glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(groundPoints)) );
 //               
	//	glDrawArrays( GL_LINES, 0, index);
	//}

}

void drawCar(void){
	Angel::mat4 modelMat = Angel::Translate(16.0, 5, 0.0);
	modelMat *= Angel::Scale(3, 3, 3);
	modelMat *= Angel::RotateX(-90.0f);
	modelMat *= Angel::RotateZ(-90.0f);
	modelMat = Angel::RotateY(90) * modelMat;
	readPlyFile("pickup_big.ply");

	if( shadowOn ){
		Angel::mat4 m = Angel::identity();
		m[3][1] = -1.0/(light.y);
		m[3][3] = 0.0;

		GLuint vShadowOn = glGetUniformLocation( program, "shadowOn");
		glUniform1i(vShadowOn, true);

		m = viewMat * Angel::Translate(light.x, light.y, light.z) * m * Angel::Translate(-light.x, -light.y, -light.z);
		m = m * modelMat;

		GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
		glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, m );

		glBindBuffer( GL_ARRAY_BUFFER, buffer);
		glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
 
		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );
 
		GLuint vColor;
		vColor = glGetAttribLocation( program, "vColor" ); 
		glEnableVertexAttribArray( vColor );
		glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
         
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glEnable( GL_DEPTH_TEST );
		glDrawArrays( GL_TRIANGLES, 0, numberOfPolygons*3 );
		glDisable( GL_DEPTH_TEST ); 

		glUniform1i(vShadowOn, false);
	}

	if(reflectionOn || refractionOn || randomOn){
		GLuint vReflect = glGetUniformLocation( program, "reflection");
		GLuint vRefract = glGetUniformLocation( program, "refraction");
		int pick = rand()%2;
        if(reflectionOn)
			pick = 0;
		else if(refractionOn)
			pick = 1;
		if(pick == 0)
			glUniform1i(vReflect, true);
		else
			glUniform1i(vRefract, true);

		GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
		glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, viewMat * modelMat );

		glBindBuffer( GL_ARRAY_BUFFER, buffer);
		glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals );
 
		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );
 
		GLuint vNormal = glGetAttribLocation(program, "Normal");
		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );

		texMapLocation = glGetUniformLocation(program, "texMap");
		glUniform1i(texMapLocation, 1); // corresponding to unit 1
         
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glEnable( GL_DEPTH_TEST );
		glDrawArrays( GL_TRIANGLES, 0, numberOfPolygons*3 );
		glDisable( GL_DEPTH_TEST ); 

		glUniform1i(vReflect, false);
		glUniform1i(vRefract, false);
	}else{
		GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
		glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, viewMat * modelMat );

		glBindBuffer( GL_ARRAY_BUFFER, buffer);
		glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
 
		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );
 
		GLuint vColor;
		vColor = glGetAttribLocation( program, "vColor" ); 
		glEnableVertexAttribArray( vColor );
		glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
         
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glEnable( GL_DEPTH_TEST );
		glDrawArrays( GL_TRIANGLES, 0, numberOfPolygons*3 );
		glDisable( GL_DEPTH_TEST ); 
	}
}

void drawLamp(){
	GLuint vColor;

	Angel::mat4 modelMat = Angel::Translate(-15.0, 0, 12.0);
	modelMat *= Angel::Scale(1.5, 1.5, 1.5);
	modelMat = Angel::RotateY(90) * modelMat;
	
	readPlyFile("street_lamp.ply");

	if( shadowOn ){
		GLuint vShadowOn = glGetUniformLocation( program, "shadowOn");
		glUniform1i(vShadowOn, true);

		Angel::mat4 m = Angel::identity();
		m[3][1] = -1.0/(light.y);
		m[3][3] = 0.0;

		m = viewMat * Angel::Translate(light.x, light.y, light.z) * m * Angel::Translate(-light.x, -light.y, -light.z);
		m = m * modelMat;

		GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
		glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, m );

		glBindBuffer( GL_ARRAY_BUFFER, buffer);
		glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );

		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );
 
		vColor = glGetAttribLocation( program, "vColor" ); 
		glEnableVertexAttribArray( vColor );
		glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
         
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glEnable( GL_DEPTH_TEST );
		glDrawArrays( GL_TRIANGLES, 0, numberOfPolygons*3 );
		glDisable( GL_DEPTH_TEST ); 

		glUniform1i(vShadowOn, false);
	}

	if(reflectionOn || refractionOn || randomOn){
		GLuint vReflect = glGetUniformLocation( program, "reflection");
		GLuint vRefract = glGetUniformLocation( program, "refraction");
		int pick = rand()%2;
        if(reflectionOn)
			pick = 0;
		else if(refractionOn)
			pick = 1;
		if(pick == 0)
			glUniform1i(vReflect, true);
		else
			glUniform1i(vRefract, true);

		GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
		glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, viewMat * modelMat );

		glBindBuffer( GL_ARRAY_BUFFER, buffer);
		glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals );
 
		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );
 
		GLuint vNormal = glGetAttribLocation(program, "Normal");
		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );

		texMapLocation = glGetUniformLocation(program, "texMap");
		glUniform1i(texMapLocation, 1); // corresponding to unit 1
         
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glEnable( GL_DEPTH_TEST );
		glDrawArrays( GL_TRIANGLES, 0, numberOfPolygons*3 );
		glDisable( GL_DEPTH_TEST ); 

		glUniform1i(vReflect, false);
		glUniform1i(vRefract, false);
	}else{
		GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
		glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, viewMat * modelMat );

		glBindBuffer( GL_ARRAY_BUFFER, buffer);
		glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );

		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );
 
		vColor = glGetAttribLocation( program, "vColor" ); 
		glEnableVertexAttribArray( vColor );
		glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
         
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glEnable( GL_DEPTH_TEST );
		glDrawArrays( GL_TRIANGLES, 0, numberOfPolygons*3 );
		glDisable( GL_DEPTH_TEST ); 
	}
}

void drawBeethoven(){
	Angel::mat4 modelMat = Angel::Translate(-12.0, 0, 8.0);
	modelMat *= Angel::Scale(0.8, 0.8, 0.8);
	modelMat *= Angel::RotateY(-110.0f);
	modelMat = Angel::RotateY(90) * modelMat;
	readPlyFile("beethoven.ply");

	if( shadowOn ){
		Angel::mat4 m = Angel::identity();
		m[3][1] = -1.0/(light.y);
		m[3][3] = 0.0;

		GLuint vShadowOn = glGetUniformLocation( program, "shadowOn");
		glUniform1i(vShadowOn, true);

		m = viewMat * Angel::Translate(light.x, light.y, light.z) * m * Angel::Translate(-light.x, -light.y, -light.z);
		m = m * modelMat;

		GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
		glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, m );

		glBindBuffer( GL_ARRAY_BUFFER, buffer);
		glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
 
		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );
 
		GLuint vColor;
		vColor = glGetAttribLocation( program, "vColor" ); 
		glEnableVertexAttribArray( vColor );
		glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
         
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glEnable( GL_DEPTH_TEST );
		glDrawArrays( GL_TRIANGLES, 0, numberOfPolygons*3 );
		glDisable( GL_DEPTH_TEST ); 

		glUniform1i(vShadowOn, false);
	}

	if(reflectionOn || refractionOn || randomOn){
		GLuint vReflect = glGetUniformLocation( program, "reflection");
		GLuint vRefract = glGetUniformLocation( program, "refraction");
		int pick = rand()%2;
        if(reflectionOn)
			pick = 0;
		else if(refractionOn)
			pick = 1;
		if(pick == 0)
			glUniform1i(vReflect, true);
		else
			glUniform1i(vRefract, true);

		GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
		glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, viewMat * modelMat );

		glBindBuffer( GL_ARRAY_BUFFER, buffer);
		glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals );
 
		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );
 
		GLuint vNormal = glGetAttribLocation(program, "Normal");
		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );

		texMapLocation = glGetUniformLocation(program, "texMap");
		glUniform1i(texMapLocation, 1); // corresponding to unit 1
         
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glEnable( GL_DEPTH_TEST );
		glDrawArrays( GL_TRIANGLES, 0, numberOfPolygons*3 );
		glDisable( GL_DEPTH_TEST ); 

		glUniform1i(vReflect, false);
		glUniform1i(vRefract, false);
	}else{
		GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
		glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, viewMat * modelMat );

		glBindBuffer( GL_ARRAY_BUFFER, buffer);
		glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
 
		GLuint vPosition = glGetAttribLocation( program, "vPosition" );
		glEnableVertexAttribArray( vPosition );
		glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );
 
		GLuint vColor;
		vColor = glGetAttribLocation( program, "vColor" ); 
		glEnableVertexAttribArray( vColor );
		glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
         
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glEnable( GL_DEPTH_TEST );
		glDrawArrays( GL_TRIANGLES, 0, numberOfPolygons*3 );
		glDisable( GL_DEPTH_TEST ); 
	}
}

void drawCube(void){
	GLuint vReflect = glGetUniformLocation( program, "reflection");
	if(reflectionOn)
		glUniform1i(vReflect, true);

	Angel::mat4 modelMat = Angel::Translate(10.0,10, 12.0);
	modelMat *= Angel::Scale(0.1, 0.1, 0.1);
	modelMat = Angel::RotateY(90) * modelMat;
	

	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
	glUniformMatrix4fv( modelMatrix, 1, GL_TRUE, viewMat * modelMat );

	glBindBuffer( GL_ARRAY_BUFFER, buffer);
	glBufferData( GL_ARRAY_BUFFER, sizeof(cubePoints) + sizeof(cubeNormals), NULL, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(cubePoints), cubePoints );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(cubePoints), sizeof(cubeNormals), cubeNormals );

	GLuint vPosition = glGetAttribLocation( program, "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				BUFFER_OFFSET(0) );
 
	GLuint vNormal = glGetAttribLocation(program, "Normal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(cubePoints)) );
         
	texMapLocation = glGetUniformLocation(program, "texMap");
	glUniform1i(texMapLocation, 1); // corresponding to unit 1

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glEnable( GL_DEPTH_TEST );
	glDrawArrays( GL_TRIANGLES, 0, 36 );
	glDisable( GL_DEPTH_TEST ); 

	glUniform1i(vReflect, false);  //close reflection on other objects
}

void quad(int a, int b, int c, int d){
	point4 normal;

	point4 vertices[8] = {
		point4( -0.5, -0.5,  0.5, 1.0 ),
		point4( -0.5,  0.5,  0.5, 1.0 ),
		point4(  0.5,  0.5,  0.5, 1.0 ),
		point4(  0.5, -0.5,  0.5, 1.0 ),
		point4( -0.5, -0.5, -0.5, 1.0 ),
		point4( -0.5,  0.5, -0.5, 1.0 ),
		point4(  0.5,  0.5, -0.5, 1.0 ),
		point4(  0.5, -0.5, -0.5, 1.0 )
    };

    static int index =0;
    normal = normalize(cross(vertices[b]-vertices[a], vertices[c]-vertices[b]));
    cubeNormals[index] = normal;
    cubePoints[index] = vertices[a];
    index++;

    cubeNormals[index] = normal;
	cubePoints[index] = vertices[b];
    index++;

    cubeNormals[index] = normal;
    cubePoints[index] = vertices[c];
    index++;

    cubeNormals[index] = normal;
    cubePoints[index] = vertices[a];
    index++;

    cubeNormals[index] = normal;
    cubePoints[index] = vertices[c];
    index++;

    cubeNormals[index] = normal;
    cubePoints[index] = vertices[d];
    index++;
}

void cube( void ){
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}
