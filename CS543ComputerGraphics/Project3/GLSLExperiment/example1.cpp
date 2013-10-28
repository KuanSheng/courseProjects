// Two-Dimensional Sierpinski Gasket       
// Generated using randomly selected vertices and bisection

#include "Angel.h"
#include <time.h>
#include <stack>

//----------------------------------------------------------------------------
int width = 0;
int height = 0;

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
point4 groundVertices[21][21];
point4 groundPoints[2000];
point4 groundColors[2000];
point4 vertices[7000];
point4 points[41000];
point4 colors[41000];
float theta=0.0f;
point4 newPoint=point4(0.0,  0.0,  0.0, 1.0);
point4 oldPoint=newPoint;
float viewMatrixf[16];
float modelMatrixf[16];
char systemState='f';

// handle to program
GLuint program;
GLuint vColor;

using namespace std;

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

void generateGeometry( void )
{	
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(spherePoints) + sizeof(sphereColors), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(spherePoints), spherePoints );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(spherePoints), sizeof(sphereColors), sphereColors );


	// Load shaders and use the resulting shader program
    program = InitShader( "vshader1.glsl", "fshader1.glsl" );
    glUseProgram( program );
     // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    vColor = glGetAttribLocation( program, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(spherePoints)) );

	// sets the default color to clear screen
    glClearColor( 1.0, 1.0, 1.0, 1.0 ); // white background
}

//----------------------------------------------------------------------------
// this is where the drawing should happen
void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );     // clear the window

	Angel::mat4 perspectiveMat = Angel::Perspective((GLfloat)75.0, (GLfloat)width/(GLfloat)height, (GLfloat)0.01, (GLfloat) 999.0);
	perspectiveMat*=Angel::Translate(0.0, 0.0, -30);
	perspectiveMat*=Angel::RotateY(90.0);

    viewMatrixf[0] = perspectiveMat[0][0];viewMatrixf[4] = perspectiveMat[0][1];
    viewMatrixf[1] = perspectiveMat[1][0];viewMatrixf[5] = perspectiveMat[1][1];
    viewMatrixf[2] = perspectiveMat[2][0];viewMatrixf[6] = perspectiveMat[2][1];
    viewMatrixf[3] = perspectiveMat[3][0];viewMatrixf[7] = perspectiveMat[3][1];

    viewMatrixf[8] = perspectiveMat[0][2];viewMatrixf[12] = perspectiveMat[0][3];
    viewMatrixf[9] = perspectiveMat[1][2];viewMatrixf[13] = perspectiveMat[1][3];
    viewMatrixf[10] = perspectiveMat[2][2];viewMatrixf[14] = perspectiveMat[2][3];
    viewMatrixf[11] = perspectiveMat[3][2];viewMatrixf[15] = perspectiveMat[3][3];

	GLuint viewMatrix = glGetUniformLocationARB(program, "projection_matrix");
    glUniformMatrix4fv( viewMatrix, 1, GL_FALSE, viewMatrixf);

	switch(systemState){
		case 'a':
			drawTree(0);
			break;
		case 'b':
			drawTree(1);
			break;
		case 'c':
			drawTree(2);
			break;
		case 'd':
			drawTree(3);
			break;
		case 'e':
			drawTree(4);
			break;
		default:
			drawForest();
			break;
	}

	glFlush(); // force output to graphics hardware
    // use this call to double buffer
    glutSwapBuffers();
}

void drawForest(){
	int randNumb;
	
	drawGround();

	for(int i=0; i<6; i++){
		randNumb=rand()%5;
		drawTree(randNumb);
	}

	drawCar();
	drawLamp();
	drawBeethoven();
}

// keyboard handler
void keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
    case 033:
        exit( EXIT_SUCCESS );
        break;
	case 'a':
		systemState='a';
		break;
	case 'b':
		systemState='b';
		break;
	case 'c':
		systemState='c';
		break;
	case 'd':
		systemState='d';
		break;
	case 'e':
		systemState='e';
		break;
	case 'f':
		systemState='f';
		break;
    }

	glutPostRedisplay();
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
    glutCreateWindow( "Color Cube" );

	// init glew
    glewInit();

	srand (time(NULL));

	for(int i=0; i<5;i++){
		readLSYSFile(i);
		processLsystem(i);
	}

	readPlyFile("sphere.ply");
	readPlyFile("cylinder.ply");

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

void drawSphere(float x, float y, float z){
    Angel::mat4 modelMat = Angel::Translate(x, y, z);

    modelMatrixf[0] = modelMat[0][0];modelMatrixf[4] = modelMat[0][1];
    modelMatrixf[1] = modelMat[1][0];modelMatrixf[5] = modelMat[1][1];
    modelMatrixf[2] = modelMat[2][0];modelMatrixf[6] = modelMat[2][1];
    modelMatrixf[3] = modelMat[3][0];modelMatrixf[7] = modelMat[3][1];

    modelMatrixf[8] = modelMat[0][2];modelMatrixf[12] = modelMat[0][3];
    modelMatrixf[9] = modelMat[1][2];modelMatrixf[13] = modelMat[1][3];
    modelMatrixf[10] = modelMat[2][2];modelMatrixf[14] = modelMat[2][3];
    modelMatrixf[11] = modelMat[3][2];modelMatrixf[15] = modelMat[3][3];

	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
    glUniformMatrix4fv( modelMatrix, 1, GL_FALSE, modelMatrixf );

    glBufferData( GL_ARRAY_BUFFER, sizeof(spherePoints) + sizeof(sphereColors), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(spherePoints), spherePoints );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(spherePoints), sizeof(sphereColors), sphereColors );
 
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(spherePoints)) );
         
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glEnable( GL_DEPTH_TEST );
    glDrawArrays( GL_TRIANGLES, 0,numberOfSpherePolygons*3 );
    glDisable( GL_DEPTH_TEST ); 
}

void drawCylinder(float x, float y, float z, float angleX, float angleY, float angleZ){
    Angel::mat4 modelMat = Angel::identity();
	modelMat =  modelMat * Angel::Translate(x, y, z);
	modelMat =  modelMat * Angel::RotateX(angleX);
	modelMat = modelMat * Angel::RotateZ(angleZ);
	modelMat = modelMat * Angel::RotateY(angleY);

    modelMatrixf[0] = modelMat[0][0];modelMatrixf[4] = modelMat[0][1];
    modelMatrixf[1] = modelMat[1][0];modelMatrixf[5] = modelMat[1][1];
    modelMatrixf[2] = modelMat[2][0];modelMatrixf[6] = modelMat[2][1];
    modelMatrixf[3] = modelMat[3][0];modelMatrixf[7] = modelMat[3][1];

    modelMatrixf[8] = modelMat[0][2];modelMatrixf[12] = modelMat[0][3];
    modelMatrixf[9] = modelMat[1][2];modelMatrixf[13] = modelMat[1][3];
    modelMatrixf[10] = modelMat[2][2];modelMatrixf[14] = modelMat[2][3];
    modelMatrixf[11] = modelMat[3][2];modelMatrixf[15] = modelMat[3][3];

	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
    glUniformMatrix4fv( modelMatrix, 1, GL_FALSE, modelMatrixf );

	glBufferData( GL_ARRAY_BUFFER, sizeof(cylinderPoints) + sizeof(cylinderColors), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(cylinderPoints), cylinderPoints );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(cylinderPoints), sizeof(cylinderColors), cylinderColors );
 
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(cylinderPoints)) );
         
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glEnable( GL_DEPTH_TEST );
    glDrawArrays( GL_TRIANGLES, 0,numberOfCylinderPolygons*3 );
    glDisable( GL_DEPTH_TEST ); 
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
		//update the position to center
		Angel::mat4 modelMat = Angel::Translate(-(xMin+xMax)/2, -(yMin+yMax)/2, -(zMin+zMax)/2);
		for(int i=0; i<numberOfVertices; i++){
			vertices[i] = modelMat*vertices[i];
		}
	}

	int index=0;
    int triangle;
    int one,two,three;
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
		}
       
	}

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

void drawTree(int index){
	stack<point4> points;
	float angleX=0.0f;
	float angleY=0.0f;
	float angleZ=0.0f;
	stack<float> angleXs;
	stack<float> angleYs;
	stack<float> angleZs;
	point4 direction=point4(0.0, 1.0, 0.0, 0.0);
	
	int randColor=rand()%6;
	int randX=rand()%200;
	int randY=rand()%200;
	int randZ=rand()%200;

	for(int i=0; i<1000; i++){
		sphereColors[i]=vertex_colors[randColor];
	}

	for(int i=0; i<200; i++){
		cylinderColors[i]=vertex_colors[randColor];
	}

	switch(systemState){
	case 'a':
		newPoint=point4(300.0, -90.0, 0.0, 1.0);
		break;
	case 'b':
		newPoint=point4(50.0, -20.0, 0.0, 1.0);
		break;
	case 'c':
		newPoint=point4(0.0, 0.0, 0.0, 1.0);
		break;
	case 'd':
		newPoint=point4(0.0, 0.0, 0.0, 1.0);
		break;
	case 'e':
		newPoint=point4(350.0, -90.0, 0.0, 1.0);
		break;
	default:
		if(index==0)
			newPoint=point4(400.0+randX, -200.0+randY*0.5, -80+randZ*0.9, 1.0);
		else if(index==1)
			newPoint=point4(100.0+randX, -250.0+randY, -80.0+randZ, 1.0);
		else if(index==2)
			newPoint=point4(-50.0+randX, -50.0+randY*0.1, randZ*0.1, 1.0);
		else if(index==3)
			newPoint=point4(-50.0+randX, -50.0+randY*0.1, randZ*0.1, 1.0);
		else 
			newPoint=point4(450.0+randX, -200.0+randY*0.5, -80.0+randZ, 1.0);
		break;
	}

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

void drawGround(void){
	int index;
	for(int i=-2000; i<=2000; i+=200)
		for(int j=-2000; j<=2000; j+=200){
			//use the two dimensional array as an matrix ground
			groundVertices[i/200+10][j/200+10]=point4(i, -400.0, j, 1.0);
		}
	
	index=0;
	point4 color=vertex_colors[6];
	for(int p=0; p<21; p++)
		for(int q=0; q<20; q++){
			groundPoints[index]=groundVertices[p][q];
			groundColors[index++]=color;
			groundPoints[index]=groundVertices[p][q+1];
			groundColors[index++]=color;
		}
	for(int q=0; q<21; q++)
		for(int p=0; p<20; p++){
			groundPoints[index]=groundVertices[p][q];
			groundColors[index++]=color;
			groundPoints[index]=groundVertices[p+1][q];
			groundColors[index++]=color;
		}
	//printf("Total point number for ground: %d\n", index);

	Angel::mat4 modelMat = Angel::Translate(2500.0, 0.0, 0.0);
	modelMat *= Angel::RotateY(-45.0f);

    modelMatrixf[0] = modelMat[0][0];modelMatrixf[4] = modelMat[0][1];
    modelMatrixf[1] = modelMat[1][0];modelMatrixf[5] = modelMat[1][1];
    modelMatrixf[2] = modelMat[2][0];modelMatrixf[6] = modelMat[2][1];
    modelMatrixf[3] = modelMat[3][0];modelMatrixf[7] = modelMat[3][1];

    modelMatrixf[8] = modelMat[0][2];modelMatrixf[12] = modelMat[0][3];
    modelMatrixf[9] = modelMat[1][2];modelMatrixf[13] = modelMat[1][3];
    modelMatrixf[10] = modelMat[2][2];modelMatrixf[14] = modelMat[2][3];
    modelMatrixf[11] = modelMat[3][2];modelMatrixf[15] = modelMat[3][3];

	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
    glUniformMatrix4fv( modelMatrix, 1, GL_FALSE, modelMatrixf );

	glBufferData( GL_ARRAY_BUFFER, sizeof(groundPoints) + sizeof(groundColors), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(groundPoints), groundPoints );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(groundPoints), sizeof(groundColors), groundColors );
 
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(groundPoints)) );
                
    glDrawArrays( GL_LINES, 0, index);

}

void drawCar(void){
	
	readPlyFile("pickup_big.ply");

	Angel::mat4 modelMat = Angel::Translate(0.0, -15.0, -10.0);
	modelMat *= Angel::Scale(2.0, 2.0, 2.0);
	modelMat *= Angel::RotateX(-90.0f);
	modelMat *= Angel::RotateZ(-90.0f);

    modelMatrixf[0] = modelMat[0][0];modelMatrixf[4] = modelMat[0][1];
    modelMatrixf[1] = modelMat[1][0];modelMatrixf[5] = modelMat[1][1];
    modelMatrixf[2] = modelMat[2][0];modelMatrixf[6] = modelMat[2][1];
    modelMatrixf[3] = modelMat[3][0];modelMatrixf[7] = modelMat[3][1];

    modelMatrixf[8] = modelMat[0][2];modelMatrixf[12] = modelMat[0][3];
    modelMatrixf[9] = modelMat[1][2];modelMatrixf[13] = modelMat[1][3];
    modelMatrixf[10] = modelMat[2][2];modelMatrixf[14] = modelMat[2][3];
    modelMatrixf[11] = modelMat[3][2];modelMatrixf[15] = modelMat[3][3];

	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
    glUniformMatrix4fv( modelMatrix, 1, GL_FALSE, modelMatrixf );
	

	point4 carColor=(vertex_colors[1]+vertex_colors[3])/2.0;
	carColor.w=1.0;
	for(int i=0; i<41000; i++){
		colors[i]=carColor;
	}

    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
 
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
         
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glEnable( GL_DEPTH_TEST );
    glDrawArrays( GL_TRIANGLES, 0, numberOfPolygons*3 );
    glDisable( GL_DEPTH_TEST ); 
}

void drawLamp(){
	readPlyFile("street_lamp.ply");
	
	Angel::mat4 modelMat = Angel::Translate(-10.0, -5.0, 12.0);
	//modelMat *= Angel::Scale(1.2, 1.3, 1.3);
	//modelMat *= Angel::RotateX(-90.0f);
	//modelMat *= Angel::RotateZ(-90.0f);

    modelMatrixf[0] = modelMat[0][0];modelMatrixf[4] = modelMat[0][1];
    modelMatrixf[1] = modelMat[1][0];modelMatrixf[5] = modelMat[1][1];
    modelMatrixf[2] = modelMat[2][0];modelMatrixf[6] = modelMat[2][1];
    modelMatrixf[3] = modelMat[3][0];modelMatrixf[7] = modelMat[3][1];

    modelMatrixf[8] = modelMat[0][2];modelMatrixf[12] = modelMat[0][3];
    modelMatrixf[9] = modelMat[1][2];modelMatrixf[13] = modelMat[1][3];
    modelMatrixf[10] = modelMat[2][2];modelMatrixf[14] = modelMat[2][3];
    modelMatrixf[11] = modelMat[3][2];modelMatrixf[15] = modelMat[3][3];

	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
    glUniformMatrix4fv( modelMatrix, 1, GL_FALSE, modelMatrixf );
	

	point4 lampColor=(vertex_colors[4]+vertex_colors[1])/2.0;
	lampColor.w=1.0;
	for(int i=0; i<41000; i++){
		colors[i]=lampColor;
	}

    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
 
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
         
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glEnable( GL_DEPTH_TEST );
    glDrawArrays( GL_TRIANGLES, 0, numberOfPolygons*3 );
    glDisable( GL_DEPTH_TEST ); 
}

void drawBeethoven(){
	readPlyFile("beethoven.ply");

	Angel::mat4 modelMat = Angel::Translate(-12.0, -7.0, 8.0);
	modelMat *= Angel::Scale(0.4, 0.4, 0.4);
	//modelMat *= Angel::RotateX(-90.0f);
	//modelMat *= Angel::RotateZ(-90.0f);
	modelMat *= Angel::RotateY(-110.0f);

    modelMatrixf[0] = modelMat[0][0];modelMatrixf[4] = modelMat[0][1];
    modelMatrixf[1] = modelMat[1][0];modelMatrixf[5] = modelMat[1][1];
    modelMatrixf[2] = modelMat[2][0];modelMatrixf[6] = modelMat[2][1];
    modelMatrixf[3] = modelMat[3][0];modelMatrixf[7] = modelMat[3][1];

    modelMatrixf[8] = modelMat[0][2];modelMatrixf[12] = modelMat[0][3];
    modelMatrixf[9] = modelMat[1][2];modelMatrixf[13] = modelMat[1][3];
    modelMatrixf[10] = modelMat[2][2];modelMatrixf[14] = modelMat[2][3];
    modelMatrixf[11] = modelMat[3][2];modelMatrixf[15] = modelMat[3][3];

	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
    glUniformMatrix4fv( modelMatrix, 1, GL_FALSE, modelMatrixf );
	

	point4 carColor=(vertex_colors[1]+vertex_colors[4])/2.0;
	carColor.w=1.0;
	for(int i=0; i<41000; i++){
		colors[i]=carColor;
	}

    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
 
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
         
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glEnable( GL_DEPTH_TEST );
    glDrawArrays( GL_TRIANGLES, 0, numberOfPolygons*3 );
    glDisable( GL_DEPTH_TEST ); 
}