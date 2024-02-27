#include <glad/glad.h>
#include <GLFW/glfw3.h>

// For textures
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// GLM Math header
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Include the shader header
#include <shader.h>
// Include the camera header
#include <camera.h>
#include <iostream>
#include <vector>

/*
* Christian Tavares || CS 330 Computer Graphic and Visualization || 2/22/2024
* 
* This is the final project of the course. We were instructed to create a 3D scene that has
* at least 4 3D objects within it. I opted to make a Rubik's cube atop a table. This scene
* consists of the Rubik's cube, the table top, 4 table legs, and a plane underneath it all
* with a 2 hidden lamp objects to illuminate the scene. Several textures will be used:
* 
* Fur texture sourced from Freepik
* Wood Texture sources from Freepik
* Glass Texture sourced from Freepik by RawPixel
* Black Texture sourced from Freepik by RawPixel
* Rubik's Cube faces were hand created by my in GIMP 2.0
*/

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "2D Scene Recreation"; // Macro for window title

    // Variables for window width and height
    const int SCR_WIDTH = 800;
    const int SCR_HEIGHT = 600;

    // For view toggling
    bool isPerspective = true; // Defines starting view
    glm::mat4 projection; // Initiates projection

    // camera
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
    float lastX = SCR_WIDTH / 2.0f;
    float lastY = SCR_HEIGHT / 2.0f;
    bool firstMouse = true;

    // timing
    float deltaTime = 0.0f;	// time between current frame and last frame
    float lastFrame = 0.0f;

    float PI = glm::radians(180.0f);

    // Stores the data relative to a given mesh
    struct GLMesh
    {
        unsigned int VAOs[15];       // Vertex array objects
        unsigned int lightCubeVAO;
        unsigned int VBOs[15];       // Vertex buffer objects
        unsigned int lightCubeVBO;
        unsigned int EBOs[15];       // Element buffer objects
        unsigned int indexCounts[11]; // Index counts

    };


    // Stores RGB values for specific colors
    struct color
    {
        float redValue;
        float greenValue;
        float blueValue;
        float alphaValue;
    };

    // Sets number of verices for some objects
    int sides = 20;
    int coneSides = 20;
    // Sets the rings and segments of sphere objects
    int segments = 20;
    int rings = 20;

    // Use to determine if color should be used
    color noColor;

    // Base color of the cat
    color catColor;

    // Sets the color of the glass
    color glassColor;

    // Textures
    unsigned int texture1;
    unsigned int texture2;
    unsigned int texture3;
    unsigned int texture4;
    unsigned int texture5;
    unsigned int texture6;
    unsigned int texture7;
    unsigned int texture8;

    // Mesh data
    GLMesh mesh;

    // Main window
    GLFWwindow* window = nullptr;

    // lighting
    glm::vec3 lightPos1(-3.0f, 1.5f, -3.0f);
    glm::vec3 lightPos2(4.0f, 0.5f, 0.0f);
}

// Function to adjust window size
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// Function for mouse callbacks
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
// Function for scroll callbacks
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
// Function to process input
void processInput(GLFWwindow* window);
// Function for generating cylinder side vertices
std::vector<float> genCylSideVerts(int sides, float height, float radius, color color);
// Function for generating cylinder top vertices
std::vector<float> genCylTopVerts(int sides, float height, float radius, color color);
// Funciton to generate cylinder top indices
std::vector<unsigned int> genCylTopIndices(int sides);
// Function for generating cylinder bottom vertices
std::vector<float> genCylBottomVerts(int sides, float height, float radius, color color);
// Funciton to generate cylinder bottom indices
std::vector<unsigned int> genCylBottomIndices(int sides);
// Function to generate sphere vertices
std::vector<float> genSphereVerts(float radius, color color);
// Function to generate sphere indices
std::vector<unsigned int> genSphereIndices();
// Function to generate a pyramids vertices
std::vector<float> genPyramidVerts(int sides, float height, float radius, color color);
// Function to generate a planes vertices
std::vector<float> genPlaneVerts(int sections, color color);
// Function to create textures
void createTextures();

// Function for toggling view between orthographic and perspective 
void toggleView();
// Function to initialize program
bool progInitialize(GLFWwindow** window);
// Function to create the mesh
void createMesh(GLMesh& mesh);


int main()
{
    if (!progInitialize(&window))
        return EXIT_FAILURE;

    createMesh(mesh);

    createTextures();

    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("shader.vs", "shader.fs");
    Shader lightCubeShader("light_cube.vs", "light_cube.fs");

    ourShader.use();
    ourShader.setInt("material.diffuse1", 0);
    ourShader.setInt("material.diffuse2", 1);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {

        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // Clears frame and sets background color
        //(0.698f, 0.863f, 1.0f, 1.0f); Original background color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        // Activate Shader
        ourShader.use();

        ourShader.setVec3("viewPos", camera.Position);

        // material properties
        ourShader.setVec3("material.specular", 0.2f, 0.2f, 0.2f);
        ourShader.setFloat("material.shininess", 30.0f);


        // directional light
        ourShader.setVec3("dirLight.direction", -0.5f, -1.0f, 0.0f);
        ourShader.setVec3("dirLight.ambient", 0.4f, 0.4f, 0.4f);
        ourShader.setVec3("dirLight.diffuse", 0.6f, 0.6f, 0.6f);
        ourShader.setVec3("dirLight.specular", 0.3f, 0.3f, 0.3f);

        // point light 1
        ourShader.setVec3("pointLights[0].position", lightPos1);
        ourShader.setVec3("pointLights[0].ambient", 0.1f, 0.1f, 0.1f);
        ourShader.setVec3("pointLights[0].diffuse", 0.5f, 0.5f, 0.5f);
        ourShader.setVec3("pointLights[0].specular", 0.6f, 0.6f, 0.6f);
        ourShader.setFloat("pointLights[0].constant", 1.0f);
        ourShader.setFloat("pointLights[0].linear", 0.007f);
        ourShader.setFloat("pointLights[0].quadratic", 0.0002f);
        ourShader.setVec3("pointLights[0].lightColor", 1.0f, 1.0f, 1.0f);
        // point light 2
        ourShader.setVec3("pointLights[1].position", lightPos2);
        ourShader.setVec3("pointLights[1].ambient", 0.1f, 0.1f, 0.1f);
        ourShader.setVec3("pointLights[1].diffuse", 0.3f, 0.3f, 0.3f);
        ourShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[1].constant", 1.0f);
        ourShader.setFloat("pointLights[1].linear", 0.007f);
        ourShader.setFloat("pointLights[1].quadratic", 0.0002f);
        ourShader.setVec3("pointLights[1].lightColor", 1.0f, 1.0f, 1.0f);

        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            // Print or log the error code for debugging
            std::cout << "OpenGL error: " << error << std::endl;
        }

        // initialize model for transformations
        glm::mat4 model = glm::mat4(1.0f);
       
        /*
        Lines 263-326 are not really used in the program in any significant way. I kept it in from the original
        simply because the code didn't function right without it in. It's not breaking anything, so no need to fix it.
        */
        
        // Transforms the first object (Lower Cylinder)
        // Move to the left
        glm::mat4 translation = glm::translate(glm::vec3(-1.5f, -0.625, 0.0f)); // -0.625 places it ontop of the plane
        // Rotations
        glm::mat4 rotation = glm::rotate(glm::radians(30.0f), glm::vec3(0.f, 1.0f, 0.0f));
        // Sets the model
        model = translation * rotation;
        ourShader.setMat4("model", model);

        // Sets the projection (view type)
        ourShader.setMat4("projection", projection); // Changes when P is pressed

        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);
        
        // Bind textures For the first cylinder
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture3);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture7);
        glUniform1i(glGetUniformLocation(ourShader.ID, "numTextures"), 2);
        
        // First cylinder sides
        glBindVertexArray(mesh.VAOs[0]);
        // Draw the sides of the first cylinder
        //glDrawElements(GL_TRIANGLES, mesh.indexCounts[0], GL_UNSIGNED_INT, 0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, mesh.indexCounts[0]);

        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1i(glGetUniformLocation(ourShader.ID, "numTextures"), 1);

        // First cylinder top
        glBindVertexArray(mesh.VAOs[1]);
        // Draw the top of the first cylinder
        glDrawElements(GL_TRIANGLES, mesh.indexCounts[1], GL_UNSIGNED_INT, 0);

        // First cylinder bottom
        glBindVertexArray(mesh.VAOs[2]);
        // Draw the bottom of the first cylinder
        glDrawElements(GL_TRIANGLES, mesh.indexCounts[2], GL_UNSIGNED_INT, 0);

        // Unbind second texture
        glBindTexture(GL_TEXTURE_2D, 0);

        // Transforms the second object (Upper Cylinder)
        // Move to the left and up (to sit ontop of the other cylinder)
        translation = glm::translate(glm::vec3(-1.5f, -0.25, 0.0f)); // -0.23 places it ontop of the other cylinder
        // Rotates the cylinder
        rotation = glm::rotate(glm::radians(30.0f), glm::vec3(0.f, 1.0f, 0.0f));
        // Sets the model
        model = translation * rotation;

        ourShader.setMat4("model", model);

        // Second cylinder
        glBindVertexArray(mesh.VAOs[3]);

        // Bind texture for upper cylinder

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glUniform1i(glGetUniformLocation(ourShader.ID, "numTextures"), 1);

        // Reset
        ourShader.setVec3("material.specular", 0.0f, 0.0f, 0.0f);
        ourShader.setFloat("material.shininess", 1.0f);

        // Draw the second cylinder sides
        //glDrawElements(GL_TRIANGLES, mesh.indexCounts[3], GL_UNSIGNED_INT, 0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, mesh.indexCounts[3]);

        // second cylinder top
        glBindVertexArray(mesh.VAOs[4]);
        // Draw the top of the first cylinder
        glDrawElements(GL_TRIANGLES, mesh.indexCounts[4], GL_UNSIGNED_INT, 0);

        // second cylinder bottom
        glBindVertexArray(mesh.VAOs[5]);
        // Draw the bottom of the first cylinder
        glDrawElements(GL_TRIANGLES, mesh.indexCounts[5], GL_UNSIGNED_INT, 0); 
        
        /*
        Everything above this point is left in from the original code just to ensure proper function.
        The ONLY objects drawn are the rubik's cube, 1 light object, the table top and 4 table legs.
        */

        // Bind textures for face 1 of cube
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        // Transforms the first object Rubik's Cube
        // Face 1
        translation = glm::translate(glm::vec3(0.5f, 0.45f, 0.1f)); // places it on top of the table
        // Rotate the object slightly
        rotation = glm::rotate(glm::radians(-5.0f), glm::vec3(0.f, 1.0f, 0.0f));
        // Sets the model
        model = translation * rotation;

        ourShader.setMat4("model", model);

        // Make cube shiny
        ourShader.setVec3("material.specular", 0.8f, 0.8f, 0.8f);
        ourShader.setFloat("material.shininess", 70.0f);

        // Next face (Cube)
        glBindVertexArray(mesh.VAOs[0]);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Bind textures for face 2 of cube
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // Face 2
        translation = glm::translate(glm::vec3(0.5f, 0.45f, 0.1f)); // places it ontop of the table
        // Rotate the object slightly
        rotation = glm::rotate(glm::radians(-5.0f), glm::vec3(0.f, 1.0f, 0.0f));
        // Sets the model
        model = translation * rotation;

        ourShader.setMat4("model", model);

        // Make cube shiny
        ourShader.setVec3("material.specular", 0.8f, 0.8f, 0.8f);
        ourShader.setFloat("material.shininess", 70.0f);

        // Next face (Cube)
        glBindVertexArray(mesh.VAOs[1]);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Bind textures for face 3 of cube
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, texture4);

        // Face 3
        translation = glm::translate(glm::vec3(0.5f, 0.45f, 0.1f)); // places it ontop of the table
        // Rotate the object slightly
        rotation = glm::rotate(glm::radians(-5.0f), glm::vec3(0.f, 1.0f, 0.0f));
        // Sets the model
        model = translation * rotation;

        ourShader.setMat4("model", model);

        // Make cube shiny
        ourShader.setVec3("material.specular", 0.8f, 0.5f, 0.8f);
        ourShader.setFloat("material.shininess", 70.0f);

        // Next face (Cube)
        glBindVertexArray(mesh.VAOs[2]);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Bind textures for face 4 of cube
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, texture3);

        // Face 4
        translation = glm::translate(glm::vec3(0.5f, 0.45f, 0.1f)); // places it ontop of the table
        // Rotate the object slightly
        rotation = glm::rotate(glm::radians(-5.0f), glm::vec3(0.f, 1.0f, 0.0f));
        // Sets the model
        model = translation * rotation;

        ourShader.setMat4("model", model);

        // Make cube shiny
        ourShader.setVec3("material.specular", 0.8f, 0.8f, 0.8f);
        ourShader.setFloat("material.shininess", 70.0f);

        // Next face (Cube)
        glBindVertexArray(mesh.VAOs[3]);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Bind textures for face 5 of cube
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, texture6);

        // Face 5
        translation = glm::translate(glm::vec3(0.5f, 0.45f, 0.1f)); // places it ontop of the table
        // Rotate the object slightly
        rotation = glm::rotate(glm::radians(-5.0f), glm::vec3(0.f, 1.0f, 0.0f));
        // Sets the model
        model = translation * rotation;

        ourShader.setMat4("model", model);

        // Make cube shiny
        ourShader.setVec3("material.specular", 0.8f, 0.8f, 0.8f);
        ourShader.setFloat("material.shininess", 70.0f);

        // Next face (Cube)
        glBindVertexArray(mesh.VAOs[4]);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Bind textures for face 6 of cube
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, texture5);

        // Face 6
        translation = glm::translate(glm::vec3(0.5f, 0.45f, 0.1f)); // places it ontop of the table
        // Rotate the object slightly
        rotation = glm::rotate(glm::radians(-5.0f), glm::vec3(0.f, 1.0f, 0.0f));
        // Sets the model
        model = translation * rotation;

        ourShader.setMat4("model", model);

        // Make cube shiny
        ourShader.setVec3("material.specular", 0.8f, 0.8f, 0.8f);
        ourShader.setFloat("material.shininess", 70.0f);

        // Next object (Table)
        glBindVertexArray(mesh.VAOs[5]);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Bind textures for table top

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, texture7);

        // Table
        translation = glm::translate(glm::vec3(0.5f, 0.125, 0.1f)); // places tabletop
        // Zero rotation
        rotation = glm::rotate(glm::radians(0.0f), glm::vec3(0.f, 1.0f, 0.0f));
        // Sets the model
        model = translation * rotation;

        ourShader.setMat4("model", model);

        // Make table shiny
        ourShader.setVec3("material.specular", 0.8f, 0.8f, 0.8f);
        ourShader.setFloat("material.shininess", 70.0f);

        // Next object (Table)
        glBindVertexArray(mesh.VAOs[6]);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Bind textures for Table Leg 1
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, texture7);

        // Table Leg 1
        // Move to the right
        translation = glm::translate(glm::vec3(-1.4f, -0.5, 1.0f)); // places it under table (front left)
        // Zero rotation
        rotation = glm::rotate(glm::radians(0.0f), glm::vec3(0.f, 1.0f, 0.0f));
        // Sets the model
        model = translation * rotation;

        ourShader.setMat4("model", model);

        // Make cube shiny
        ourShader.setVec3("material.specular", 0.8f, 0.8f, 0.8f);
        ourShader.setFloat("material.shininess", 70.0f);

        // Next object (Table)
        glBindVertexArray(mesh.VAOs[8]);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Bind textures for Table Leg 2
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, texture7);

        // Table Leg 2
        // Move to the right
        translation = glm::translate(glm::vec3(2.4f, -0.5, 1.0f)); // places it under table (front right)
        // Zero rotation
        rotation = glm::rotate(glm::radians(0.0f), glm::vec3(0.f, 1.0f, 0.0f));
        // Sets the model
        model = translation * rotation;

        ourShader.setMat4("model", model);

        // Make cube shiny
        ourShader.setVec3("material.specular", 0.8f, 0.8f, 0.8f);
        ourShader.setFloat("material.shininess", 70.0f);

        // Next object (Table)
        glBindVertexArray(mesh.VAOs[9]);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Bind textures for Table Leg 3
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, texture7);

        // Table Leg 3
        translation = glm::translate(glm::vec3(-1.4f, -0.5, -0.8f)); // places it under table (back left)
        // Zero rotation
        rotation = glm::rotate(glm::radians(0.0f), glm::vec3(0.f, 1.0f, 0.0f));
        // Sets the model
        model = translation * rotation;

        ourShader.setMat4("model", model);

        // Make cube shiny
        ourShader.setVec3("material.specular", 0.8f, 0.8f, 0.8f);
        ourShader.setFloat("material.shininess", 70.0f);

        // Next object (Table)
        glBindVertexArray(mesh.VAOs[10]);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Bind textures for Table Leg 4
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, texture7);

        // Table Leg 4
        translation = glm::translate(glm::vec3(2.4f, -0.5, -0.8f)); // places it under table (back right)
        // Zero rotation
        rotation = glm::rotate(glm::radians(0.0f), glm::vec3(0.f, 1.0f, 0.0f));
        // Sets the model
        model = translation * rotation;

        ourShader.setMat4("model", model);

        // Make cube shiny
        ourShader.setVec3("material.specular", 0.8f, 0.8f, 0.8f);
        ourShader.setFloat("material.shininess", 70.0f);

        // Next object (Plane)
        glBindVertexArray(mesh.VAOs[11]);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Bind textures for Plane
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, texture8);

        // Scales the object
        glm::mat4 scale = glm::scale(glm::vec3(5.0f, 1.0f, 5.0f));
        // Move down
        translation = glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)); 
        // Sets the model
        model = translation * scale;

        ourShader.setMat4("model", model);

        // Adjust Specular/Shininess
        ourShader.setVec3("material.specular", 0.6f, 0.6f, 0.6f);
        ourShader.setFloat("material.shininess", 200.0f);

        // directional light
        ourShader.setVec3("dirLight.direction", -0.5f, -1.0f, 0.0f);
        ourShader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
        ourShader.setVec3("dirLight.diffuse", 0.25f, 0.25f, 0.25f);
        ourShader.setVec3("dirLight.specular", 0.01f, 0.01f, 0.01f);

        // Fourth Object (Plane)
        glBindVertexArray(mesh.VAOs[7]);

        glDrawArrays(GL_TRIANGLES, 0, mesh.indexCounts[7]);

        // directional light
        ourShader.setVec3("dirLight.direction", -0.5f, -1.0f, 0.0f);
        ourShader.setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
        ourShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        ourShader.setVec3("dirLight.specular", 0.3f, 0.3f, 0.3f);

        float xScale = 0.625f / 0.5625f;
        float zScale = 0.125f / 0.5625f;

        /*
        // Bind textures sphere 1
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, texture8);

        // Reset
        ourShader.setVec3("material.specular", 0.0f, 0.0f, 0.0f);
        ourShader.setFloat("material.shininess", 1.0f);
        
        // Transforms the fifth object (sphere 1)
        // Scales the object
        scale = glm::scale(glm::vec3(xScale, 1.0f, zScale));
        // Rotation
        rotation = glm::rotate(glm::radians(30.0f), glm::vec3(0.0f, 0.7f, 0.0f));
        glm::mat4 rotation2 = glm::rotate(glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // Moves the object
        translation = glm::translate(glm::vec3(-0.84f, -0.338f, -1.375f));
        // Sets the model
        model = translation * rotation2 * rotation * scale;

        ourShader.setMat4("model", model);

        // Fifth Object (Sphere 1)
        glBindVertexArray(mesh.VAOs[8]);

        glDrawElements(GL_TRIANGLES, mesh.indexCounts[8], GL_UNSIGNED_INT, 0);

        // Bind textures for sphere 2
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture6); // Face Texture 
        glUniform1i(glGetUniformLocation(ourShader.ID, "numTextures"), 2); // Change to 2  for face

        zScale = 0.1875 / 0.5625;

        // Transforms the fifth object (Face) (gives sphere 2)
        // Scales the object
        scale = glm::scale(glm::vec3(xScale, 1.0f, zScale));
        // Rotation
        rotation = glm::rotate(glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        rotation2 = glm::rotate(glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // Moves object
        translation = glm::translate(glm::vec3(-0.219049f, 0.02f, -0.140525f));
        // Sets the model
        model = translation * rotation2 * rotation * scale;

        ourShader.setMat4("model", model);

        glDrawElements(GL_TRIANGLES, mesh.indexCounts[8], GL_UNSIGNED_INT, 0);

        // Bind textures for cat body (cylinder 3)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glUniform1i(glGetUniformLocation(ourShader.ID, "numTextures"), 1);

        // Transforms the sixth object (cylinder 3)
        // Scales the object
        scale = glm::scale(glm::vec3(xScale, 1.0f, 1.0f));
        // Rotates the object
        rotation = glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        rotation2 = glm::rotate(glm::radians(30.0f), glm::vec3(0.0f, 0.7f, 0.0f));
        glm::mat4 rotation3 = glm::rotate(glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // Moves object
        translation = glm::translate(glm::vec3(-0.53f, -0.16f, -0.76f)); 
        // Sets the model
        model = translation * rotation3 * rotation2 * rotation * scale;
        

        ourShader.setMat4("model", model);

        // Sixth Object (cyl 3)
        glBindVertexArray(mesh.VAOs[9]);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, mesh.indexCounts[9]);
        

        // Transforms the seventh object (cone 1)
        // Moves object
        translation = glm::translate(glm::vec3(-0.02f, 0.5f, -0.58f));
        // Rotation
        rotation2 = glm::rotate(glm::radians(-25.0f), glm::vec3(0.7f, 0.0f, 0.0f));
        // Sets the model
        model = translation * rotation2;

        ourShader.setMat4("model", model);

        // 8th Object (cone)
        glBindVertexArray(mesh.VAOs[10]);

        glDrawArrays(GL_TRIANGLES, 0, mesh.indexCounts[10] / 9);

        // Transforms the seventh object (gives cone 2)
        // Moves object
        translation = glm::translate(glm::vec3(-0.55f, 0.53f, -0.30f));
        // Rotations
        rotation = glm::rotate(glm::radians(5.0f), glm::vec3(0.0f, 0.0f, 0.1f));
        rotation2 = glm::rotate(glm::radians(-15.0f), glm::vec3(0.7f, 0.0f, 0.0f));
        // Sets the model
        model = translation * rotation2 * rotation;

        ourShader.setMat4("model", model);

        // seventh Object (cone)
        glBindVertexArray(mesh.VAOs[10]);

        glDrawArrays(GL_TRIANGLES, 0, mesh.indexCounts[10] / 9);
        */

        glBindVertexArray(mesh.lightCubeVAO);

        // also draw the lamp object
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos1);
        model = glm::scale(model, glm::vec3(0.0f)); // a smaller cube
        lightCubeShader.setMat4("model", model);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // also draw the lamp object
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos2);
        model = glm::scale(model, glm::vec3(0.0f)); // a smaller cube
        lightCubeShader.setMat4("model", model);

        glDrawArrays(GL_TRIANGLES, 0, 36);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(12, mesh.VAOs);
    glDeleteBuffers(12, mesh.VBOs);
    glDeleteBuffers(12, mesh.EBOs);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void createTextures() {
    // load textures
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // Find file
    unsigned char* data = stbi_load(("resources/Rubiks1.png"), &width, &height, &nrChannels, 0);
    if (data) {
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        glGenTextures(1, &texture1);
        glBindTexture(GL_TEXTURE_2D, texture1);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Check channels
        if (nrChannels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else if (nrChannels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Not implemented to handle image with " << nrChannels << " channels" << std::endl;
        }


        stbi_image_free(data);
    }
    // Find file
    data = stbi_load(("resources/Rubiks2.png"), &width, &height, &nrChannels, 0);
    if (data) {
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        glGenTextures(1, &texture2);
        glBindTexture(GL_TEXTURE_2D, texture2);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Check channels
        if (nrChannels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else if (nrChannels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Not implemented to handle image with " << nrChannels << " channels" << std::endl;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
    }
    // Find file
    data = stbi_load(("resources/Rubiks3.png"), &width, &height, &nrChannels, 0);
    if (data) {
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        glGenTextures(1, &texture3);
        glBindTexture(GL_TEXTURE_2D, texture3);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Check channels
        if (nrChannels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else if (nrChannels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Not implemented to handle image with " << nrChannels << " channels" << std::endl;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
    }
    // Find file
    data = stbi_load(("resources/Rubiks4.png"), &width, &height, &nrChannels, 0);
    if (data) {
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        glGenTextures(1, &texture4);
        glBindTexture(GL_TEXTURE_2D, texture4);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Check channels
        if (nrChannels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else if (nrChannels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Not implemented to handle image with " << nrChannels << " channels" << std::endl;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
    }
    // Find file
    data = stbi_load(("resources/Rubiks5.png"), &width, &height, &nrChannels, 0);
    if (data) {
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        glGenTextures(1, &texture5);
        glBindTexture(GL_TEXTURE_2D, texture5);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Check channels
        if (nrChannels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else if (nrChannels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Not implemented to handle image with " << nrChannels << " channels" << std::endl;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
    }
    // Find file
    data = stbi_load(("resources/Rubiks6.png"), &width, &height, &nrChannels, 0);
    if (data) {
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        glGenTextures(1, &texture6);
        glBindTexture(GL_TEXTURE_2D, texture6);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Check channels
        if (nrChannels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else if (nrChannels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Not implemented to handle image with " << nrChannels << " channels" << std::endl;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
    }
    // Find file
    data = stbi_load(("resources/WoodTexture.jpg"), &width, &height, &nrChannels, 0);
    if (data) {
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        glGenTextures(1, &texture7);
        glBindTexture(GL_TEXTURE_2D, texture7);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Check channels
        if (nrChannels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else if (nrChannels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Not implemented to handle image with " << nrChannels << " channels" << std::endl;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
    }
    // Find file
    data = stbi_load(("resources/Black Texture.jpg"), &width, &height, &nrChannels, 0);
    if (data) {
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        glGenTextures(1, &texture8);
        glBindTexture(GL_TEXTURE_2D, texture8);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Check channels
        if (nrChannels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else if (nrChannels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Not implemented to handle image with " << nrChannels << " channels" << std::endl;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
    }
}

// createMesh features additional vertices for the table and the table legs. Scroll lower to see more. 

// Function to create mesh
void createMesh(GLMesh &mesh) {

    noColor.redValue = 1.0f;
    noColor.greenValue = 1.0f;
    noColor.blueValue = 1.0f;
    noColor.alphaValue = 1.0f;

    glassColor.redValue = 0.51f;
    glassColor.greenValue = 0.298f;
    glassColor.blueValue = 0.812f;
    glassColor.alphaValue = 1.0f;

    // All size values are 1/4 of real life sizes in inches
    /*
    // Generates the first cylinder's side vetices and indices
    std::vector<float> cylSideVertices1 = genCylSideVerts(sides, 0.75, 0.59375f, glassColor);
    // Generates the first cylinder's top vetices and indices
    std::vector<float> cylTopVertices1 = genCylTopVerts(sides, 0.75, 0.59375f, glassColor);
    std::vector<unsigned int> cylTopIndices1 = genCylTopIndices(sides);
    // Generates the first cylinder's bottom vetices and indices
    std::vector<float> cylBottomVertices1 = genCylBottomVerts(sides, 0.75, 0.59375f, glassColor);
    std::vector<unsigned int> cylBottomIndices1 = genCylBottomIndices(sides);

    // Generates the second cylinder's vetices and indices
    std::vector<float> cylSideVertices2 = genCylSideVerts(sides, 0.03125f, 0.57f, noColor);
    // Generates the second cylinder's top vetices and indices
    std::vector<float> cylTopVertices2 = genCylTopVerts(sides, 0.03125f, 0.57f, noColor);
    std::vector<unsigned int> cylTopIndices2 = genCylTopIndices(sides);
    // Generates the second cylinder's bottom vetices and indices
    std::vector<float> cylBottomVertices2 = genCylBottomVerts(sides, 0.03125f, 0.57f, noColor);
    std::vector<unsigned int> cylBottomIndices2 = genCylBottomIndices(sides);*/

    // Sets the length, width, and height of the cube
    float length = 1.0f;
    float width = 1.0f;
    float height = 1.0f;

    // Define the vertices of the cube and their associated colors
    float cubeFace1Verts[] = { // Rubik's cube faces
        // Front Face Triangle 1
        // width          height           length         normals              color                        texture
       -width / 5,    -height / 5,     -length / 5,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 0
        width / 5,    -height / 5,     -length / 5,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 1
        width / 5,     height / 5,     -length / 5,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 2

        // Front Face Triangle 2
       -width / 5,    -height / 5,     -length / 5,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 0
        width / 5,     height / 5,     -length / 5,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 2
       -width / 5,     height / 5,     -length / 5,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 3
    };
    float cubeFace2Verts[] = {
        // width          height           length         normals              color                        texture
        // Left Face Triangle 1
       -width / 5,    -height / 5,     -length / 5,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 0
       -width / 5,    -height / 5,      length / 5,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 4
       -width / 5,     height / 5,      length / 5,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 7
       
        // Left Face Triangle 2
       -width / 5,    -height / 5,     -length / 5,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 0
       -width / 5,     height / 5,     -length / 5,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 3
       -width / 5,     height / 5,      length / 5,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 7
    };
    float cubeFace3Verts[] = {
        // Right Face Triangle 1
        width / 5,    -height / 5,     -length / 5,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 1
        width / 5,    -height / 5,      length / 5,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 5
        width / 5,     height / 5,      length / 5,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f,      // Vertex 6

        // Right Face Triangle 2
        width / 5,    -height / 5,     -length / 5,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 1
        width / 5,     height / 5,     -length / 5,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 2
        width / 5,     height / 5,      length / 5,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f,      // Vertex 6
    };
    float cubeFace4Verts[] = {
        // Back Face Triangle 1
       -width / 5,    -height / 5,      length / 5,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 4
        width / 5,    -height / 5,      length / 5,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 5
        width / 5,     height / 5,      length / 5,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 6

        // Back Face Triangle 2
       -width / 5,    -height / 5,      length / 5,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 4
        width / 5,     height / 5,      length / 5,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 6
       -width / 5,     height / 5,      length / 5,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 7
    };
    float cubeFace5Verts[] = {
        // Bottom Face Triangle 1
       -width / 5,    -height / 5,     -length / 5,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 0
        width / 5,    -height / 5,     -length / 5,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 1
       -width / 5,    -height / 5,      length / 5,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 4

       // Bottom Face Triangle 2
        width / 5,    -height / 5,     -length / 5,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 1
       -width / 5,    -height / 5,      length / 5,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 4
        width / 5,    -height / 5,      length / 5,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 5
    };
    float cubeFace6Verts[] = {
        // Top Face Triangle 1
        width / 5,     height / 5,     -length / 5,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 2
       -width / 5,     height / 5,     -length / 5,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 3
       -width / 5,     height / 5,      length / 5,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 7

       // Top Face Triangle 2
        width / 5,     height / 5,     -length / 5,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 2
        width / 5,     height / 5,      length / 5,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 6
       -width / 5,     height / 5,      length / 5,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 7
    };
    float tableVerts[] = { // table top
        // Front Face Triangle 1
       -width / 0.5,    -height / 8,     -length / 1,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 0
        width / 0.5,    -height / 8,     -length / 1,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 1
        width / 0.5,     height / 8,     -length / 1,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 2

        // Front Face Triangle 2
       -width / 0.5,    -height / 8,     -length / 1,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 0
        width / 0.5,     height / 8,     -length / 1,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 2
       -width / 0.5,     height / 8,     -length / 1,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 3

        // Top Face Triangle 1
        width / 0.5,     height / 8,     -length / 1,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 2
       -width / 0.5,     height / 8,     -length / 1,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 3
       -width / 0.5,     height / 8,      length / 1,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 7

       // Top Face Triangle 2
        width / 0.5,     height / 8,     -length / 1,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 2
        width / 0.5,     height / 8,      length / 1,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 6
       -width / 0.5,     height / 8,      length / 1,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 7

        // Left Face Triangle 1
       -width / 0.5,    -height / 8,     -length / 1,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 0
       -width / 0.5,    -height / 8,      length / 1,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 4
       -width / 0.5,     height / 8,      length / 1,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 7

       // Left Face Triangle 2
       -width / 0.5,    -height / 8,     -length / 1,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 0
       -width / 0.5,     height / 8,     -length / 1,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 3
       -width / 0.5,     height / 8,      length / 1,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 7

       // Right Face Triangle 1
        width / 0.5,    -height / 8,     -length / 1,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 1
        width / 0.5,    -height / 8,      length / 1,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 5
        width / 0.5,     height / 8,      length / 1,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f,      // Vertex 6

        // Right Face Triangle 2
        width / 0.5,    -height / 8,     -length / 1,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 1
        width / 0.5,     height / 8,     -length / 1,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 2
        width / 0.5,     height / 8,      length / 1,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f,      // Vertex 6

        // Back Face Triangle 1
       -width / 0.5,    -height / 8,      length / 1,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 4
        width / 0.5,    -height / 8,      length / 1,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 5
        width / 0.5,     height / 8,      length / 1,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 6

        // Back Face Triangle 2
       -width / 0.5,    -height / 8,      length / 1,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 4
        width / 0.5,     height / 8,      length / 1,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 6
       -width / 0.5,     height / 8,      length / 1,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 7

       // Bottom Face Triangle 1
       -width / 0.5,    -height / 8,     -length / 1,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 0
        width / 0.5,    -height / 8,     -length / 1,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 1
       -width / 0.5,    -height / 8,      length / 1,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 4

       // Bottom Face Triangle 2
        width / 0.5,    -height / 8,     -length / 1,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 1
       -width / 0.5,    -height / 8,      length / 1,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 4
        width / 0.5,    -height / 8,      length / 1,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 5
    };
    float tableLegVerts[] = { // table legs
        // Front Face Triangle 1
       -width / 10,    -height / 2,     -length / 10,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 0
        width / 10,    -height / 2,     -length / 10,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 1
        width / 10,     height / 2,     -length / 10,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 2

        // Front Face Triangle 2
       -width / 10,    -height / 2,     -length / 10,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 0
        width / 10,     height / 2,     -length / 10,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 2
       -width / 10,     height / 2,     -length / 10,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 3

        // Top Face Triangle 1
        width / 10,     height / 2,     -length / 10,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 2
       -width / 10,     height / 2,     -length / 10,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 3
       -width / 10,     height / 2,      length / 10,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 7

        // Top Face Triangle 2
        width / 10,     height / 2,     -length / 10,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 2
        width / 10,     height / 2,      length / 10,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 6
       -width / 10,     height / 2,      length / 10,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 7

        // Left Face Triangle 1
       -width / 10,    -height / 2,     -length / 10,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 0
       -width / 10,    -height / 2,      length / 10,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 4
       -width / 10,     height / 2,      length / 10,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 7

        // Left Face Triangle 2
       -width / 10,    -height / 2,     -length / 10,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 0
       -width / 10,     height / 2,     -length / 10,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 3
       -width / 10,     height / 2,      length / 10,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 7

        // Right Face Triangle 1
        width / 10,    -height / 2,     -length / 10,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 1
        width / 10,    -height / 2,      length / 10,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 5
        width / 10,     height / 2,      length / 10,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f,      // Vertex 6

        // Right Face Triangle 2
        width / 10,    -height / 2,     -length / 10,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 1
        width / 10,     height / 2,     -length / 10,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 2
        width / 10,     height / 2,      length / 10,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f,      // Vertex 6

        // Back Face Triangle 1
       -width / 10,    -height / 2,      length / 10,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 4
        width / 10,    -height / 2,      length / 10,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 5
        width / 10,     height / 2,      length / 10,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 6
        
        // Back Face Triangle 2
       -width / 10,    -height / 2,      length / 10,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 4
        width / 10,     height / 2,      length / 10,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 6
       -width / 10,     height / 2,      length / 10,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 7

         // Bottom Face Triangle 1
       -width / 10,    -height / 2,     -length / 10,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 0.0f,     // Vertex 0
        width / 10,    -height / 2,     -length / 10,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 1
       -width / 10,    -height / 2,      length / 10,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 4

         // Bottom Face Triangle 2
        width / 10,    -height / 2,     -length / 10,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 0.0f,     // Vertex 1
       -width / 10,    -height / 2,      length / 10,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     // Vertex 4
        width / 10,    -height / 2,      length / 10,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,      1.0f, 1.0f,     // Vertex 5
    };

    // Lines 1291-1307 were kept for consistency despite not being needed, initialize buffers changed to 12.

    // Generate the planes vertices
    std::vector<float>planeVerts1 = genPlaneVerts(10, noColor);

    catColor.redValue = 0.62f;
    catColor.greenValue = 0.929f;
    catColor.blueValue = 0.243f;
    catColor.alphaValue = 1.0f;

    // Shapes for the cat
    // Generates the first sphere's vertices and indices
    std::vector<float> sphereVertices1 = genSphereVerts(0.5625, catColor);
    std::vector<unsigned int> sphereIndices1 = genSphereIndices();

    // Generates the third cylinder's vertices and indices
    std::vector<float> cylVertices3 = genCylSideVerts(sides, 1.4375f, 0.5625f, catColor);

    // Generate the pyramid/cone vertices and indices
    std::vector<float> coneVerts1 = genPyramidVerts(coneSides, 0.5f, 0.25f, catColor);

    // Initialize buffers
    glGenVertexArrays(12, mesh.VAOs);
    glGenBuffers(12, mesh.VBOs);
    glGenBuffers(12, mesh.EBOs);

    /*
    * The below commented chunk was like this when obtained. Leaving it in just in case.
    * 
    // First Cylinder Sides
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[0]);

    // VBO of the first cylinder
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[0]);
    //glBufferData(GL_ARRAY_BUFFER, cylSideVertices1.size() * sizeof(float), cylSideVertices1.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);


    // First Cylinder Top
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[1]);

    // VBO of the top of the first cylinder
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[1]);
    //glBufferData(GL_ARRAY_BUFFER, cylTopVertices1.size() * sizeof(float), cylTopVertices1.data(), GL_STATIC_DRAW);

    // EBO of the first cylinder
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBOs[1]);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylTopIndices1.size() * sizeof(unsigned int), cylTopIndices1.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);


    // First Cylinder Bottom
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[2]);

    // VBO of the bottom of the first cylinder
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[2]);
    //glBufferData(GL_ARRAY_BUFFER, cylBottomVertices1.size() * sizeof(float), cylBottomVertices1.data(), GL_STATIC_DRAW);

    // EBO of the first cylinder
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBOs[2]);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylBottomIndices1.size() * sizeof(unsigned int), cylBottomIndices1.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);


    // Second Cylinder Sides
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[3]);

    // VBO of the sides of the second cylinder
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[3]);
    //glBufferData(GL_ARRAY_BUFFER, cylSideVertices2.size() * sizeof(float), cylSideVertices2.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);


    // Second Cylinder Top
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[4]);

    // VBO of the Top of the second cylinder
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[4]);
    //glBufferData(GL_ARRAY_BUFFER, cylTopVertices2.size() * sizeof(float), cylTopVertices2.data(), GL_STATIC_DRAW);

    // EBOs of second cylinder
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBOs[4]);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylTopIndices2.size() * sizeof(unsigned int), cylTopIndices2.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);


    // Second Cylinder Bottom
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[5]);

    // VBO of the bottom of the second cylinder
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[5]);
    //glBufferData(GL_ARRAY_BUFFER, cylBottomVertices2.size() * sizeof(float), cylBottomVertices2.data(), GL_STATIC_DRAW);

    // EBOs of second cylinder
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBOs[5]);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylBottomIndices2.size() * sizeof(unsigned int), cylBottomIndices2.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);
    */

    // First Cube (First Object)
    // Face 1
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[0]);

    // VBO of the cube
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeFace1Verts), cubeFace1Verts, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // For the lights
    glGenVertexArrays(1, &mesh.lightCubeVAO);
    glBindVertexArray(mesh.lightCubeVAO);
    glGenBuffers(1, &mesh.lightCubeVBO);


    glBindBuffer(GL_ARRAY_BUFFER, mesh.lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeFace1Verts), cubeFace1Verts, GL_STATIC_DRAW);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Face 2
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[1]);

    // VBO of the cube
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeFace2Verts), cubeFace2Verts, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // For the lights
    glGenVertexArrays(1, &mesh.lightCubeVAO);
    glBindVertexArray(mesh.lightCubeVAO);
    glGenBuffers(1, &mesh.lightCubeVBO);


    glBindBuffer(GL_ARRAY_BUFFER, mesh.lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeFace2Verts), cubeFace2Verts, GL_STATIC_DRAW);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Face 3
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[2]);

    // VBO of the cube
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeFace3Verts), cubeFace3Verts, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // For the lights
    glGenVertexArrays(1, &mesh.lightCubeVAO);
    glBindVertexArray(mesh.lightCubeVAO);
    glGenBuffers(1, &mesh.lightCubeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeFace3Verts), cubeFace3Verts, GL_STATIC_DRAW);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Face 4
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[3]);

    // VBO of the cube
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeFace4Verts), cubeFace4Verts, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // For the lights
    glGenVertexArrays(1, &mesh.lightCubeVAO);
    glBindVertexArray(mesh.lightCubeVAO);
    glGenBuffers(1, &mesh.lightCubeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeFace4Verts), cubeFace4Verts, GL_STATIC_DRAW);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);

    // Face 5
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[4]);

    // VBO of the cube
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeFace5Verts), cubeFace5Verts, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // For the lights
    glGenVertexArrays(1, &mesh.lightCubeVAO);
    glBindVertexArray(mesh.lightCubeVAO);
    glGenBuffers(1, &mesh.lightCubeVBO);


    glBindBuffer(GL_ARRAY_BUFFER, mesh.lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeFace5Verts), cubeFace5Verts, GL_STATIC_DRAW);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Face 6
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[5]);

    // VBO of the cube
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeFace6Verts), cubeFace6Verts, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // For the lights
    glGenVertexArrays(1, &mesh.lightCubeVAO);
    glBindVertexArray(mesh.lightCubeVAO);
    glGenBuffers(1, &mesh.lightCubeVBO);


    glBindBuffer(GL_ARRAY_BUFFER, mesh.lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeFace6Verts), cubeFace6Verts, GL_STATIC_DRAW);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Table top
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[6]);

    // VBO of the cube
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[6]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tableVerts), tableVerts, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // For the lights
    glGenVertexArrays(1, &mesh.lightCubeVAO);
    glBindVertexArray(mesh.lightCubeVAO);
    glGenBuffers(1, &mesh.lightCubeVBO);


    glBindBuffer(GL_ARRAY_BUFFER, mesh.lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tableVerts), tableVerts, GL_STATIC_DRAW);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Table leg 1
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[8]);

    // VBO of the cube
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[8]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tableLegVerts), tableLegVerts, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // For the lights
    glGenVertexArrays(1, &mesh.lightCubeVAO);
    glBindVertexArray(mesh.lightCubeVAO);
    glGenBuffers(1, &mesh.lightCubeVBO);


    glBindBuffer(GL_ARRAY_BUFFER, mesh.lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tableLegVerts), tableLegVerts, GL_STATIC_DRAW);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Table leg 2
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[9]);

    // VBO of the cube
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[9]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tableLegVerts), tableLegVerts, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // For the lights
    glGenVertexArrays(1, &mesh.lightCubeVAO);
    glBindVertexArray(mesh.lightCubeVAO);
    glGenBuffers(1, &mesh.lightCubeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tableLegVerts), tableLegVerts, GL_STATIC_DRAW);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Table leg 3
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[10]);

    // VBO of the cube
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[10]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tableLegVerts), tableLegVerts, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // For the lights
    glGenVertexArrays(1, &mesh.lightCubeVAO);
    glBindVertexArray(mesh.lightCubeVAO);
    glGenBuffers(1, &mesh.lightCubeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tableLegVerts), tableLegVerts, GL_STATIC_DRAW);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Table leg 4
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[11]);

    // VBO of the cube
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[11]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tableLegVerts), tableLegVerts, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // For the lights
    glGenVertexArrays(1, &mesh.lightCubeVAO);
    glBindVertexArray(mesh.lightCubeVAO);
    glGenBuffers(1, &mesh.lightCubeVBO);


    glBindBuffer(GL_ARRAY_BUFFER, mesh.lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tableLegVerts), tableLegVerts, GL_STATIC_DRAW);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Plane (Fourth Object)
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[7]);

    // VBO of the plane
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[7]);
    glBufferData(GL_ARRAY_BUFFER, planeVerts1.size() * sizeof(float), planeVerts1.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(6);

    /*
    * This code creates objects we will not use so it gets commented out.
    * 
    // Spheres (Fifth Object)
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[8]);

    // VBO of the sphere
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[8]);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices1.size() * sizeof(float), sphereVertices1.data(), GL_STATIC_DRAW);

    // EBO of sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBOs[8]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices1.size() * sizeof(unsigned int), sphereIndices1.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);


    // Third Cylinder (Sixth Object)
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[9]);

    // VBO of the cylinder
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[9]);
    glBufferData(GL_ARRAY_BUFFER, cylVertices3.size() * sizeof(float), cylVertices3.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);


    // Cones (Seventh Object)
    // bind the Vertex Array Object
    glBindVertexArray(mesh.VAOs[10]);

    // VBO of the cone
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOs[10]);
    glBufferData(GL_ARRAY_BUFFER, coneVerts1.size() * sizeof(float), coneVerts1.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // texture attibute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);

    //mesh.indexCounts[0] = cylSideVertices1.size();
    //mesh.indexCounts[1] = cylTopIndices2.size();
    //mesh.indexCounts[2] = cylBottomIndices1.size();
    //mesh.indexCounts[3] = cylSideVertices2.size();
    //mesh.indexCounts[4] = cylTopIndices2.size();
    //mesh.indexCounts[5] = cylBottomIndices2.size();*/

    mesh.indexCounts[7] = planeVerts1.size(); //This line must remain to draw the plan under our table

    //mesh.indexCounts[8] = sphereIndices1.size();
    //mesh.indexCounts[9] = cylVertices3.size();
    //mesh.indexCounts[10] = coneVerts1.size();
}

/*
 * Unused function, but it remains because it isn't breaking anything.
 */

// Function to generate the side veritces of a cylinder
std::vector<float> genCylSideVerts(int sides, float height, float radius, color color) {

    std::vector<float> vertices;

    float angleStep = 2.0f * PI / sides;

    float halfHeight = height / 2.0f;
    

    // Bottom circle vertices
    for (int i = 0; i <= sides; ++i) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        // Bottom Vertex
        // Position
        vertices.push_back(x);
        vertices.push_back(-halfHeight); // y (bottom)
        vertices.push_back(z);

        // Normal
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);

        // Color 
        vertices.push_back(color.redValue);     // r
        vertices.push_back(color.greenValue);   // g
        vertices.push_back(color.blueValue);    // b
        vertices.push_back(color.alphaValue);   // a

        vertices.push_back(static_cast<float>(i) / static_cast<float>(sides));    // u
        vertices.push_back(0.0f);   // v

        // Top Vertex
        // Position
        vertices.push_back(x);
        vertices.push_back(halfHeight); // y (top)
        vertices.push_back(z);

        // Normal
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);

        // Color 
        vertices.push_back(color.redValue);     // r
        vertices.push_back(color.greenValue);   // g
        vertices.push_back(color.blueValue);    // b
        vertices.push_back(color.alphaValue);   // a
        

        vertices.push_back(static_cast<float>(i) / static_cast<float>(sides));    // u
        vertices.push_back(1.0f);   // v
    }

    return vertices;
}

/*
 * Unused function, but it remains because it isn't breaking anything.
 */

// Function to generate the top veritces of a cylinder
std::vector<float> genCylTopVerts(int sides, float height, float radius, color color) {

    std::vector<float> vertices;

    float angleStep = 2.0f * PI / sides;

    float halfHeight = height / 2.0f;

    // Top circle vertices
    for (int i = 0; i < sides; ++i) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        // Position
        vertices.push_back(x);
        vertices.push_back(halfHeight); // y (top)
        vertices.push_back(z);

        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);

        // Color 
        vertices.push_back(color.redValue);     // r
        vertices.push_back(color.greenValue);   // g
        vertices.push_back(color.blueValue);    // b
        vertices.push_back(color.alphaValue);   // a


        // Texture coordinates for the top circle
        vertices.push_back(0.5f + 0.5f * cos(angle));  // u
        vertices.push_back(0.5f + 0.5f * sin(angle));  // v
    }
    
    // For triangle fan
    float y =  halfHeight; 

    // Position
    vertices.push_back(0.0f);  // x
    vertices.push_back(y);     // y
    vertices.push_back(0.0f);  // z

    // Normal
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);

    // Color 
    vertices.push_back(color.redValue);     // r
    vertices.push_back(color.greenValue);   // g
    vertices.push_back(color.blueValue);    // b
    vertices.push_back(color.alphaValue);   // a
         

    // Texture coordinates for the center points
    vertices.push_back(0.5f);  // u (center point)
    vertices.push_back(0.5f);  // v (center point)
        
    return vertices;
}

/*
 * Unused function, but it remains because it isn't breaking anything.
 */

// Function to generate the bottom veritces of a cylinder
std::vector<float> genCylBottomVerts(int sides, float height, float radius, color color) {

    std::vector<float> vertices;

    float angleStep = 2.0f * PI / sides;

    float halfHeight = height / 2.0f;

    // Bottom circle vertices
    for (int i = 0; i < sides ; ++i) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        // Position
        vertices.push_back(x);
        vertices.push_back(-halfHeight); // y (bottom)
        vertices.push_back(z);

        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
        vertices.push_back(0.0f);

        // Color 
        vertices.push_back(color.redValue);     // r
        vertices.push_back(color.greenValue);   // g
        vertices.push_back(color.blueValue);    // b
        vertices.push_back(color.alphaValue);   // a
        

        // Texture coordinates for the bottom circle
        vertices.push_back(0.5f + 0.5f * cos(angle));  // u
        vertices.push_back(0.5f + 0.5f * sin(angle));  // v
    }

    // For triangle fan
    float y = -halfHeight;

    // Position
    vertices.push_back(0.0f);  // x
    vertices.push_back(y);     // y
    vertices.push_back(0.0f);  // z

    // Normal
    vertices.push_back(0.0f);
    vertices.push_back(-1.0f);
    vertices.push_back(0.0f);

    // Color 
    vertices.push_back(color.redValue);     // r
    vertices.push_back(color.greenValue);   // g
    vertices.push_back(color.blueValue);    // b
    vertices.push_back(color.alphaValue);   // a


    // Texture coordinates for the center points
    vertices.push_back(0.5f);  // u (center point)
    vertices.push_back(0.5f);  // v (center point)

    return vertices;
}

/*
 * Unused function, but it remains because it isn't breaking anything.
 */

// Function to generate a cylinder's bottom indices
std::vector<unsigned int> genCylBottomIndices(int sides) {
    std::vector<unsigned int> indices;

    for (int i = 0; i < sides; ++i) {
        int currentVertex = i;
        int nextVertex = (i + 1) % sides;

        // Triangle fan indices
        indices.push_back(currentVertex);
        indices.push_back(nextVertex);
        indices.push_back(sides);
    }

    return indices;
}

/*
 * Unused function, but it remains because it isn't breaking anything.
 */

// Function to generate a cylinder's top indices
std::vector<unsigned int> genCylTopIndices(int sides) {
    std::vector<unsigned int> indices;

    for (int i = 0; i < sides; ++i) {
        int currentVertex = i;
        int nextVertex = (i + 1) % sides;

        // Triangle fan indices
        indices.push_back(currentVertex);
        indices.push_back(nextVertex);
        indices.push_back(sides); // Center point
    }

    return indices;
}

/*
 * Unused function, but it remains because it isn't breaking anything.
 */

// Function for generating a sphere's vertices
std::vector<float> genSphereVerts(float radius, color color) {
    std::vector<float> vertices;
    float phi, theta;

    // Generate vertices and colors for the sphere
    for (int i = 0; i <= rings; ++i) {
        phi = static_cast<float>((-PI / 2.0f) + PI *  i / rings); // Vertical angle

        for (int j = 0; j <= segments; ++j) {
            theta = static_cast<float>(2 * PI) * j / segments; // Horizontal angle

            // Calculate the vertex position
            float x = radius * std::cos(phi) * std::cos(theta);
            float y = radius * std::sin(phi);
            float z = radius * std::cos(phi) * std::sin(theta);

            // Add the vertex position to the vector
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normals
            vertices.push_back(x / radius);
            vertices.push_back(y / radius);
            vertices.push_back(z / radius);

            // Color 
            vertices.push_back(color.redValue);     // r
            vertices.push_back(color.greenValue);   // g
            vertices.push_back(color.blueValue);    // b
            vertices.push_back(color.alphaValue);   // a

            // Calculate and push texture coordinates
            float u = static_cast<float>(j) / segments; // U texture coordinate
            float v = static_cast<float>(i) / rings;    // V texture coordinate
            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

    return vertices;
}

/*
 * Unused function, but it remains because it isn't breaking anything.
 */

// Function for generating a sphere's indices
std::vector<unsigned int> genSphereIndices()
{
    std::vector<unsigned int> indices;

    // Generate indices for rendering triangles
    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < segments; ++j) {
            int current = i * (segments + 1) + j;
            int next = current + segments + 1;

            // Define two triangles using indices
            indices.push_back(current);
            indices.push_back(current + 1);
            indices.push_back(next);

            indices.push_back(current + 1);
            indices.push_back(next + 1);
            indices.push_back(next);
        }
    }

    return indices;
}

/*
 * Unused function, but it remains because it isn't breaking anything.
 */

// Function to generate a pyramid's vertices
std::vector<float> genPyramidVerts(int sides, float height, float radius, color color)
{
    std::vector<float> vertices;
    vertices.reserve((sides + 1) * 9); // Each vertex has 9 components now (including alpha)

    float angleIncrement = 2 * PI / static_cast<float>(sides);

    float r = color.redValue;
    float g = color.greenValue;
    float b = color.blueValue;
    float a = color.alphaValue;

    // Vertex at the top of the pyramid/cone
    float topVertex[] = { 0.0f, height / 2.0f, 0.0f, r, g, b, a, 0.5f, 1.0f }; // Center of the texture

    for (int i = 0; i < sides; ++i) {
        // Calculate the coordinates for the base of the pyramid/cone
        float x1 = radius * cos(i * angleIncrement);
        float z1 = radius * sin(i * angleIncrement);
        float x2 = radius * cos((i + 1) * angleIncrement);
        float z2 = radius * sin((i + 1) * angleIncrement);

        // Calculate normals for the side vertices using the parametric approach
        float tangent1[3] = { x2 - x1, -height, z2 - z1 };
        float tangent2[3] = { x1, height, z1 };
        float normal[3] = {
            tangent1[1] * tangent2[2] - tangent1[2] * tangent2[1],
            tangent1[2] * tangent2[0] - tangent1[0] * tangent2[2],
            tangent1[0] * tangent2[1] - tangent1[1] * tangent2[0]
        };

        // Normalize the normal vector
        float normalLength = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
        normal[0] /= normalLength;
        normal[1] /= normalLength;
        normal[2] /= normalLength;

        // Calculate the texture coordinates for the sides
        float u1 = static_cast<float>(i) / static_cast<float>(sides - 1); // Range [0, 1]
        float u2 = static_cast<float>(i + 1) / static_cast<float>(sides - 1);

        // Add vertices for the base triangle with texture coordinates
        vertices.insert(vertices.end(), {
            x1, -height / 2.0f, z1,
            normal[0], normal[1], normal[2],
            r, g, b, a,
            u1, 0.0f,

            x2, -height / 2.0f, z2,
            normal[0], normal[1], normal[2],
            r, g, b, a,
            u2, 0.0f,

            0.0f, -height / 2.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            r, g, b, a,
            0.5f, 0.5f
            });

        // Add vertices for the side triangle with texture coordinates
        vertices.insert(vertices.end(), {
            x1, -height / 2.0f, z1,
            normal[0], normal[1], normal[2],
            r, g, b, a,
            u1, 0.0f,

            x2, -height / 2.0f, z2,
            normal[0], normal[1], normal[2],
            r, g, b, a,
            u2, 0.0f,

            0.0f, height / 2.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            r, g, b, a,
            0.5f, 1.0f
            });
    }

    // Add vertices for the base
    for (int i = 0; i < sides; ++i) {
        float x1 = radius * cos(i * angleIncrement);
        float z1 = radius * sin(i * angleIncrement);
        float x2 = radius * cos((i + 1) * angleIncrement);
        float z2 = radius * sin((i + 1) * angleIncrement);

        // Calculate normals for the base vertices
        float baseNormal[] = { 0.0f, -1.0f, 0.0f };


        // Calculate the texture coordinates for the base
        float u1 = 0.5f + 0.5f * cos(i * angleIncrement);
        float u2 = 0.5f + 0.5f * cos((i + 1) * angleIncrement);

        // Add vertices for the base triangle with texture coordinates
        vertices.insert(vertices.end(), {
            x1, -height / 2.0f, z1,
            baseNormal[0], baseNormal[1], baseNormal[2],
            r, g, b, a,
            u1, 0.0f,

            x2, -height / 2.0f, z2,
            baseNormal[0], baseNormal[1], baseNormal[2],
            r, g, b, a,
            u2, 0.0f,

            0.0f, -height / 2.0f, 0.0f,
            baseNormal[0], baseNormal[1], baseNormal[2],
            r, g, b, a,
            0.5f, 0.5f
            });
    }

    return vertices;
}

// Function to generate a plane's vertices
std::vector<float> genPlaneVerts(int sections, color color) {

    std::vector<float> vertices;

    float r = color.redValue;
    float g = color.greenValue;
    float b = color.blueValue;
    float a = color.alphaValue;

    float step = 2.0f / sections;

    for (float x = 1.0f; x > -1.0f; x -= step) {
        for (float z = 1.0f; z >= -1.0f; z -= step) {
            // Triangle 1
            vertices.insert(vertices.end(), {
            x, 0.0f, z,
            0.0f, 1.0f, 0.0f,
            r, g, b, a,
            2.0f, 0.0f,

            x - step, 0.0f, z,
            0.0f, 1.0f, 0.0f,
            r, g, b, a,
            0.0f, 0.0f,

            x, 0.0f, z - step,
            0.0f, 1.0f, 0.0f,
            r, g, b, a,
            2.0f, 2.0f,
            });
            // Triangle 2
            vertices.insert(vertices.end(), {
            x - step, 0.0f, z - step,
            0.0f, 1.0f, 0.0f,
            r, g, b, a,
            0.0f, 2.0f,

            x - step, 0.0f, z,
            0.0f, 1.0f, 0.0f,
            r, g, b, a,
            0.0f, 0.0f,

            x, 0.0f, z - step,
            0.0f, 1.0f, 0.0f,
            r, g, b, a,
            2.0f, 2.0f,
            });

        }
    }

    return vertices;
}

bool progInitialize(GLFWwindow** window) {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2DScene Recreation", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    // Setting callbacks
    glfwSetFramebufferSizeCallback(*window, framebuffer_size_callback);
    glfwSetCursorPosCallback(*window, mouse_callback);
    glfwSetScrollCallback(*window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    return true;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)	// When pressing W
        camera.ProcessKeyboard(FORWARD, deltaTime);			// move forwards
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)	// When pressing S
        camera.ProcessKeyboard(BACKWARD, deltaTime);		// move backwards
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)	// When pressing A
        camera.ProcessKeyboard(LEFT, deltaTime);			// move to the left
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)	// When pressing D
        camera.ProcessKeyboard(RIGHT, deltaTime);			// move to the right
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)	// When pressing Q
        camera.ProcessKeyboard(UP, deltaTime);				// move up
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)	// When pressing E
        camera.ProcessKeyboard(DOWN, deltaTime);			// move down
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)	// When P is pressed
        isPerspective = !isPerspective;                     // toggle orthographic and 
        toggleView();                                       // perspective views
}

// Function for toggling view between orthograpic and persepctive
void toggleView() {
    if (isPerspective)
    {
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    }
    else
    {
        float orthoWidth = 1.0f; 
        float aspectRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;
        projection = glm::ortho(-orthoWidth * aspectRatio, orthoWidth * aspectRatio, -orthoWidth, orthoWidth, 0.0001f, 100.0f);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}