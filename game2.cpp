#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float person_posx=-4.5,person_posz=-4.5,person_posy=1;
int block_pos=99;
int lives=3;
bool jump=false;
float kn = -8.0 , kp = 8.0 , on = -8.0 , op = 8.0 ;
float eyeview=-1,targetview=6,upview=1;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	/*if (action == GLFW_RELEASE) {
	  switch (key) {
	  case GLFW_KEY_C:
	  rectangle_rot_status = !rectangle_rot_status;
	  break;
	  case GLFW_KEY_P:
	  triangle_rot_status = !triangle_rot_status;
	  break;
	  case GLFW_KEY_X:
	// do something ..
	break;
	default:
	break;
	}
	}
	else if (action == GLFW_PRESS) {
	switch (key) {
	case GLFW_KEY_ESCAPE:
	quit(window);
	break;
	default:
	break;
	}
	}*/

	if(action == GLFW_REPEAT || action==GLFW_PRESS)
	{
		switch(key)
		{
			case GLFW_KEY_RIGHT:
				if(jump==true)
				{
					if(person_posz<3.5)
					{
						person_posz+=2;
						block_pos-=2;
						if(block_pos%8==5)
						{
							cout<<"fallen into pit \nlost life"<<endl;
							block_pos=99;
							person_posy=0;
							lives--;
						}
						else if(block_pos%6==1)
						{
							person_posz-=2;
							block_pos+=2;
						}
					}
					jump=false;
				}
				else if(person_posz<4.5)
				{
					person_posz++;
					block_pos--;
					if(block_pos%8==5)
					{
						cout<<"fallen into pit \nlost life"<<endl;
						block_pos=99;
						person_posy=0;
						lives--;
					}
					else if(block_pos%6==1)
					{
						person_posz--;
						block_pos++;
					}
				}
				cout<<"block "<<block_pos<<endl;
				break;

			case GLFW_KEY_LEFT:
				if(jump==true)
				{
					if(person_posz>-3.5)
					{
						person_posz-=2;
						block_pos+=2;
						if(block_pos%8==5)
						{
							cout<<"fallen into pit \nlost life"<<endl;
							block_pos=99;
							person_posy=0;
							lives--;
						}
						else if(block_pos%6==1)
						{
							person_posz+=2;
							block_pos-=2;
						}
					}
					jump=false;
				}
				else if(person_posz>-4.5)
				{
					person_posz--;
					block_pos++;
					if(block_pos%8==5)
					{
						cout<<"fallen into pit \nlost life"<<endl;
						block_pos=99;
						person_posy=0;
						lives--;
					}
					else if(block_pos%6==1)
					{
						person_posz++;
						block_pos--;
					}

				}
				cout<<"block "<<block_pos<<endl;
				break;

			case GLFW_KEY_UP:
				if(jump==true)
				{
					if(person_posx<3.5)
					{
						person_posx+=2;
						block_pos-=20;
						if(block_pos%8==5)
						{
							cout<<"fallen into pit \nlost life"<<endl;
							block_pos=99;
							person_posy=0;
							lives--;
						}
						else if(block_pos%6==1)
						{
							person_posx-=2;
							block_pos+=20;
						}
					}
					jump=false;
				}
				else if(person_posx<4.5)
				{
					person_posx++;
					block_pos-=10;
					if(block_pos%8==5)
					{
						cout<<"fallen into pit \nlost life"<<endl;
						block_pos=99;
						person_posy=0;
						lives--;
					}
					else if(block_pos%6==1)
					{
						person_posx--;
						block_pos+=10;
					}
				}
				cout<<"block "<<block_pos<<endl;
				break;

			case GLFW_KEY_DOWN:
				if(jump==true)
				{
					if(person_posx>-3.5)
					{
						person_posx-=2;
						block_pos+=20;
						if(block_pos%8==5)
						{
							cout<<"fallen into pit \nlost life"<<endl;
							block_pos=99;
							person_posy=0;
							lives--;
						}
						else if(block_pos%6==1)
						{
							person_posx+=2;
							block_pos-=20;
						}
					}
					jump=false;
				}
				else if(person_posx>-4.5)
				{
					person_posx--;
					block_pos+=10;
					if(block_pos%8==5)
					{
						cout<<"fallen into pit \nlost life"<<endl;
						block_pos=99;
						person_posy=0;
						lives--;
					}
					else if(block_pos%6==1)
					{
						person_posx++;
						block_pos-=10;
					}
				}
				cout<<"block "<<block_pos<<endl;
				break;

			case GLFW_KEY_SPACE:
				jump=true;
				break;

			case GLFW_KEY_A:
				eyeview=-6;
				targetview=6;
				upview=6;
				break;

			case GLFW_KEY_S:
				eyeview=-1;
				targetview=6;
				upview=1;
				break;
			case GLFW_KEY_Z:
				op/=1.25;
				on/=1.25;
				kp/=1.25;
				kn/=1.25;
				break;
		
			case GLFW_KEY_O:
				op*=2.5;
				on*=2.5;
				kp*=2.5;
				kn*=2.5;
				break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE)
				triangle_rot_dir *= -1;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
				rectangle_rot_dir *= -1;
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(kn, kp, on, op, 0.1f, 500.0f);
}

VAO *triangle, *rectangle, *board[105], *person,*obstacle[100];

// Creates the triangle object used in this sample code
void createTriangle ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	static const GLfloat vertex_buffer_data [] = {
		0, 1,0, // vertex 0
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 0
		0,1,0, // color 1
		0,0,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

VAO* createPerson(float width,float length,float height,float r,float g, float b)
{
	GLfloat vertex_buffer_data [] = {
		-width/2,-length/2,-height/2, // vertex 1
		width/2,-length/2,-height/2, // vertex 2
		width/2, length/2,height/2, // vertex 3

		width/2, length/2,-height/2, // vertex 3
		-width/2, length/2,-height/2, // vertex 4
		-width/2,-length/2,-height/2 , // vertex 1

		width/2,length/2,-height/2,
		-width/2,length/2,-height/2,  
		-width/2,length/2,height/2,

		-width/2,length/2,height/2,
		width/2,length/2,height/2,
		width/2,length/2,-height/2,

		width/2,length/2,height/2,
		width/2,length/2,-height/2,
		width/2,-length/2,-height/2,

		width/2,-length/2,-height/2,
		width/2,-length/2,height/2,
		width/2,length/2,height/2,



		-width/2,-length/2,-height/2,
		width/2,-length/2,-height/2,
		width/2,-length/2,height/2,

		width/2,-length/2,height/2,
		-width/2,-length/2,height/2,
		-width/2,-length/2,-height/2,

		width/2,length/2,height/2,
		width/2,-length/2,height/2,
		-width/2,-length/2,height/2,

		-width/2,-length/2,height/2,
		-width/2,length/2,height/2,
		width/2,length/2,height/2,

		-width/2,-length/2,-height/2,
		-width/2,length/2,-height/2, 
		-width/2,length/2,height/2,

		-width/2,length/2,height/2,
		-width/2,-length/2,height/2,
		-width/2,-length/2,-height/2,
	};
	GLfloat color_buffer_data [100];
	for(int i=0;i<12;i++)
	{
		color_buffer_data[3*i]=r;
		color_buffer_data[(3*i)+1]=g;
		color_buffer_data[(3*i)+2]=b;
	}
	return create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}
VAO* createObstacle(float width,float length,float height,float r,float g,float b)
{
	return createPerson(width,length,height,r,g,b);
}
// Creates the rectangle object used in this sample code
VAO* createBoard (float width,float length, float height,float r, float g, float b)
{
	// GL3 accepts only Triangles. Quads are not supported
	GLfloat vertex_buffer_data [] = {
		-width/2,-length/2,-height/2, // vertex 1
		width/2,-length/2,-height/2, // vertex 2
		width/2, length/2,height/2, // vertex 3

		width/2, length/2,-height/2, // vertex 3
		-width/2, length/2,-height/2, // vertex 4
		-width/2,-length/2,-height/2 , // vertex 1

		width/2,length/2,-height/2,
		-width/2,length/2,-height/2,  
		-width/2,length/2,height/2,

		-width/2,length/2,height/2,
		width/2,length/2,height/2,
		width/2,length/2,-height/2,

		width/2,length/2,height/2,
		width/2,length/2,-height/2,
		width/2,-length/2,-height/2,

		width/2,-length/2,-height/2,
		width/2,-length/2,height/2,
		width/2,length/2,height/2,



		-width/2,-length/2,-height/2,
		width/2,-length/2,-height/2,
		width/2,-length/2,height/2,

		width/2,-length/2,height/2,
		-width/2,-length/2,height/2,
		-width/2,-length/2,-height/2,

		width/2,length/2,height/2,
		width/2,-length/2,height/2,
		-width/2,-length/2,height/2,

		-width/2,-length/2,height/2,
		-width/2,length/2,height/2,
		width/2,length/2,height/2,

		-width/2,-length/2,-height/2,
		-width/2,length/2,-height/2, 
		-width/2,length/2,height/2,

		-width/2,length/2,height/2,
		-width/2,-length/2,height/2,
		-width/2,-length/2,-height/2,
	};
//	0.6 - 0.1
//	0.2 - 0.4
//	1 - 0.9
	GLfloat color_buffer_data []={

		0.2, 0.1,0.7,
		0.2, 0.1,0.7,
		0.2, 0.1,0.7,

		0.2, 0.1,0.7,
		0.2, 0.1,0.7,
		0.2, 0.1,0.7,

		0.4,0.5,0.5,
		0.4,0.5,0.5,
		0.4,0.5,0.5,

		0.4,0.5,0.5,
		0.4,0.5,0.5,
		0.4,0.5,0.5,

		0.2,0.4,0.6,
		0.2,0.4,0.6,
		0.2,0.4,0.6,

		0.2,0.4,0.6,
		0.2,0.4,0.6,
		0.2,0.4,0.6,

		0.1,0.2,0.6,
		0.1,0.2,0.6,
		0.1,0.2,0.6,

		0.1,0.2,0.6,
		0.1,0.2,0.6,
		0.1,0.2,0.6,

		0.1,0.7,0.7,
		0.1,0.7,0.7,
		0.1,0.7,0.7,

		0.1,0.7,0.7,
		0.1,0.7,0.7,
		0.1,0.7,0.7,

		0.4,0.4,0.4,
		0.4,0.4,0.4,
		0.4,0.4,0.4,

		0.4,0.4,0.4,
		0.4,0.4,0.4,
		0.4,0.4,0.4,


	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	return create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(eyeview,targetview,upview), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
	//Matrix.view=glm::lookAt(0.0, 0.0, 0.0, 0.0, 0.0, -100.0, 0.0, 1.0, 0.0);
	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	//Matrices.model = glm::mat4(1.0f);

	/* Render your scene */

	/* glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef

	   glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	   glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
	   Matrices.model *= triangleTransform; 
	   MVP = VP * Matrices.model; // MVP = p * V * M

	//  Don't change unless you are sure!!
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(triangle);
	*/
	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	int index=0,oindex=0;
	for(float i=4.5;i>=-4.5;i--)
	{
		for(float j=4.5;j>=-4.5;j--)
		{
			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateBlock = glm::translate (glm::vec3(i, 0, j));        // glTranslatef
			//glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			Matrices.model =translateBlock;
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

			// draw3DObject draws the VAO given to it using current MVP matrix
			if(index%8!=5)
			{
				draw3DObject(board[index]);



				if(index%6==1)
				{
					Matrices.model = glm::mat4(1.0f);

					glm::mat4 translateObstacle = glm::translate (glm::vec3(i, 1,j ));        // glTranslatef
					//glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
					Matrices.model =translateObstacle;
					MVP = VP * Matrices.model;
					glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

					// draw3DObject draws the VAO given to it using current MVP matrix
					draw3DObject(obstacle[oindex++]);
				}
			}

			index++;
		}
	}
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translatePerson = glm::translate (glm::vec3(person_posx, person_posy,person_posz ));        // glTranslatef
	//glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model =translatePerson;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(person);

	if(person_posy==0)
	{
		person_posy=1;
		person_posz=-4.5;
		person_posx=-4.5;
	}

	// Increment angles
	float increments = 1;

	//camera_rotation_angle++; // Simulating camera rotation
	triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Game", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	//createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	for(int i=0;i<100;i++)
	{
		board[i]=createBoard (1,1,1,0,0,0);
	}

	for(int i=0;i<30;i++)
	{
		obstacle[i]=createObstacle(0.8,1,0.8,0.4,0.2,0);
	}	


	person=createPerson(0.4,1,0.4,1,0,0.5);
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.1f, 0.1f, 0.8f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		draw();
		reshapeWindow ( window , width , height );
		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 1) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
		}
		if(lives==0)
		{
			cout<<"LOST THE GAME"<<endl;
			break;
		}
		if(block_pos==0)
		{
			cout<<"YOU WON THE GAME"<<endl;
			break;
		}
	}

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
