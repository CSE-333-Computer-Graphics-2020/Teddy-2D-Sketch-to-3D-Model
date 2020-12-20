#include "utils.h"
#include "math.h"
#include "poly2tri.h"
#include <set>
#include <iostream>
#include <math.h>
#include "half_edge.h"
#include <map>
#include<bits/stdc++.h> 
// GLobal variables
std::vector<float> controlPoints;
std::vector<float> verticesToDraw;
int width = 640, height = 640; 
bool controlPointsUpdated = false;
std::vector<p2t::Point*> points;
std::vector<p2t::Triangle*> triangles;
std::vector<float> triangleFlattenedArray;
std::vector<vertex *> vertices;
std::vector<face *> faces;
std::map<std::pair<int ,int>,struct halfedge *> dictionary_edges;
p2t::CDT* cdt;
float translation[] = {0.0,0.0};


void pushPoint(float x,float y){
    p2t::Point* prevPoint;
    double rescaled_x = -1.0 + ((1.0*x - 0) / (width - 0)) * (1.0 - (-1.0));
    double rescaled_y = -1.0 + ((1.0*(height - y) - 0) / (height - 0)) * (1.0 - (-1.0));
    p2t::Point* p = new p2t::Point(rescaled_x,rescaled_y); 
    double minDist = 0.15;
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
    triangleFlattenedArray.clear();
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

void makeFaceBuffer(){
    triangleFlattenedArray.clear();
    
    for (auto face: faces){
        triangleFlattenedArray.push_back(face->e->v->x);
        triangleFlattenedArray.push_back(face->e->v->y);
        triangleFlattenedArray.push_back(face->e->v->z);
        triangleFlattenedArray.push_back(face->e->next->v->x);
        triangleFlattenedArray.push_back(face->e->next->v->y);
        triangleFlattenedArray.push_back(face->e->next->v->z);
        triangleFlattenedArray.push_back(face->e->next->next->v->x);
        triangleFlattenedArray.push_back(face->e->next->next->v->y);
        triangleFlattenedArray.push_back(face->e->next->next->v->z);
        triangleFlattenedArray.push_back(face->e->v->x);
        triangleFlattenedArray.push_back(face->e->v->y);
        triangleFlattenedArray.push_back(face->e->v->z);
        triangleFlattenedArray.push_back(face->e->next->next->v->x);
        triangleFlattenedArray.push_back(face->e->next->next->v->y);
        triangleFlattenedArray.push_back(face->e->next->next->v->z);
        triangleFlattenedArray.push_back(face->e->next->v->x);
        triangleFlattenedArray.push_back(face->e->next->v->y);
        triangleFlattenedArray.push_back(face->e->next->v->z);
    }

    for (auto face: faces){
        triangleFlattenedArray.push_back(face->e->v->x);
        triangleFlattenedArray.push_back(face->e->v->y);
        triangleFlattenedArray.push_back(-1*face->e->v->z);
        triangleFlattenedArray.push_back(face->e->next->v->x);
        triangleFlattenedArray.push_back(face->e->next->v->y);
        triangleFlattenedArray.push_back(-1*face->e->next->v->z);
        triangleFlattenedArray.push_back(face->e->next->next->v->x);
        triangleFlattenedArray.push_back(face->e->next->next->v->y);
        triangleFlattenedArray.push_back(-1*face->e->next->next->v->z);
        triangleFlattenedArray.push_back(face->e->v->x);
        triangleFlattenedArray.push_back(face->e->v->y);
        triangleFlattenedArray.push_back(-1*face->e->v->z);
        triangleFlattenedArray.push_back(face->e->next->next->v->x);
        triangleFlattenedArray.push_back(face->e->next->next->v->y);
        triangleFlattenedArray.push_back(-1*face->e->next->next->v->z);
        triangleFlattenedArray.push_back(face->e->next->v->x);
        triangleFlattenedArray.push_back(face->e->next->v->y);
        triangleFlattenedArray.push_back(-1*face->e->next->v->z);
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

    unsigned int VBO_pointsToDraw;
    unsigned int VAO_pointsToDraw;
    glGenBuffers(1, &VBO_pointsToDraw);
    glGenVertexArrays(1, &VAO_pointsToDraw);
    

    unsigned int VBO_triangles;
    unsigned int VAO_triangles;
    glGenBuffers(1, &VBO_triangles);
    glGenVertexArrays(1, &VAO_triangles);

    float rescaled_x;
    float rescaled_y;
    bool mouseDowned = false;
    //Display loop
    int flag=0;
    bool displayFlag=true;
    

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
        if (io.MouseDown[0] && !ImGui::IsAnyItemActive() && !ImGui::IsAnyItemHovered()){
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
            createHalfEdgeBuffers(points,triangles,vertices,faces);
            markTriangles(faces);
            faces= pruneTriangles(vertices, faces);

           
            // makeFaceBuffer();
            controlPointsUpdated = true;
            mouseDowned = false;
        }
        else if ((io.MouseReleased[1] || io.MouseReleased[2]) &&  !ImGui::IsAnyItemActive() && !ImGui::IsAnyItemHovered()){
            if(io.MouseReleased[2])
                faces= erection(vertices, faces);
            makeFaceBuffer();
            controlPointsUpdated = true;
            mouseDowned = false;
        }
        else if (io.KeyShift &&  !ImGui::IsAnyItemActive() && !ImGui::IsAnyItemHovered()){
            addToTriangleBuffer();
            controlPointsUpdated = true;
            mouseDowned = false;
        }
        else if (io.KeyCtrl &&  !ImGui::IsAnyItemActive() && !ImGui::IsAnyItemHovered()){
            verticesToDraw.clear();
            // std::cout<<"----------------------------------------------------------\n";
            int n=0;
            // faces= erection(vertices, faces);

            for(auto v:vertices){
                if(v->boundary){
                    verticesToDraw.push_back(v->x);
                    verticesToDraw.push_back(v->y);
                    verticesToDraw.push_back(v->z);
                    n++;
                }
            }
            std::cout<<"OUTSIDE TOTAL:       "<<n<<std::endl;
            controlPointsUpdated=true;
        }
        else if (io.KeyAlt &&  !ImGui::IsAnyItemActive() && !ImGui::IsAnyItemHovered()){
            
            verticesToDraw.clear();
            // std::cout<<"#############################################################\n";
            int n=0;
            for(auto v:vertices){
                if(!v->boundary){
                    verticesToDraw.push_back(v->x);
                    verticesToDraw.push_back(v->y);
                    verticesToDraw.push_back(v->z);
                    n++;
                }
            }
            std::cout<<"INSIDE TOTAL:       "<<n<<std::endl;
            controlPointsUpdated=true;
        }
        if(controlPointsUpdated) {
            flag=1;
            if (io.MouseDown[0] && !ImGui::IsAnyItemActive()){
                glBindVertexArray(VAO_controlPoints);
                glBindBuffer(GL_ARRAY_BUFFER, VBO_controlPoints);
                glBufferData(GL_ARRAY_BUFFER, controlPoints.size()*sizeof(GLfloat), &controlPoints[0], GL_DYNAMIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0); 
                glBindVertexArray(VAO_controlPoints);
                glDrawArrays(GL_LINES, 0, controlPoints.size()/3);
                glUseProgram(0);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                glfwSwapBuffers(window);
            }

            if ((io.MouseReleased[0] || io.MouseReleased[1] || io.KeyShift || io.MouseReleased[2]) &&  !ImGui::IsAnyItemActive()){
                glBindVertexArray(VAO_triangles);
                glBindBuffer(GL_ARRAY_BUFFER, VAO_triangles);
                glBufferData(GL_ARRAY_BUFFER, triangleFlattenedArray.size()*sizeof(GLfloat), &triangleFlattenedArray[0], GL_DYNAMIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                glBindVertexArray(VAO_triangles);
                glDrawArrays(GL_LINES, 0, triangleFlattenedArray.size()/3);
                glUseProgram(0);
                points.clear();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                glfwSwapBuffers(window);
                controlPointsUpdated = false;
            }

            if (io.KeyCtrl && !ImGui::IsAnyItemActive()){
                glBindVertexArray(VAO_pointsToDraw);
                glBindBuffer(GL_ARRAY_BUFFER, VBO_pointsToDraw);
                glBufferData(GL_ARRAY_BUFFER, verticesToDraw.size()*sizeof(GLfloat), &verticesToDraw[0], GL_DYNAMIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0); 
                glBindVertexArray(VAO_pointsToDraw);
                glDrawArrays(GL_POINTS, 0, verticesToDraw.size()/3);
                glUseProgram(0);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                glfwSwapBuffers(window);
            }

            if (io.KeyAlt && !ImGui::IsAnyItemActive()){
                glBindVertexArray(VAO_pointsToDraw);
                glBindBuffer(GL_ARRAY_BUFFER, VBO_pointsToDraw);
                glBufferData(GL_ARRAY_BUFFER, verticesToDraw.size()*sizeof(GLfloat), &verticesToDraw[0], GL_DYNAMIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0); 
                glBindVertexArray(VAO_pointsToDraw);
                glDrawArrays(GL_POINTS, 0, verticesToDraw.size()/3);
                glUseProgram(0);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                glfwSwapBuffers(window);
            }
        }
        if(flag==0){
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
        }
        glUseProgram(shaderProgram);
    }

    glDeleteBuffers(1, &VBO_triangles);
    glDeleteBuffers(1,&VBO_controlPoints);
    //TODO:

    // Cleanup
    cleanup(window);
    return 0;
}
