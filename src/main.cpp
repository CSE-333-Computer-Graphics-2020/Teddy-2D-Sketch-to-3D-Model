#include "utils.h"
#include "math.h"
#include "poly2tri.h"
#include <set>
#include <iostream>
#include <math.h>
// GLobal variables
std::vector<float> controlPoints;
int width = 640, height = 640; 
bool controlPointsUpdated = false;
std::vector<p2t::Point*> points;
std::vector<p2t::Triangle*> triangles;
std::vector<float> triangleFlattenedArray;
p2t::CDT* cdt;

void pushPoint(std::vector<p2t::Point*> & points,p2t::Point* p){
    p2t::Point* prevPoint;
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

int main(int, char* argv[])
{
    GLFWwindow* window = setupWindow(width, height);
    ImGuiIO& io = ImGui::GetIO(); // Create IO object

    ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    unsigned int shaderProgram = createProgram("./shaders/vshader.vs", "./shaders/fshader.fs");
	glUseProgram(shaderProgram);

    // Create VBOs, VAOs
    unsigned int VBO_controlPoints;
    unsigned int VAO_controlPoints;
    glGenBuffers(1, &VBO_controlPoints);
    glGenVertexArrays(1, &VAO_controlPoints);

    unsigned int VBO_triangles;
    unsigned int VAO_triangles;
    glGenBuffers(1, &VBO_triangles);
    glGenVertexArrays(1, &VAO_triangles);
    

    float rescaled_x;
    float rescaled_y;
    bool mouseDowned = false;
    p2t::Point* p;
    //Display loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Rendering
        showOptionsDialog(controlPoints, io);
        ImGui::Render();
        // Add a new point on mouse click
        float x,y ;
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        if (io.MouseDown[0] && !ImGui::IsAnyItemActive()){
            x = io.MousePos.x;
            y = io.MousePos.y;
            addPoints(controlPoints, x, y, width, height);
            rescaled_x = -1.0 + ((1.0*x - 0) / (width - 0)) * (1.0 - (-1.0));
            rescaled_y = -1.0 + ((1.0*(height - y) - 0) / (height - 0)) * (1.0 - (-1.0));
            p = new p2t::Point(rescaled_x,rescaled_y); 
            pushPoint(points,p);
            if (mouseDowned){
                addPoints(controlPoints, x, y, width, height);
            }
            mouseDowned = true;
            controlPointsUpdated = true;
        }

        if (io.MouseReleased[0] &&  !ImGui::IsAnyItemActive()){
            x = io.MousePos.x;
            y = io.MousePos.y;
            addPoints(controlPoints, x, y, width, height);
            rescaled_x = -1.0 + ((1.0*x - 0) / (width - 0)) * (1.0 - (-1.0));
            rescaled_y = -1.0 + ((1.0*(height - y) - 0) / (height - 0)) * (1.0 - (-1.0));
            p = new p2t::Point(rescaled_x,rescaled_y); 
            pushPoint(points,p);
            controlPointsUpdated = true;
            mouseDowned = false;

            cdt = new p2t::CDT(points);
            cdt->Triangulate();
    		triangles = cdt->GetTriangles();
            addToTriangleBuffer();  
        }

        if(controlPointsUpdated) {

            if (io.MouseDown[0] && !ImGui::IsAnyItemActive()){
            // // Update VAO/VBO for control points (since we added a new point)
                glBindVertexArray(VAO_controlPoints);
                glBindBuffer(GL_ARRAY_BUFFER, VBO_controlPoints);
                glBufferData(GL_ARRAY_BUFFER, controlPoints.size()*sizeof(GLfloat), &controlPoints[0], GL_DYNAMIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0); //Enable first attribute buffer (vertices)
                glBindVertexArray(VAO_controlPoints);
                glDrawArrays(GL_LINES, 0, controlPoints.size()/3); // Draw points
                glUseProgram(0);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                glfwSwapBuffers(window);
            }

            if (io.MouseReleased[0] &&  !ImGui::IsAnyItemActive()){

                glBindVertexArray(VAO_triangles);
                glBindBuffer(GL_ARRAY_BUFFER, VAO_triangles);
                glBufferData(GL_ARRAY_BUFFER, triangleFlattenedArray.size()*sizeof(GLfloat), &triangleFlattenedArray[0], GL_DYNAMIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                controlPointsUpdated = false; // Finish all VAO/VBO 
                glBindVertexArray(VAO_triangles);
                glDrawArrays(GL_LINES, 0, triangleFlattenedArray.size()/3); // Draw points
                glUseProgram(0);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                glfwSwapBuffers(window);
            }
        }
        glUseProgram(shaderProgram);
    }

    // glDeleteBuffers(1, &VBO_triangles);
    glDeleteBuffers(1, &VBO_triangles);
    glDeleteBuffers(1,&VBO_controlPoints);
    //TODO:

    // Cleanup
    cleanup(window);
    return 0;
}
