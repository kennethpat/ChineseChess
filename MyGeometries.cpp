//
//  MyGeometries.cpp
//
//   Sets up and renders 
//     - the ground plane, and
//     - the surface of rotation
//   for the Math 155A project #6.
//


// Use the static library (so glew32.dll is not needed):
#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include "LinearR3.h"		// Adjust path as needed.
#include "LinearR4.h"		// Adjust path as needed.
#include "MathMisc.h"       // Adjust path as needed

#include "MyGeometries.h"
#include "TextureProj.h"
#include "PhongData.h"
#include "RgbImage.h"
#include "GlGeomCylinder.h"

#include <iostream>
using namespace std;

// **********************************
// Material to underlie a texture map.
// YOU MAY DEFINE A SECOND ONE OF THESE IF YOU WISH
// **********************************
phMaterial materialUnderTexture;

// **************************
// Information for loading textures
// **************************
const int NumTextures = 16;
unsigned int TextureNames[NumTextures];     // Texture names generated by OpenGL
const char* TextureFiles[NumTextures] = {
    "./Textures/Board.bmp", // 0
    "./Textures/RoughWood.bmp", // 1
    "./Textures/RShuai.bmp", // 2
    "./Textures/RShi.bmp", // 3
    "./Textures/RXiang.bmp", // 4
    "./Textures/RMa.bmp", // 5
    "./Textures/RJu.bmp", // 6
    "./Textures/RPao.bmp", // 7
    "./Textures/RBing.bmp", // 8
    "./Textures/BJiang.bmp", // 9
    "./Textures/BShi.bmp", // 10
    "./Textures/BXiang.bmp", // 11
    "./Textures/BMa.bmp", // 12
    "./Textures/BJu.bmp", // 13
    "./Textures/BPao.bmp", // 14
    "./Textures/BZu.bmp" // 15
};

// *******************************
// For spheres and a cylinder and a torus (Torus is currently not used.)
// *******************************
GlGeomCylinder texCylinder(4, 4, 4);

// ************************
// General data helping with setting up VAO (Vertex Array Objects)
//    and Vertex Buffer Objects.
// ***********************
const int NumObjects = 3;
const int iFloor = 0;

unsigned int myVBO[NumObjects];  // a Vertex Buffer Object holds an array of data
unsigned int myVAO[NumObjects];  // a Vertex Array Object - holds info about an array of vertex data;
unsigned int myEBO[NumObjects];  // a Element Array Buffer Object - holds an array of elements (vertex indices)

// ********************************************
// This sets up for texture maps. It is called only once
// ********************************************
void SetupForTextures()
{
    // This material goes under the textures.
    // IF YOU WISH, YOU MAY DEFINE MORE THAN ONE OF THESE TO DIFFERENT GEOMETRIES
    materialUnderTexture.SpecularColor.Set(0.9, 0.9, 0.9);
    materialUnderTexture.AmbientColor.Set(0.3, 0.3, 0.3);
    materialUnderTexture.DiffuseColor.Set(0.7, 0.7, 0.7);       // Increase or decrease to adjust brightness
    materialUnderTexture.SpecularExponent = 40.0;

    // Load texture maps
    RgbImage texMap;

    glUseProgram(shaderProgramBitmap);
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(NumTextures, TextureNames);
    for (int i = 0; i < NumTextures; i++) {
        texMap.LoadBmpFile(TextureFiles[i]);            // Read i-th texture from the i-th file.
        glBindTexture(GL_TEXTURE_2D, TextureNames[i]);  // Bind (select) the i-th OpenGL texture

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Set best quality filtering.   Also see below for disabling mipmaps.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  // Requires that mipmaps be generated (see below)
        // You may also try GL_LINEAR_MIPMAP_NEAREST -- try looking at the wall from a 30 degree angle, and look for sweeping transitions.

        // Store the texture into the OpenGL texture named TextureNames[i]
        long int textureWidth = texMap.GetNumCols();
        long int textureHeight = texMap.GetNumRows();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (short)textureWidth, (short)textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, texMap.ImageData());
 #if 1
        // Use mipmaps  (Best!)
        glGenerateMipmap(GL_TEXTURE_2D);
#else
        // Don't use mipmaps.  Try moving away from the brick wall a great distance
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif

    }

    // Make sure that the shaderProgramBitmap uses the GL_TEXTURE_0 texture.
    glUseProgram(shaderProgramBitmap);
    glUniform1i(glGetUniformLocation(shaderProgramBitmap, "theTextureMap"), 0);
    glActiveTexture(GL_TEXTURE0);


}

// **********************
// This sets up geometries needed for 
//   (a) the floor (ground plane)
//   (b) the back wall
//   (c) the circular mesh
//   (d) two spheres
//   (e) one cylinder
//  It is called only once.
//  YOU NEED TO CHANGE THIS ONCE YOU ADD THE TEXTURE COORDINATES TO THE CIRCULAR SURFACE.
// **********************
void MySetupSurfaces() {
    texCylinder.InitializeAttribLocations(vertPos_loc, vertNormal_loc, vertTexCoords_loc);

    // Initialize the VAO's, VBO's and EBO's for the ground plane, the back wall
    // and the surface of rotation. Gives them the "vertPos" location,
    // and the "vertNormal"  and the "vertTexCoords" locations in the shader program.
    // No data is loaded into the VBO's or EBO's for the circular surface until the "Remesh"
    //   routines is called

    glGenVertexArrays(NumObjects, &myVAO[0]);
    glGenBuffers(NumObjects, &myVBO[0]);
    glGenBuffers(NumObjects, &myEBO[0]);

    // For the Floor:
    // Allocate the needed Vertex Array Objects (VAO's),
    //      Vertex Buffer Objects (VBO's) and Element Array Buffer Objects (EBO's)
    // Since the floor has only four vertices.  Each vertex stores its
    //    position, its normal (0,1,0) and its (s,t)-coordinates.
    // YOU DO NOT NEED TO REMESH THE FLOOR (OR THE BACK WALL) SINCE WE USE PHONG INTERPOLATION
    float floorVerts[] = {
        // Position              // Normal                  // Texture coordinates
        -7.5f, 0.0f, -7.5f,      0.0f, 1.0f, 0.0f,          0.0f, 1.0f,         // Back left
         7.5f, 0.0f, -7.5f,      0.0f, 1.0f, 0.0f,          1.0f, 1.0f,         // Back right
         7.5f, 0.0f,  7.5f,      0.0f, 1.0f, 0.0f,          1.0f, 0.0f,         // Front right
        -7.5f, 0.0f,  7.5f,      0.0f, 1.0f, 0.0f,          0.0f, 0.0f,         // Front left
    };
    unsigned int floorElts[] = { 0, 3, 1, 2 };
    glBindBuffer(GL_ARRAY_BUFFER, myVBO[iFloor]);
    glBindVertexArray(myVAO[iFloor]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVerts), floorVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(vertPos_loc, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);	   // Vertex positions in the VBO
    glEnableVertexAttribArray(vertPos_loc);									// Enable the stored vertices
    glVertexAttribPointer(vertNormal_loc, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));	// Vertex normals in the VBO
    glEnableVertexAttribArray(vertNormal_loc);									// Enable the stored vertices
    glVertexAttribPointer(vertTexCoords_loc, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));	// Vertex texture coordinates in the VBO
    glEnableVertexAttribArray(vertTexCoords_loc);									// Enable the stored vertices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myEBO[iFloor]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorElts) * sizeof(unsigned int), floorElts, GL_STATIC_DRAW);

    check_for_opengl_errors();      // Watch the console window for error messages!
}

void MyRemeshGeometries() 
{
    texCylinder.Remesh(meshRes, meshRes, meshRes);
    check_for_opengl_errors();      // Watch the console window for error messages!
}

void MyRenderGeometries() {

    float matEntries[16];       // Temporary storage for floats

    selectShaderProgram(shaderProgramBitmap);
    LinearMapR4 m = viewMatrix;
    glBindVertexArray(myVAO[iFloor]);                // Select the floor VAO (Vertex Array Object)
    materialUnderTexture.LoadIntoShaders();         // Use the bright underlying color
    m.DumpByColumns(matEntries);           // Apply the model view matrix
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[0]);
    glUniform1i(applyTextureLocation, true);           // Enable applying the texture!
    // Draw the floor as a single triangle strip
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, (void*)0);
    glUniform1i(applyTextureLocation, false);           // Turn off applying texture!
    
    // Red pieces
    // Red Shuai
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(0.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[2]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    // Red Shi x 2
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(2.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[3]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(-2.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[3]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    // Red Xiang x 2
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(4.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[4]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(-4.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[4]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    // Red Ma x 2
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(6.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[5]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(-6.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[5]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    // Red Ju x 2
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(8.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[6]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(-8.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[6]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    // Red Pao x 2
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(6.0, 0.0, 4.5);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[7]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(-6.0, 0.0, 4.5);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[7]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    // Red Bing x 5
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(8.0, 0.0, 2.75);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[8]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(-8.0, 0.0, 2.75);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[8]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(4.0, 0.0, 2.75);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[8]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(-4.0, 0.0, 2.75);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[8]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(0.0, 0.0, 2.75);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[8]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    // Black pieces
    // Black Jiang
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(0.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[9]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    // Black Shi x 2
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(2.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[10]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(-2.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[10]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    // Black Xiang x 2
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(4.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[11]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(-4.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[11]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    // Black Ma x 2
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(6.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[12]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(-6.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[12]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    // Black Ju x 2
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(8.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[13]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(-8.0, 0.0, 8.1);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[13]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    // Black Pao x 2
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(6.0, 0.0, 4.5);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[14]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(-6.0, 0.0, 4.5);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[14]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    // Black Zu x 5
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(8.0, 0.0, 2.75);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[15]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(-8.0, 0.0, 2.75);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[15]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(4.0, 0.0, 2.75);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[15]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(-4.0, 0.0, 2.75);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[15]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
    
    m = viewMatrix;
    m.Mult_glRotate(PI, 0.0, 1.0, 0.0);
    m.Mult_glScale(0.8, 0.25, 0.8);
    m.Mult_glTranslate(0.0, 0.0, 2.75);
    m.DumpByColumns(matEntries);
    glUniformMatrix4fv(modelviewMatLocation, 1, false, matEntries);
    glBindTexture(GL_TEXTURE_2D, TextureNames[1]);
    glUniform1i(applyTextureLocation, true);
    texCylinder.RenderSide();
    glBindTexture(GL_TEXTURE_2D, TextureNames[15]);
    texCylinder.RenderTop();
    texCylinder.RenderBase();
    glUniform1i(applyTextureLocation, false);
        
    check_for_opengl_errors();      // Watch the console window for error messages!
}