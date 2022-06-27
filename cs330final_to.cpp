#include <iostream>         
#include <cstdlib>         
#include <GL/glew.h>        
#include <GLFW/glfw3.h> 

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//TEDDIE - include camera for viewing
#include "tutorial_05_05/camera.h"
//TEDDIE - include math for helpfulness with the cylinder
#include <corecrt_math_defines.h>

//TEDDIE - includes for images
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "tutorial_05_05/sphere.h"
#include "tutorial_05_05/SphereCite.h"
#include "tutorial_05_05/cyl.h"
#include "tutorial_05_05/coaster.h"
#include "tutorial_05_05/lime.h"
#include "tutorial_05_05/Bmp.h"

//TEDDIE - Set up namespace
using namespace std;

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    //TEDDIE - Creat namefor window
    const char* const WINDOW_TITLE = "Teddie O'Ceallaigh's Final Project"; 
    const int STRIDE = 7;

    //TEDDIE - resize window for better viewing
    const int WINDOW_WIDTH = 1400;
    const int WINDOW_HEIGHT = 1000;

    //TEDDIE - Structure mesh to store VAOs, VBOs, and Vertices
    // Stores the GL data relative to a given mesh


    struct GLMesh
    {

        GLuint vao[20];         // Handle for the vertex array object
        GLuint vbo[20];         // Handle for the vertex buffer object
        GLuint nVertices[20];    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;

    //TEDDIE - Texture name initializing Texture
    GLuint planeTex, morbidTex, drinkTex, sphereTex, limeTex, rindTex, corkTex, ceramicTex;
    //TEDDIE - Set up for uv / texture coordinates
    glm::vec2 gUVScale(5.0f, 5.0f);
    //TEDDIE - set textures to clamp to edge
    GLint gTexWrapMode = GL_CLAMP_TO_EDGE;

    //TEDDIE - other options per source: https://learnopengl.com/Getting-started/Textures
    //GLint gTexWrapMode = GL_REPEAT; The default behavior for textures.Repeats the texture image.
    //GLint gTexWrapMode = GL_MIRRORED_REPEAT; Same as GL_REPEAT but mirrors the image with each repeat.
    //GLint gTexWrapMode = GL_CLAMP_TO_EDGE; Clamps the coordinates between 0 and 1. The result is that higher coordinates become clamped to the edge, resulting in a stretched edge pattern.
    //GLint gTexWrapMode = GL_CLAMP_TO_BORDER; Coordinates outside the range are now given a user - specified border color.

    //TEDDIE - Set up shader programs

    GLuint gProgramId;
    GLuint gLightId;


   //TEDDIE - camera set up
    Camera gCamera(glm::vec3(0.0f, 0.0f, 8.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;
    bool viewProjection = true;
    float cameraSpeed = 0.05f;

    //TEDDIE - timing float initialized
    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    // Cube and light color
   glm::vec3 gObjectColor(1.0f, 0.2f, 0.0f);

    // Light color, position, and scale
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 gLightPosition(1.0f, 3.0f, -3.0f);
    glm::vec3 gLightScale(0.25f);

    //cube position
    glm::vec3 gCubePosition(0.75f, -0.1f, -1.0f);
    glm::vec3 gCubeScale(2.0f);

    glm::vec3 gFillColor(1.0f, 0.9f, 0.2f);
    glm::vec3 gFillPosition(-8.0f, 11.5f, 7.0f);
    glm::vec3 gFillScale(1.3f);


    // Lamp animation
    bool gLampIsOrbiting = true;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);




/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,

	layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
	layout(location = 1) in vec3 normal; // VAP position 1 for normals
	layout(location = 2) in vec2 textureCoordinate;

	out vec3 vertexNormal; // For outgoing normals to fragment shader
	out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
	out vec2 vertexTextureCoordinate;

	//Uniform / Global variables for the  transform matrices
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

	vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

	vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
	vertexTextureCoordinate = textureCoordinate;
}
);

/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,

	in vec3 vertexNormal; // For incoming normals
	in vec3 vertexFragmentPos; // For incoming fragment position
	in vec2 vertexTextureCoordinate;

	out vec4 fragmentColor; // For outgoing cube color to the GPU
	out vec4 fillFragmentColor;
	// Uniform / Global variables for light color, light position, and camera/view position
	uniform vec3 objectColor;
	uniform vec3 lightColor;
	uniform vec3 lightPos;
	uniform vec3 fillColor;
	uniform vec3 fillPos;
	uniform vec3 viewPosition;
	uniform sampler2D uTexture; // Useful when working with multiple textures
	uniform vec2 uvScale;

void main()
{
    //TEDDIE - phong lighting for ambient, diffuse and spec lighting
    // 
    //TEDDIE - key lighting calculations
    //TEDDIE - amb lighting strength for key light at 50%
    float ambientStrength = 0.2f; 
    //TEDDIE - amb light color
    vec3 ambient = ambientStrength * lightColor;

    //TEDDIE - Diffuse lighting
    //TEDDIE -  norm vectors to 1
    vec3 norm = normalize(vertexNormal);
    //TEDDIE - calc light distancing 
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); 
    //TEDDIE - calc diffuse impact
    float impact = max(dot(norm, lightDirection), 0.0);
    //TEDDIE - difuse light color
    vec3 diffuse = impact * lightColor;

    //TEDDIE - specular lighting - spec
    //TEDDIE - spec light strength
    float specularIntensity = 0.3f; 
    //TEDDIE - highlight sizing
    float highlightSize = 2.0f;
    //TEDDIE - view direction
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos);
    //TEDDIE - reflection vector calcs
    vec3 reflectDir = reflect(-lightDirection, norm);
    //TEDDIE - spec component calcs
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    //TEDDIE - spec calcs
    vec3 specular = specularIntensity * specularComponent * lightColor;


    //TEDDIE - Fill lighting
    //TEDDIE -  ambient fill strength
    float fillAmbientStrength = 0.1f; 
    //TEDDIE - amb light color
    vec3 fillAmbient = fillAmbientStrength * fillColor; 


    //TEDDIE - calc light distancing 
    vec3 fillDirection = normalize(fillPos - vertexFragmentPos); 
    //TEDDIE - calc diffuse impact
    float fillImpact = max(dot(norm, fillDirection), 0.0);
    //TEDDIE - calc diffuse impact
    vec3 fillDiffuse = fillImpact * fillColor; 

    //TEDDIE - specular fill lighting - spec
    //TEDDIE - spec light strength
    float fillSpecularIntensity = 0.5f; 
    //TEDDIE - highlight sizing
    float fillHighlightSize = 8.0f; 
    //TEDDIE - view direction
    vec3 fillViewDir = normalize(viewPosition - vertexFragmentPos); 
    //TEDDIE - reflection vector calcs
    vec3 fillReflectDir = reflect(-fillDirection, norm);
    //TEDDIE - spec component calcs
    float fillSpecularComponent = pow(max(dot(fillViewDir, fillReflectDir), 0.0), fillHighlightSize);
    //TEDDIE - spec calcs
    vec3 fillSpecular = fillSpecularIntensity * fillSpecularComponent * fillColor;

    //TEDDIE - calc Phong results
    vec3 objectColor = texture(uTexture, vertexTextureCoordinate).xyz;
    //TEDDIE - key light totals
    vec3 keyResult = (ambient + diffuse + specular);
    //TEDDIE - fill light totals
    vec3 fillResult = (fillAmbient + fillDiffuse + fillSpecular);
    //TEDDIE - all lighting totals
    vec3 lightingResult = keyResult + fillResult;
    //TEDDIE - phong results
    vec3 phong = (lightingResult)*objectColor;

    //TEDDIE - fragment adjust as neede
    fragmentColor = vec4(phong, 1.0f);
}
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

        //Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);


const GLchar* fillVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

        //Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* fillFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);



// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object



    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;
    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLightId))
        return EXIT_FAILURE;

    //TEDDIE -  try nwe way for texture loading due to repeated issues
    const char* planeTexFilename = "C:/Users/teddi/OneDrive/Desktop/SNHU/CS330/FINAL/textures/plane.jpg";
    const char* morbidTexFilename = "C:/Users/teddi/OneDrive/Desktop/SNHU/CS330/FINAL/textures/box.jpg";
    const char* drinkTexFilename = "C:/Users/teddi/OneDrive/Desktop/SNHU/CS330/FINAL/textures/drink.jpg";
    const char* sphereTexFilename = "C:/Users/teddi/OneDrive/Desktop/SNHU/CS330/FINAL/textures/sphere.jpg";
    const char* limeTexFilename = "C:/Users/teddi/OneDrive/Desktop/SNHU/CS330/FINAL/textures/lime.jpg";
    const char* rindTexFilename = "C:/Users/teddi/OneDrive/Desktop/SNHU/CS330/FINAL/textures/rind.jpg";
    const char* corkTexFilename = "C:/Users/teddi/OneDrive/Desktop/SNHU/CS330/FINAL/textures/cork.jpg";
    const char* ceramicTexFilename = "C:/Users/teddi/OneDrive/Desktop/SNHU/CS330/FINAL/textures/ceramic.jpg";

    if (!UCreateTexture(planeTexFilename, planeTex))
    {
        cout << "Failed to load texture " << planeTexFilename << endl;
        return EXIT_FAILURE;
    }
    if (!UCreateTexture(morbidTexFilename, morbidTex))
    {
        cout << "Failed to load texture " << morbidTexFilename << endl;
        return EXIT_FAILURE;
    }
    if (!UCreateTexture(drinkTexFilename, drinkTex))
    {
        cout << "Failed to load texture " << drinkTexFilename << endl;
        return EXIT_FAILURE;
    }
    if (!UCreateTexture(sphereTexFilename, sphereTex))
    {
        cout << "Failed to load texture " << sphereTexFilename << endl;
        return EXIT_FAILURE;
    }
    if (!UCreateTexture(limeTexFilename, limeTex))
    {
        cout << "Failed to load texture " << limeTexFilename << endl;
        return EXIT_FAILURE;
    }
    if (!UCreateTexture(rindTexFilename, rindTex))
    {
        cout << "Failed to load texture " << rindTexFilename << endl;
        return EXIT_FAILURE;
    }
    if (!UCreateTexture(corkTexFilename, corkTex))
    {
        cout << "Failed to load texture " << corkTexFilename << endl;
        return EXIT_FAILURE;
    }
    if (!UCreateTexture(ceramicTexFilename, ceramicTex))
    {
        cout << "Failed to load texture " << ceramicTexFilename << endl;
        return EXIT_FAILURE;
    }

  

    //TEDDIE - set texture to which program
    glUseProgram(gProgramId);
    //TEDDIE - set unit as 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    //TEDDIE - set background o black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------

    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);

    //TEDDIE - release textures
    UDestroyTexture(planeTex);
    UDestroyTexture(morbidTex);
    UDestroyTexture(drinkTex);
    UDestroyTexture(sphereTex);
    UDestroyTexture(limeTex);
    UDestroyTexture(rindTex);
    UDestroyTexture(corkTex);
    UDestroyTexture(ceramicTex);

    //TEDDIE - release shaders
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gLightId);


    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);


    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}

//TEDDIE _ DO NOT TOUCH!!!!!!!!!!!!!!!!!
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 4.5f;

    //TEDDIE - use P to create ortho-projection
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {

        gCamera.Position = glm::vec3(-3.0f, 5.0f, -2.0f);
        gCamera.Pitch = -100.0f;
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    //TEDDIE - use O to reset ortho-projection
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {

        gCamera.Position = glm::vec3(0.0f, 0.0f, 10.0f);
        gCamera.Pitch = 0.0f;
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    //TEDDIE - use esc to exit
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    //TEDDIE - Move the pyramid Forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    //TEDDIE - Move the pyramid Backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    //TEDDIE - Move the pyramid Left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    //TEDDIE - Move the pyramid Right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    //TEDDIE - Move the pyramid Upward
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    //TEDDIE - Move the pyramid Downward
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);



}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}



// Functioned called to render a frame
void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //TEDDIE - set shader to use
    glUseProgram(gProgramId);

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Creates a projection or Ortho view
    glm::mat4 projection;
    if (viewProjection) {
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else {
        float scale = 120;
        projection = glm::ortho((800.0f / scale), -(800.0f / scale), -(600.0f / scale), (600.0f / scale), -2.5f, 6.5f);
    }


    //TEDDIE - CONES FOR DRINK AND LIGHTING
    //TEDIE _ DO NOT TOUCH THIS CONE!!!
    float spike = 0.25f;
    glm::mat4 scale = glm::scale(glm::vec3(spike, 0.9f, spike));
    glm::mat4 rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
    glm::mat4 translation = glm::translate(glm::vec3(-0.545f, -0.5f, 0.0f));
    glm::mat4 model = translation * rotation * scale;

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    GLint keyLightColorLoc = glGetUniformLocation(gProgramId, "keyLightColor");
    GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[0]);
    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, drinkTex);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[0]);


    //TEDDIE - Cone 2
    //TEDDIE - DO NOT TOUCH THIS IT IS IN PLACE
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, drinkTex);
    scale = glm::scale(glm::vec3(spike, 0.6f, spike));
    rotation = glm::rotate(3.1415f, glm::vec3(1.0f, 0.0f, 0.0f));
    translation = glm::translate(glm::vec3(-0.545f, 0.3f, 0.0f));
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[1]);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[1]);


    //TEDDIE - PLANE
    //TEDDIE _ IN POSITION DO NOT TOUCH
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planeTex);
    scale = glm::scale(glm::vec3(2.0f, 1.0f, 1.5f));
    translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
    model = translation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[2]);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[2]);

    //TEDDIE - ceramic FOR coasters
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, corkTex);
    glBindVertexArray(gMesh.vao[3]);
    //TEDDIE - DONT CHANGE THIS IS IN POSITION!!!!!!
    scale = glm::scale(glm::vec3(0.26f, 0.005f, 0.26f));
    rotation = glm::rotate(45.0f, glm::vec3(0.0, 1.0f, 0.0f));
    translation = glm::translate(glm::vec3(-0.76f, -0.425f, -0.32f));
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    static_meshes_3D::Cylinder C1(1, 30, 3, true, true, true);
    C1.render();

    //TEDDIE - ceramic FOR coasters
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, corkTex);
    glBindVertexArray(gMesh.vao[4]);
    //TEDDIE - DONT CHANGE THIS IS IN POSITION!!!!!!
    scale = glm::scale(glm::vec3(0.26f, 0.005f, 0.26f));
    rotation = glm::rotate(45.0f, glm::vec3(0.0, 1.0f, 0.0f));
    translation = glm::translate(glm::vec3(-0.90f, -0.445f, -0.33f));
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    static_meshes_3D::Cylinder C2(1, 30, 3, true, true, true);
    C2.render();

    //TEDDIE - cork for coasters
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, corkTex);
    glBindVertexArray(gMesh.vao[5]);
    //TEDDIE - DONT CHANGE THIS IS IN POSITION!!!!!!
    scale = glm::scale(glm::vec3(0.26f, 0.005f, 0.26f));
    rotation = glm::rotate(45.0f, glm::vec3(0.0, 1.0f, 0.0f));
    translation = glm::translate(glm::vec3(-0.98f, -0.465f, -0.26f));
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    static_meshes_3D::Cylinder C3(1, 30, 3, true, true, true);
    C3.render();

    //TEDDIE - cork for coasters
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, corkTex);
    glBindVertexArray(gMesh.vao[6]);
    //TEDDIE - DONT CHANGE THIS IS IN POSITION!!!!!!
    scale = glm::scale(glm::vec3(0.26f, 0.005f, 0.26f));
    rotation = glm::rotate(45.0f, glm::vec3(0.0, 1.0f, 0.0f));
    translation = glm::translate(glm::vec3(-0.98f, -0.485f, -0.18f));
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    static_meshes_3D::Coaster C4(1, 30, 3, true, true, true);
    C4.render();

    //TEDDIE - ceramic for coasters
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ceramicTex);
    glBindVertexArray(gMesh.vao[7]);
    //TEDDIE - DONT CHANGE THIS IS IN POSITION!!!!!!
    scale = glm::scale(glm::vec3(0.27f, 0.005f, 0.27f));
    rotation = glm::rotate(45.0f, glm::vec3(0.0, 1.0f, 0.0f));
    translation = glm::translate(glm::vec3(-0.76f, -0.43f, -0.32f));
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    static_meshes_3D::Coaster C5(1, 30, 3, true, true, true);
    C5.render();

    //TEDDIE - ceramic for coasters
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ceramicTex);
    glBindVertexArray(gMesh.vao[8]);
    //TEDDIE - DONT CHANGE THIS IS IN POSITION!!!!!!
    scale = glm::scale(glm::vec3(0.27f, 0.005f, 0.27f));
    rotation = glm::rotate(45.0f, glm::vec3(0.0, 1.0f, 0.0f));
    translation = glm::translate(glm::vec3(-0.90f, -0.45f, -0.33f));
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    static_meshes_3D::Coaster C6(1, 30, 3, true, true, true);
    C6.render();

    //TEDDIE - ceramic for coasters
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ceramicTex);
    glBindVertexArray(gMesh.vao[9]);
    //TEDDIE - DONT CHANGE THIS IS IN POSITION!!!!!!
    scale = glm::scale(glm::vec3(0.27f, 0.005f, 0.27f));
    rotation = glm::rotate(45.0f, glm::vec3(0.0, 1.0f, 0.0f));
    translation = glm::translate(glm::vec3(-0.98f, -0.47f, -0.26f));
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    static_meshes_3D::Coaster C7(1, 30, 3, true, true, true);
    C7.render();

    //TEDDIE - ceramic for coasters
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ceramicTex);
    glBindVertexArray(gMesh.vao[10]);
    //TEDDIE - DONT CHANGE THIS IS IN POSITION!!!!!!
    scale = glm::scale(glm::vec3(0.27f, 0.005f, 0.27f));
    rotation = glm::rotate(45.0f, glm::vec3(0.0, 1.0f, 0.0f));
    translation = glm::translate(glm::vec3(-0.98f, -0.49f, -0.18f));
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    static_meshes_3D::Coaster C8(1, 30, 3, true, true, true);
    C8.render();



    //TEDDIE - INSIDE LIME
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, limeTex);
    scale = glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
    translation = glm::translate(glm::vec3(-1.0f, 1.5f, -0.6f));
    model = translation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    //Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[11]);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[11]);

    //TEDDIE - MORBID BOX
    //TEDDIE - COMPLETED DO NOT TOUCH AGAIN
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, morbidTex);
    scale = glm::scale(glm::vec3(0.5f, 0.35f, 0.35f));
    rotation = glm::rotate(15.0f, glm::vec3(0.0, 0.27f, 0.0f));
    translation = glm::translate(glm::vec3(0.7f, -0.3f, -0.4f));
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    //Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[12]);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[12]);

    //TEDDIE - DOME THING RIND
    //TEDDIE - COMPLETED DO NOT TOUCH AGAIN
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, rindTex);
    //scale = glm::scale(glm::vec3(0.23f, 0.16f, 0.16f));
    //rotation = glm::rotate(15.0f, glm::vec3(1.5, 0.0f, 1.0f));
    //translation = glm::translate(glm::vec3(1.0f, -0.45f, -0.45f));
    //model = translation * rotation * scale;
    //modelLoc = glGetUniformLocation(gProgramId, "model");
    //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    //Activate the VBOs contained within the mesh's VAO
    //glBindVertexArray(gMesh.vao[13]);
    // Draws the triangles
    //glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[13]);

    //TEDDIE - DOME THING 2
//TEDDIE - COMPLETED DO NOT TOUCH AGAIN
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, limeTex);
    scale = glm::scale(glm::vec3(0.22f, 0.15f, 0.15f));
    rotation = glm::rotate(90.0f, glm::vec3(1.5f, -0.4f, 0.5f));
    translation = glm::translate(glm::vec3(0.0f, -0.45f, -0.45f));
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    //Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[14]);
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[14]);

    

    //TEDDIE - LIME RIND
    Sphere sphere(1.0f, 72, 24, false);
    sphere.setRadius(2.0f);
    sphere.setSectorCount(72);
    sphere.setStackCount(24);
    sphere.setSmooth(false);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sphereTex);
    sphere.draw();

    

    //TEDDIE - SPHERE FOR ON DRINK THING
    //TEDDIE - DONT CHANGE THIS IS IN POSITION!!!!!!
    Sphere2 orb(0.4f, 30, 10);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sphereTex);
    scale = glm::scale(glm::vec3(0.35f, 0.35f, 0.35f));
    rotation = glm::rotate(45.0f, glm::vec3(-0.85f, -0.7f, 0.1f));
    translation = glm::translate(glm::vec3(-0.545f, 0.38f, 0.0f));
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    orb.Draw();

    //TEDDIE - LIME RIND
    //TEDDIE - COME BACK TO THIS
    Sphere lime2(0.3f, 30, 10);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rindTex);
    //TEDDIE - DONT CHANGE THIS IS IN POSITION!!!!!!
    scale = glm::scale(glm::vec3(0.45f, 0.45f, 0.45f));
    rotation = glm::rotate(45.0f, glm::vec3(-1.85f, -0.7f, 0.1f));
    translation = glm::translate(glm::vec3(0.0f, -0.4f, -0.5f));
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    lime2.draw();



    //TEDDIE - SET UP THE LAMP PROGRAM
    glUseProgram(gLightId);

    //Transform the cone used as a visual que for the light source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLightId, "model");
    viewLoc = glGetUniformLocation(gLightId, "view");
    projLoc = glGetUniformLocation(gLightId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(gMesh.vao[0]);
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[0]);

    // Deactivate the Vertex Array Object and shader program
    glBindVertexArray(0);


    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.


}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{

    //TEDDIE - DO NOT TOUCH THIS IS PERFECT AND YOU WILL HATE YOURSELF IF YOU MUCK THIS UP
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    //TEDDIE - DO NOT TOUCH THIS IS PERFECT AND YOU WILL HATE YOURSELF IF YOU MUCK THIS UP
    GLfloat planeVerts[] = {
        //TEDDIE - create plane with pos/norms/texs 

                //TEDDIE - TRIANGLE 1
                // 
                //TEDDIE - Vertex 0
                -0.7f, -0.5f, -0.5f,	0.0f, 1.0f,  0.0f,		0.0f, 1.0f,
                //TEDDIE - Vertex 1
                0.7f, -0.5f, -0.5f,		0.0f, 1.0f,  0.0f,		1.0f, 1.0f,
                //TEDDIE - Vertex 2
                0.7f, -0.5f,  0.5f,		0.0f, 1.0f,  0.0f,		1.0f, 0.0f,

                //TEDDIE - TRIANGLE 2
                // 
                //TEDDIE - Vertex 2
                0.7f, -0.5f,  0.5f,		0.0f, 1.0f,  0.0f,		1.0f, 0.0f,
                //TEDDIE - Vertex 3
                -0.7f, -0.5f,  0.5f,	0.0f, 1.0f,  0.0f,		0.0f, 0.0f,
                //TEDDIE - Vertex 0
                -0.7f, -0.5f, -0.5f,	0.0f, 1.0f,  0.0f,		0.0f, 1.0f,
    };

    GLfloat cubeVerts[] = {
        //TEDDIE - DO NOT TOUCH THIS IS PERFECT AND YOU WILL HATE YOURSELF IF YOU MUCK THIS UP
        //TEDDIE - create MORBID CARD game box with pos/norms/texs 
        // 
        //TEDDIE - BACK LOWER RIGHT
         0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        0.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        1.0f, 0.0f,
          -0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        1.0f, 1.0f,
          //TEDDIE - BACK UPPER LEFT
          -0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        0.0f, 0.0f,

         //TEDDIE - FRONT LOWER RIGHT
         -0.5f, -0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        0.0f, 0.0f,
          0.5f, -0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        1.0f, 1.0f,
          //TEDDIE - FRONT UPPER LEFT
          0.5f,  0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        1.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        0.0f, 0.0f,

         //TEDDIE - LEFT UPPER LEFT
         -0.5f,  -0.5f,  0.5f,      -1.0f,  0.0f,  0.0f,        1.0f, 0.0f,
         -0.5f,  0.5f, 0.5f,      -1.0f,  0.0f,  0.0f,        1.0f, 1.0f,
         -0.5f, 0.5f, -0.5f,      -1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
         //TEDDIE - LEFT LOWER RIGHT
         -0.5f, 0.5f, -0.5f,      -1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
         -0.5f, -0.5f,  -0.5f,      -1.0f,  0.0f,  0.0f,        0.0f, 0.0f,
         -0.5f,  -0.5f,  0.5f,      -1.0f,  0.0f,  0.0f,        1.0f, 0.0f,

         //TEDDIE - RIGHT UPPER RIGHT
          0.5f,  -0.5f,  -0.5f,       1.0f,  0.0f,  0.0f,        1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,       1.0f,  0.0f,  0.0f,        1.0f, 1.0f,
          0.5f, 0.5f, 0.5f,       1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
          //TEDDIE - RIGHT LOWER LEFT
          0.5f, 0.5f, 0.5f,       1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.5f,  0.5f,       1.0f,  0.0f,  0.0f,        0.0f, 0.0f,
          0.5f,  -0.5f,  -0.5f,       1.0f,  0.0f,  0.0f,        1.0f, 0.0f,

          //TEDDIE - BOTTOM RIGHT
         -0.5f, -0.5f, -0.5f,       0.0f, -1.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.5f, -0.5f,       0.0f, -1.0f,  0.0f,        1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,       0.0f, -1.0f,  0.0f,        1.0f, 0.0f,
          //TEDDIE - BOTTOM LEFT
          0.5f, -0.5f,  0.5f,       0.0f, -1.0f,  0.0f,        1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,       0.0f, -1.0f,  0.0f,        0.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,       0.0f, -1.0f,  0.0f,        0.0f, 1.0f,

         //TEDDIE - TOP RIGHT
         0.5f,  0.5f, 0.5f,       0.0f,  1.0f,  0.0f,        0.0f, 1.0f,
          -0.5f,  0.5f, 0.5f,       0.0f,  1.0f,  0.0f,        1.0f, 1.0f,
          -0.5f,  0.5f,  -0.5f,       0.0f,  1.0f,  0.0f,        1.0f, 0.0f,
          //TEDDIE - TOP LEFT
         -0.5f,  0.5f,  -0.5f,       0.0f,  1.0f,  0.0f,        1.0f, 0.0f,
         0.5f,  0.5f,  -0.5f,       0.0f,  1.0f,  0.0f,        0.0f, 0.0f,
         0.5f,  0.5f, 0.5f,       0.0f,  1.0f,  0.0f,        0.0f, 1.0f
    };

   
    GLfloat coneVerts[] = {

        //TEDDIE - Triangle 1

        //TEDDIE - Vertex 0 
        0.0f, 1.0f, 0.0f,   0.0f,  0.0f, -1.0f,     0.5f, 1.0f,
        //TEDDIE - Vertex 1
        0.5f, 0.0f, 0.0f,   0.0f,  0.0f, -1.0f,     1.0f, 1.0f,
        //TEDDIE - Vertex 2 
        0.45f, 0.0f, 0.2f,   0.0f,  0.0f, -1.0f,     0.0f, 1.0f,

        //TEDDIE - Triangle 2

        //TEDDIE - Vertex 0 
        0.0f, 1.0f, 0.0f,   0.0f,  0.0f,  1.0f,     0.5f, 1.0f,
        //TEDDIE - Vertex 16
        0.45f, 0.0f, -0.2f,   0.0f,  0.0f,  1.0f,     0.0f, 1.0f,
        //TEDDIE - Vertex 1
        0.5f, 0.0f, 0.0f,   0.0f,  0.0f,  1.0f,     1.0f, 1.0f,

        //TEDDIE - Triangle 3

        //TEDDIE - Vertex 0 
        0.0f, 1.0f, 0.0f,   -1.0f,  0.0f,  0.0f,    0.5f, 1.0f,
        //TEDDIE - Vertex 15
        0.35f, 0.0f, -0.35f,  -1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
        //TEDDIE - Vertex 16
        0.45f, 0.0f, -0.2f,  -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,

        //TEDDIE - Triangle 4

        //TEDDIE - Vertex 0
        0.0f, 1.0f, 0.0f,   1.0f,  0.0f,  0.0f,     0.5f, 1.0f,
        //TEDDIE - Vertex 14
        0.2f, 0.0f, -0.45f,   1.0f,  0.0f,  0.0f,     0.0f, 1.0f,
        //TEDDIE - Vertex 15
        0.35f, 0.0f, -0.35f,  1.0f,  0.0f,  0.0f,     1.0f, 1.0f,

        //TEDDIE - Triangle 5

        //TEDDIE - Vertex 0
        0.0f, 1.0f, 0.0f,   0.0f, -1.0f,  0.0f,     0.5f, 1.0f,
        //TEDDIE - Vertex 13
        0.0f, 0.0f, -0.5f,   0.0f, -1.0f,  0.0f,     1.0f, 1.0f,
        //TEDDIE - Vertex 14
        0.2f, 0.0f, -0.45f,  0.0f, -1.0f,  0.0f,     0.0f, 1.0f,

        //TEDDIE - Triangle 6

        //TEDDIE - Vertex 0
        0.0f, 1.0f, 0.0f,   0.0f,  1.0f,  0.0f,     0.5f, 1.0f,
        //TEDDIE - Vertex 12
        -0.2f, 0.0f, -0.45f,  0.0f,  1.0f,  0.0f,     0.0f, 1.0f,
        //TEDDIE - Vertex 13
        0.0f, 0.0f, -0.5f,   0.0f,  1.0f,  0.0f,     1.0f, 1.0f,

        //TEDDIE - Triangle 7

        //TEDDIE - Vertex 0 
        0.0f, 1.0f, 0.0f,   0.0f,  0.0f, -1.0f,     0.5f, 1.0f,
        //TEDDIE - Vertex 11
        -0.35f, 0.0f, -0.35f,   0.0f,  0.0f, -1.0f,     1.0f, 1.0f,
        //TEDDIE - Vertex 12
        -0.2f, 0.0f, -0.45f,   0.0f,  0.0f, -1.0f,     0.0f, 1.0f,

        //TEDDIE - Triangle 8

        //TEDDIE - Vertex 0 
        0.0f, 1.0f, 0.0f,   0.0f,  0.0f,  1.0f,     0.5f, 1.0f,
        //TEDDIE - Vertex 10
        -0.45f, 0.0f, -0.2f,   0.0f,  0.0f,  1.0f,     0.0f, 1.0f,
        //TEDDIE - Vertex 11
        -0.35f, 0.0f, -0.35f,   0.0f,  0.0f,  1.0f,     1.0f, 1.0f,

        //TEDDIE - Triangle 9

        //TEDDIE - Vertex 0 
        0.0f, 1.0f, 0.0f,   0.0f,  0.0f, -1.0f,     0.5f, 1.0f,
        //TEDDIE - Vertex 9
        -0.5f, 0.0f, 0.0f,   0.0f,  0.0f, -1.0f,     1.0f, 1.0f,
        //TEDDIE - Vertex 10 
        -0.45f, 0.0f, -0.2f,   0.0f,  0.0f, -1.0f,     0.0f, 1.0f,

        //TEDDIE - Triangle 10

        //TEDDIE - Vertex 0 
        0.0f, 1.0f, 0.0f,   0.0f,  0.0f,  1.0f,     0.5f, 1.0f,
        //TEDDIE - Vertex 8 
        -0.45f, 0.0f, 0.2f,   0.0f,  0.0f,  1.0f,     0.0f, 1.0f,
        //TEDDIE - Vertex 9
        -0.5f, 0.0f, 0.0f,   0.0f,  0.0f,  1.0f,     1.0f, 1.0f,

        //TEDDIE - Triangle 11

        //TEDDIE - Vertex 0 
        0.0f, 1.0f, 0.0f,   -1.0f,  0.0f,  0.0f,    0.5f, 1.0f,
        //TEDDIE - Vertex 7
        -0.35f, 0.0f, 0.35f,  -1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
        //TEDDIE - Vertex 8
        -0.45f, 0.0f, 0.2f,  -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,

        //TEDDIE - Triangle 12

        //TEDDIE - Vertex 0
        0.0f, 1.0f, 0.0f,   1.0f,  0.0f,  0.0f,     0.5f, 1.0f,
        //TEDDIE - Vertex 6
        -0.2f, 0.0f, 0.45f,   1.0f,  0.0f,  0.0f,     0.0f, 1.0f,
        //TEDDIE - Vertex 7
        -0.35f, 0.0f, 0.35f,  1.0f,  0.0f,  0.0f,     1.0f, 1.0f,

        //TEDDIE - Triangle 13

        //TEDDIE - Vertex 0
        0.0f, 1.0f, 0.0f,   0.0f, -1.0f,  0.0f,     0.5f, 1.0f,
        //TEDDIE - Vertex 5
        0.0f, 0.0f, 0.5f,   0.0f, -1.0f,  0.0f,     1.0f, 1.0f,
        //TEDDIE - Vertex 6
        -0.2f, 0.0f, 0.45f,  0.0f, -1.0f,  0.0f,     0.0f, 1.0f,

        //TEDDIE - Triangle 14

        //TEDDIE - Vertex 0
        0.0f, 1.0f, 0.0f,   0.0f,  1.0f,  0.0f,     0.5f, 1.0f,
        //TEDDIE - Vertex 4
        0.2f, 0.0f, 0.45f,  0.0f,  1.0f,  0.0f,     0.0f, 1.0f,
        //TEDDIE - Vertex 5
        0.0f, 0.0f, 0.5f,   0.0f,  1.0f,  0.0f,     1.0f, 1.0f,

        //TEDDIE - Triangle 15

        //TEDDIE - Vertex 0 
        0.0f, 1.0f, 0.0f,  0.0f,  0.0f, -1.0f,     0.5f, 1.0f,
        //TEDDIE - Vertex 3
        0.35f, 0.0f, 0.35f,   0.0f,  0.0f, -1.0f,     1.0f, 1.0f,
        //TEDDIE - Vertex 4
        0.2f, 0.0f, 0.45f, 0.0f,  0.0f, -1.0f,     0.0f, 1.0f,

        //TEDDIE - Triangle 16

        //TEDDIE - Vertex 0 
        0.0f, 1.0f, 0.0f,   0.0f,  0.0f,  1.0f,     0.5f, 1.0f,
        //TEDDIE - Vertex 2
        0.45f, 0.0f, 0.2f, 0.0f,  0.0f,  1.0f,     0.0f, 1.0f,
        //TEDDIE - Vertex 3
        0.35f, 0.0f, 0.35f,   0.0f,  0.0f,  1.0f,     1.0f, 1.0f,
    };


    // Position and Color data
    GLfloat pyramidVerts[] = {
        //TEDDIE - Triangle 1

        //TEDDIE - Vertex 0 
        0.0f, 0.5f, 0.0f,   0.0f,  0.0f, -1.0f,     1.0f, 0.0f,
        //TEDDIE - Vertex 1
        0.5f, 0.0f, 0.0f,   0.0f,  0.0f, -1.0f,     1.0f, 1.0f,
        //TEDDIE - Vertex 2 
        0.0f, 0.0f, 0.5f,   0.0f,  0.0f, -1.0f,     0.0f, 1.0f,

        //TEDDIE - Triangle 2

        //TEDDIE - Vertex 0 
        0.0f, 0.5f, 0.0f,   0.0f,  0.0f,  1.0f,     1.0f, 0.0f,
        //TEDDIE - Vertex 4 
        .0f, 0.0f, -0.5f,   0.0f,  0.0f,  1.0f,     0.0f, 1.0f,
        //TEDDIE - Vertex 1
        0.5f, 0.0f, 0.0f,   0.0f,  0.0f,  1.0f,     1.0f, 1.0f,

        //TEDDIE - Triangle 3

        //TEDDIE - Vertex 0 
        0.0f, 0.5f, 0.0f,   -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
        //TEDDIE - Vertex 3 
        -0.5f, 0.0f, 0.0f,  -1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
        //TEDDIE - Vertex 4 
        0.0f, 0.0f, -0.5f,  -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,

        //TEDDIE - Triangle 4

        //TEDDIE - Vertex 0
        0.0f, 0.5f, 0.0f,   1.0f,  0.0f,  0.0f,     1.0f, 0.0f,
        //TEDDIE - Vertex 2 
        0.0f, 0.0f, 0.5f,   1.0f,  0.0f,  0.0f,     0.0f, 1.0f,
        //TEDDIE - Vertex 3 
        -0.5f, 0.0f, 0.0f,  1.0f,  0.0f,  0.0f,     1.0f, 1.0f,

        //TEDDIE - Triangle 5

        //TEDDIE - Vertex 4
        .0f, 0.0f, -0.5f,   0.0f, -1.0f,  0.0f,     0.0f, 1.0f,
        //TEDDIE - Vertex 1
        0.5f, 0.0f, 0.0f,   0.0f, -1.0f,  0.0f,     1.0f, 1.0f,
        //TEDDIE - Vertex 3 
        -0.5f, 0.0f, 0.0f,  0.0f, -1.0f,  0.0f,     1.0f, 1.0f,

        //TEDDIE - Triangle 6

        //TEDDIE - Vertex 2
        0.0f, 0.0f, 0.5f,   0.0f,  1.0f,  0.0f,     0.0f, 1.0f,
        //TEDDIE - Vertex 3 
        -0.5f, 0.0f, 0.0f,  0.0f,  1.0f,  0.0f,     1.0f, 1.0f,
        //TEDDIE - Vertex 1
        0.5f, 0.0f, 0.0f,   0.0f,  1.0f,  0.0f,     1.0f, 1.0f
    };
    GLfloat domeVerts[] = {

       -0.3f, -0.0f, -0.3f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.3f, -0.0f, -0.3f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.7f,  0.5f, 0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        0.7f,  0.5f, 0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
       -0.7f,  0.5f, 0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
       -0.3f, -0.0f, -0.3f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,


      -0.3f, -0.0f,  0.3f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
       0.3f, -0.0f,  0.3f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
       0.7f,  0.5f, 0.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
       0.7f,  0.5f, 0.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
      -0.7f,  0.5f, 0.0f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
      -0.3f, -0.0f,  0.3f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,


     -0.7f,  0.5f, 0.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     -0.7f,  0.5f, 0.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     -0.3f, -0.0f, -0.3f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.3f, -0.0f, -0.3f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.3f, -0.0f,  0.3f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     -0.7f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,


     0.7f,  0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.7f,  0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.3f, -0.0f, -0.3f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.3f, -0.0f, -0.3f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.3f, -0.0f,  0.3f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.7f,  0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    -0.3f, -0.0f, -0.3f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.3f, -0.0f, -0.3f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.3f, -0.0f,  0.3f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.3f, -0.0f,  0.3f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.3f, -0.0f,  0.3f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.3f, -0.0f, -0.3f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,


   -0.7f,  0.5f, 0.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    0.7f,  0.5f, 0.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
   0.7f,  0.5f, 0.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    0.7f,  0.5f, 0.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
   -0.7f,  0.5f, 0.0f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
   -0.7f,  0.5f, 0.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };



    GLfloat torusVerts[] = {

       -0.3f, -0.0f, -0.3f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.3f, -0.0f, -0.3f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
       -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
       -0.3f, -0.0f, -0.3f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

  
      -0.3f, -0.0f,  0.3f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
       0.3f, -0.0f,  0.3f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
       0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
      -0.3f, -0.0f,  0.3f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

  
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     -0.3f, -0.0f, -0.3f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.3f, -0.0f, -0.3f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.3f, -0.0f,  0.3f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,


     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.3f, -0.0f, -0.3f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.3f, -0.0f, -0.3f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.3f, -0.0f,  0.3f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

  
    -0.3f, -0.0f, -0.3f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.3f, -0.0f, -0.3f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.3f, -0.0f,  0.3f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.3f, -0.0f,  0.3f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.3f, -0.0f,  0.3f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.3f, -0.0f, -0.3f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };
   
    // Position and Color data
    GLfloat limeVerts[] = {



     -0.7f,  0.5f, 0.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     -0.7f,  0.5f, 0.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     -0.3f, -0.0f, -0.3f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.3f, -0.0f, -0.3f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     -0.3f, -0.0f,  0.3f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     -0.7f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     0.7f,  0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.7f,  0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.3f, -0.0f, -0.3f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.3f, -0.0f, -0.3f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.3f, -0.0f,  0.3f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.7f,  0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,


    -0.3f, -0.0f, -0.3f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.3f, -0.0f, -0.3f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.3f, -0.0f,  0.3f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.3f, -0.0f,  0.3f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.3f, -0.0f,  0.3f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.3f, -0.0f, -0.3f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
    };


    

    mesh.nVertices[0] = sizeof(coneVerts) / (sizeof(coneVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    mesh.nVertices[1] = sizeof(coneVerts) / (sizeof(coneVerts[1]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    mesh.nVertices[2] = sizeof(planeVerts) / (sizeof(planeVerts[2]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    mesh.nVertices[11] = sizeof(pyramidVerts) / (sizeof(pyramidVerts[11]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    mesh.nVertices[12] = sizeof(cubeVerts) / (sizeof(cubeVerts[12]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
    mesh.nVertices[13] = sizeof(domeVerts) / (sizeof(domeVerts[13]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));
   mesh.nVertices[14] = sizeof(limeVerts) / (sizeof(limeVerts[14]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    //TEDDIE - Spike 1
    glGenVertexArrays(1, &mesh.vao[0]);
    glGenBuffers(1, &mesh.vbo[0]);
    glBindVertexArray(mesh.vao[0]);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(coneVerts), coneVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //TEDDIE - Spike 2
    glGenVertexArrays(1, &mesh.vao[1]);
    glGenBuffers(1, &mesh.vbo[1]);
    glBindVertexArray(mesh.vao[1]);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[1]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(coneVerts), coneVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //TEDDIE - SHELF
    glGenVertexArrays(1, &mesh.vao[2]);
    glGenBuffers(1, &mesh.vbo[2]);
    glBindVertexArray(mesh.vao[2]);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[2]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVerts), planeVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //TEDDIE - Coaster - Cork
    glGenVertexArrays(1, &mesh.vao[3]);
    glGenBuffers(1, &mesh.vbo[3]);
    glBindVertexArray(mesh.vao[3]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[3]);

    glGenVertexArrays(1, &mesh.vao[4]);
    glGenBuffers(1, &mesh.vbo[4]);
    glBindVertexArray(mesh.vao[4]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[4]);

    glGenVertexArrays(1, &mesh.vao[5]);
    glGenBuffers(1, &mesh.vbo[5]);
    glBindVertexArray(mesh.vao[5]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[5]);

    glGenVertexArrays(1, &mesh.vao[6]);
    glGenBuffers(1, &mesh.vbo[6]);
    glBindVertexArray(mesh.vao[6]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[6]);

    glGenVertexArrays(1, &mesh.vao[7]);
    glGenBuffers(1, &mesh.vbo[7]);
    glBindVertexArray(mesh.vao[7]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[7]);

    //TEDDIE - Coaster - Ceramic
    glGenVertexArrays(1, &mesh.vao[8]);
    glGenBuffers(1, &mesh.vbo[8]);
    glBindVertexArray(mesh.vao[8]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[8]);

    glGenVertexArrays(1, &mesh.vao[9]);
    glGenBuffers(1, &mesh.vbo[9]);
    glBindVertexArray(mesh.vao[9]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[9]);

    glGenVertexArrays(1, &mesh.vao[10]);
    glGenBuffers(1, &mesh.vbo[10]);
    glBindVertexArray(mesh.vao[10]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[10]);


    //TEDDIE - WASP NEST
    glGenVertexArrays(1, &mesh.vao[11]);
    glGenBuffers(1, &mesh.vbo[11]);
    glBindVertexArray(mesh.vao[11]);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[11]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVerts), pyramidVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

                                                                                   // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //TEDDIE - CUBEEEEEE
    glGenVertexArrays(1, &mesh.vao[12]);
    glGenBuffers(1, &mesh.vbo[12]);
    glBindVertexArray(mesh.vao[12]);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[12]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

                                                                                 // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //TEDDIE - DOME
    glGenVertexArrays(1, &mesh.vao[13]); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao[13]);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo[13]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[13]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(domeVerts), domeVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //TEDDIE - DOME
    glGenVertexArrays(1, &mesh.vao[14]); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao[14]);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo[14]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[14]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(domeVerts), domeVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //TEDDIE - WASP NEST
    glGenVertexArrays(1, &mesh.vao[11]);
    glGenBuffers(1, &mesh.vbo[11]);
    glBindVertexArray(mesh.vao[11]);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[11]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVerts), pyramidVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

                                                                                   // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    Sphere sphere;
    // copy interleaved vertex data (V/N/T) to VBO
    GLuint vboId;
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);           // for vertex data
    glBufferData(GL_ARRAY_BUFFER,                   // target
        sphere.getInterleavedVertexSize(), // data size, # of bytes
        sphere.getInterleavedVertices(),   // ptr to vertex data
        GL_STATIC_DRAW);                   // usage

// copy index data to VBO
    GLuint iboId;
    glGenBuffers(1, &iboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);   // for index data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,           // target
        sphere.getIndexSize(),             // data size, # of bytes
        sphere.getIndices(),               // ptr to index data
        GL_STATIC_DRAW);                   // usage


        // bind VBOs
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

    // activate attrib arrays
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // set attrib arrays with stride and offset
    int strideSphere = sphere.getInterleavedStride();     // should be 32 bytes
    glVertexAttribPointer(0, 3, GL_FLOAT, false, strideSphere, (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, strideSphere, (void*)(sizeof(float) * 3));
    glVertexAttribPointer(2, 2, GL_FLOAT, false, strideSphere, (void*)(sizeof(float) * 6));

    // draw a sphere with VBO
    glDrawElements(GL_TRIANGLES,                    // primitive type
        sphere.getIndexCount(),          // # of indices
        GL_UNSIGNED_INT,                 // data type
        (void*)0);                       // offset to indices

// deactivate attrib arrays
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    // unbind VBOs
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(20, mesh.vao);
    glDeleteBuffers(20, mesh.vbo);
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //TEDDIE - was getting an access violation error due to channel issue - specifically for waspy.jpg
        //TEDDIE - solution source: https://stackoverflow.com/questions/9950546/c-opengl-glteximage2d-access-violation
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}