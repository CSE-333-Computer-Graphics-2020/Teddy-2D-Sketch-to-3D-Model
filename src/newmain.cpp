#include "utils.h"
#include "math.h"
#include "poly2tri.h"
#include <set>
#include <iostream>
#include <math.h>
#include<eigen3/Eigen/Dense>
// GLobal variables
std::vector<float> controlPoints;
int width = 640, height = 640; 
bool controlPointsUpdated = false;
std::vector<p2t::Point*> points;
std::vector<p2t::Triangle*> triangles;
std::vector<float> triangleFlattenedArray;
p2t::CDT* cdt;

void pushPoint(float x,float y){
    p2t::Point* prevPoint;
    double rescaled_x = -1.0 + ((1.0*x - 0) / (width - 0)) * (1.0 - (-1.0));
    double rescaled_y = -1.0 + ((1.0*(height - y) - 0) / (height - 0)) * (1.0 - (-1.0));
    p2t::Point* p = new p2t::Point(rescaled_x,rescaled_y); 
    double minDist = 0.1;
    if (points.empty()){
        points.push_back(p);
    }
    else{
        prevPoint = points.back();
        double distance = sqrt(pow(p->x-prevPoint->x,2) + pow(prevPoint->y-p->y,2));
        
        if (!((prevPoint->x==p->x && prevPoint->y==p->y) || (distance<=minDist))){
            points.push_back(p);
        }
    }
}

void addToTriangleBuffer(){
    for (auto triangle: triangles){
        for (int i =0;i<3;i++){
            triangleFlattenedArray.push_back(triangle->GetPoint(i)->x);
            triangleFlattenedArray.push_back(triangle->GetPoint(i)->y);
            triangleFlattenedArray.push_back(0);
        }
        triangleFlattenedArray.push_back(triangle->GetPoint(0)->x);
        triangleFlattenedArray.push_back(triangle->GetPoint(0)->y);
        triangleFlattenedArray.push_back(0);
        triangleFlattenedArray.push_back(triangle->GetPoint(2)->x);
        triangleFlattenedArray.push_back(triangle->GetPoint(2)->y);
        triangleFlattenedArray.push_back(0);
        triangleFlattenedArray.push_back(triangle->GetPoint(1)->x);
        triangleFlattenedArray.push_back(triangle->GetPoint(1)->y);
        triangleFlattenedArray.push_back(0);
    }
}

void setupModelTransformation(unsigned int &program)
{
        //Modelling transformations (Model -> World coordinates)
    glm::mat4 model;
    // TODO: Reset modelling transformations to Identity. Uncomment line below before attempting Q4! and Comment out for Q1 and 3.
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -1));//Model coordinates are the world coordinates
    model = glm::rotate(model, (3.14f/6.0f), glm::vec3(1.0,1.0,0.0));
    

    //Pass on the modelling matrix to the vertex shader
    glUseProgram(program);
    int vModel_uniform = glGetUniformLocation(program, "vModel");
    if(vModel_uniform == -1){
        fprintf(stderr, "Could not bind location: vModel\n");
        exit(0);
    }
    glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(model));
}

void setupViewTransformation(unsigned int &program)
{
    //Viewing transformations (World -> Camera coordinates
    glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 1.8), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    
    glUseProgram(program);
    int vView_uniform = glGetUniformLocation(program, "vView");
    if(vView_uniform == -1){
        fprintf(stderr, "Could not bind location: vView\n");
        exit(0);
    }
    glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(view));
}

void setupProjectionTransformation(unsigned int &program, int screen_width, int screen_height)
{
    //Projection transformation
    float aspect = (float)screen_width/(float)screen_height;

    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)screen_width/(GLfloat)screen_height, 0.1f, 1000.0f);

    //Pass on the projection matrix to the vertex shader
    glUseProgram(program);
    int vProjection_uniform = glGetUniformLocation(program, "vProjection");
    if(vProjection_uniform == -1){
        fprintf(stderr, "Could not bind location: vProjection\n");
        exit(0);
    }
    glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE, glm::value_ptr(projection));
}

void createCubeObject(unsigned int &program, unsigned int &cube_VAO)
{
    glUseProgram(program);

    //Bind shader variables
    int vVertex_attrib = glGetAttribLocation(program, "vVertex");
    if(vVertex_attrib == -1) {
        fprintf(stderr, "Could not bind location: vVertex\n");
        exit(0);
    }
    int vColor_attrib = glGetAttribLocation(program, "vColor");
    if(vColor_attrib == -1) {
        fprintf(stderr, "Could not bind location: vColor\n");
        exit(0);
    }

    //Cube data
    GLfloat cube_vertices[] = {0.1, 0.1, 0.1, -0.1, 0.1, 0.1, -0.1, -0.1, 0.1, 0.1, -0.1, 0.1, //Front
                   0.1, 0.1, -0.1, -0.1, 0.1, -0.1, -0.1, -0.1, -0.1, 0.1, -0.1, -0.1}; //Back
    GLushort cube_indices[] = {0, 2, 3, 0, 1, 2, //Front
                4, 7, 6, 4, 6, 5, //Back
                5, 2, 1, 5, 6, 2, //Left
                4, 3, 7, 4, 0, 3, //Right
                1, 0, 4, 1, 4, 5, //Top
                2, 7, 3, 2, 6, 7}; //Bottom
    GLfloat cube_colors[] = {1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1}; //Unique face colors

    //Generate VAO object
    glGenVertexArrays(1, &cube_VAO);
    glBindVertexArray(cube_VAO);

    //Create VBOs for the VAO
    //Position information (data + format)
    int nVertices = 6*2*3; //(6 faces) * (2 triangles each) * (3 vertices each)
    GLfloat *expanded_vertices = new GLfloat[nVertices*3];
    for(int i=0; i<nVertices; i++) {
        expanded_vertices[i*3] = cube_vertices[cube_indices[i]*3];
        expanded_vertices[i*3 + 1] = cube_vertices[cube_indices[i]*3+1];
        expanded_vertices[i*3 + 2] = cube_vertices[cube_indices[i]*3+2];
    }
    GLuint vertex_VBO;
    glGenBuffers(1, &vertex_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertices*3*sizeof(GLfloat), expanded_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vVertex_attrib);
    glVertexAttribPointer(vVertex_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    delete []expanded_vertices;

    //Color - one for each face
    GLfloat *expanded_colors = new GLfloat[nVertices*3];
    for(int i=0; i<nVertices; i++) {
        int color_index = i / 6;
        expanded_colors[i*3] = cube_colors[color_index*3];
        expanded_colors[i*3+1] = cube_colors[color_index*3+1];
        expanded_colors[i*3+2] = cube_colors[color_index*3+2];
    }
    GLuint color_VBO;
    glGenBuffers(1, &color_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, color_VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertices*3*sizeof(GLfloat), expanded_colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vColor_attrib);
    glVertexAttribPointer(vColor_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    delete []expanded_colors;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); //Unbind the VAO to disable changes outside this function.
}

int main(int, char* argv[])
{
    GLFWwindow* window = setupWindow(width, height);
    ImGuiIO& io = ImGui::GetIO(); // Create IO object

    ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    unsigned int shaderProgram = createProgram("./shaders/vshader1.vs", "./shaders/fshader1.fs");
	glUseProgram(shaderProgram);
    unsigned int shaderProgram2 = createProgram("./shaders/vshader.vs", "./shaders/fshader.fs");

    // Create VBOs, VAOs
    unsigned int VBO_controlPoints;
    unsigned int VAO_controlPoints;
    glGenBuffers(1, &VBO_controlPoints);
    glGenVertexArrays(1, &VAO_controlPoints);

    unsigned int VBO_triangles;
    unsigned int VAO_triangles;
    glGenBuffers(1, &VBO_triangles);
    glGenVertexArrays(1, &VAO_triangles);

    setupModelTransformation(shaderProgram);
    setupViewTransformation(shaderProgram);
    setupProjectionTransformation(shaderProgram, width , height);

    float rescaled_x;
    float rescaled_y;
    bool mouseDowned = false;
    //Display loop
    int flag=0;
    bool displayFlag=true;

    unsigned int cubeVAO;
    glGenVertexArrays(1, &cubeVAO);

    createCubeObject(shaderProgram, cubeVAO);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Rendering
        showOptionsDialog(controlPoints,points,triangles,triangleFlattenedArray, displayFlag,io);
        ImGui::Render();
        // Add a new point on mouse click
        if (!displayFlag){
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
            displayFlag=true;
            mouseDowned=false;
        }
        float x,y ;
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);
        if (io.MouseDown[0] && !ImGui::IsAnyItemActive()){
            x = io.MousePos.x;
            y = io.MousePos.y;
            addPoints(controlPoints, x, y, width, height);
            if (mouseDowned){
                addPoints(controlPoints, x, y, width, height);
            }
            pushPoint(x,y);
            mouseDowned = true;
            controlPointsUpdated = true;
        }

        if (io.MouseReleased[0] &&  !ImGui::IsAnyItemActive() && !ImGui::IsAnyItemHovered()){
            x = io.MousePos.x;
            y = io.MousePos.y;
            addPoints(controlPoints, x, y, width, height);
            pushPoint(x,y);
            cdt = new p2t::CDT(points);
            cdt->Triangulate();
    		triangles = cdt->GetTriangles();
            addToTriangleBuffer();  
            controlPointsUpdated = true;
            mouseDowned = false;
        }

        if(controlPointsUpdated) {
            flag=1;
            if (io.MouseDown[0] && !ImGui::IsAnyItemActive()){
                glUseProgram(shaderProgram);
                glEnable(GL_DEPTH_TEST);
                glEnable( GL_CULL_FACE ); // cull face
                glCullFace( GL_BACK );      // cull back face
                glBindVertexArray(cubeVAO); 
                glDrawArrays(GL_TRIANGLES, 0, 6*2*3);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                glDisable(GL_DEPTH_TEST);
                glUseProgram(shaderProgram2);
                glBindVertexArray(VAO_controlPoints);
                glBindBuffer(GL_ARRAY_BUFFER, VBO_controlPoints);
                glBufferData(GL_ARRAY_BUFFER, controlPoints.size()*sizeof(GLfloat), &controlPoints[0], GL_DYNAMIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0); 
                glBindVertexArray(VAO_controlPoints);
                glDrawArrays(GL_LINES, 0, controlPoints.size()/3);
                glfwSwapBuffers(window);
            }

            if (io.MouseReleased[0] &&  !ImGui::IsAnyItemActive()){
                glUseProgram(shaderProgram);
                glEnable(GL_DEPTH_TEST);
                glEnable( GL_CULL_FACE ); // cull face
                glCullFace( GL_BACK );      // cull back face 
                glBindVertexArray(cubeVAO); 
                glDrawArrays(GL_TRIANGLES, 0, 6*2*3);
                controlPointsUpdated = false;
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                glDisable(GL_DEPTH_TEST);
                glUseProgram(shaderProgram2);
                glBindVertexArray(VAO_triangles);
                glBindBuffer(GL_ARRAY_BUFFER, VAO_triangles);
                glBufferData(GL_ARRAY_BUFFER, triangleFlattenedArray.size()*sizeof(GLfloat), &triangleFlattenedArray[0], GL_DYNAMIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                glBindVertexArray(VAO_triangles);
                glDrawArrays(GL_LINES, 0, triangleFlattenedArray.size()/3);
                points.clear();
                glfwSwapBuffers(window);
            }
        }
        
        if(flag==0){
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
        }
        // glUseProgram(shaderProgram);
    }

    glDeleteBuffers(1, &VBO_triangles);
    glDeleteBuffers(1,&VBO_controlPoints);
    //TODO:

    // Cleanup
    cleanup(window);
    return 0;
}
