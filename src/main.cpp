#include "utils.h"
#include "math.h"
#include "poly2tri.h"
#include <set>
#include <iostream>
// GLobal variables
std::vector<float> controlPoints;
int width = 640, height = 640; 
bool controlPointsUpdated = false;
std::vector<p2t::Point*> points;
std::vector<p2t::Triangle*> triangles;
p2t::CDT* cdt;
p2t::Point* p;

void pushPoint(std::vector<p2t::Point*> & points,p2t::Point* p){
    p2t::Point* prevPoint;
    if (points.empty()){
        points.push_back(p);
    }
    else{
        prevPoint = points.back();
        if (!(prevPoint->x==p->x && prevPoint->y==p->y)){
            points.push_back(p);
        }
    }
}

// void removeSimilarPoints(std::vector<p2t::Point*> & points){
//     for (auto x:points){
//         std::cout<< x->x<<" "<<x->y<<std::endl;
//     }

// }

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

    int button_status = 0;

    float x0;
    float y0;
    float diff;
    float rescaled_x;
    float rescaled_y;
    float minDif;
    int minX;
    int minY;
    float translation[] = {0.0, 0.0};
    bool mouseUppedOnce = false;
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
        minDif=5.0;
        if (io.MouseDown[0] && !ImGui::IsAnyItemActive()){
            x = io.MousePos.x;
            y = io.MousePos.y;
            addPoints(controlPoints, x, y, width, height);
            p = new p2t::Point(x,y); 
            pushPoint(points,p);
            if (mouseUppedOnce){
                addPoints(controlPoints, x, y, width, height);
            }
            mouseUppedOnce = true;
            controlPointsUpdated = true;
        }

        if (io.MouseReleased[0] &&  !ImGui::IsAnyItemActive()){
            x = io.MousePos.x;
            y = io.MousePos.y;
            addPoints(controlPoints, x, y, width, height);
            p = new p2t::Point(x,y); 
            pushPoint(points,p);
            // std::cout<<p->x<<p->y<<std::endl;
            // points[0]->set_zero();
            controlPointsUpdated = true;
            mouseUppedOnce = false;
            // removeSimilarPoints(points);

            cdt = new p2t::CDT(points);
            cdt->Triangulate();
    		triangles = cdt->GetTriangles();
            for (auto triangle: triangles){
                std::cout<<triangle->GetPoint(0)->x<<" "<<triangle->GetPoint(0)->y<<std::endl;
                // std::cout<<triangle->a<<" "<<triangle->b<<" "<< triangle->c<<std::endl;

            }
            // 	cdt->Triangulate();
            // 	triangles = cdt->GetTriangles();
        }

        if(controlPointsUpdated) {
            // Update VAO/VBO for control points (since we added a new point)
            glBindVertexArray(VAO_controlPoints);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_controlPoints);
            glBufferData(GL_ARRAY_BUFFER, controlPoints.size()*sizeof(GLfloat), &controlPoints[0], GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0); //Enable first attribute buffer (vertices)
        
            controlPointsUpdated = false; // Finish all VAO/VBO 
        }

        glUseProgram(shaderProgram);

        // Draw control points
        glBindVertexArray(VAO_controlPoints);
		glDrawArrays(GL_LINES, 0, controlPoints.size()/3); // Draw points
        glUseProgram(0);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

    }

    glDeleteBuffers(1, &VBO_controlPoints);
    //TODO:

    // Cleanup
    cleanup(window);
    return 0;
}
