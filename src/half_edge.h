#include "utils.h"
#include <iostream>

typedef struct halfedge{
    struct halfvertex *v;
    struct halfface *f;
    struct halfedge *opposite;
    struct halfedge *next;
    struct halfedge *prev;
} edge;

typedef struct halfface{
    edge *e;
    int triangleType;
    int visit;
} face;

typedef struct halfvertex{
    float x;
    float y;
    float z;
    struct halfedge *e;
    int vNum;
} vertex;


static vertex * makeHalfEdgeVertex(float x, float y, float z, int vertexNumber){
    vertex *v = new vertex;
    v->x = x; v->y = y; v->z = z;
    v->e = NULL;  v->vNum = vertexNumber;
    return v;
}

static void makeHalfEdgeFace(int i1,int i2,int i3, std::vector<vertex *>& vertices, std::vector<face *>& faces){
    edge *edges[3];
    face *f = new face;
    f->visit = 0;
    f->triangleType = -1;
    edges[0] = new edge;
    edges[1] = new edge;
    edges[2] = new edge;
    f->e = edges[0];

    for (int i =0;i<3;i++){
        edges[i]->f = f;
        edges[i]->opposite = NULL;
        edges[i]->next = edges[(i+1)%3];
        edges[i]->prev = edges[ (i-1)>0 ? i-1 : 2];
    }
    
    glm::vec3 dir1(vertices[i2]->x-vertices[i1]->x, vertices[i2]->y-vertices[i1]->y, 0.0);
    
    glm::vec3 dir2(vertices[i3]->x-vertices[i1]->x, vertices[i3]->y-vertices[i1]->y, 0.0);

    GLfloat cross_prod_val = glm::cross(dir1, dir2).z;

    if (cross_prod_val<0.0){
        swapInts(i1,i3);
    }
    int indices[] = {i1,i2,i3};

    for (int i=0;i<3;i++){
        edges[i]->v = vertices.at(indices[i]);
        vertices.at(indices[i])->e = edges[i];
    }    

    delete indices; delete edges;

    faces.push_back(f);
}