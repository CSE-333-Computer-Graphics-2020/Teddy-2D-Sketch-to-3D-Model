#include "utils.h"
#include <iostream>
#include <math.h>
#include <map>
extern std::map<std::pair<int ,int>,struct halfedge *> dictionary_edges;

typedef struct halfedge{
    struct halfvertex *v;
    struct halfface *f;
    struct halfedge *opposite;
    struct halfedge *next;
    struct halfedge *prev;
} edge;

typedef struct halfface{
    edge *e;
    int triangleType; //2 for terminal, 1 for sleeve and 0 for junction
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

static void makeHalfEdgeFace(int i1,int i2,int i3, const std::vector<vertex *> &vertices, std::vector<face *> &faces){
    // std::cout<<<<std::endl;
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
    std::pair<int,int> p;
    for (int i=0;i<3;i++){
        edges[i]->v = vertices.at(indices[i]);
        vertices.at(indices[i])->e = edges[i];

        p.first = std::min(indices[i],indices[(i+1)%3]);
        p.second = std::max(indices[i],indices[(i+1)%3]);
        if (dictionary_edges.find(p) == dictionary_edges.end()){
            dictionary_edges[p] = edges[i];
        }
        else{
            dictionary_edges[p]->opposite = edges[i];
            edges[i]->opposite = dictionary_edges[p];
        }
    }    

    // for (int i =0;i<3)

    faces.push_back(f);
}

static void markTriangles(std::vector<face *>& faces){
    int count;
    edge *e;
    edge *prevE;
    for (auto face:faces){
        count = 0;
        prevE = face->e;
        if (prevE -> opposite == NULL){
            count +=1;
        }
        e = prevE ->next;
        while (e != prevE){
            if (e->opposite == NULL){
                count +=1;
            }
            e = e -> next;
        }
        face->triangleType = count;
    }
}

static void createHalfEdgeBuffers(std::vector<p2t::Point*> &points, std::vector<p2t::Triangle*> &triangles,std::vector<vertex *> &vertices, std::vector<face *> &faces){

    // ;
    std::vector<int> indices;
    float x,y,z;
    float d;
    int indx;
    bool foundIndx;

    for (auto point: points){
        // std::cout<< vertices.size()<<std::endl;
        vertices.push_back(makeHalfEdgeVertex(point->x,point->y,0,vertices.size()));
    }

    for (auto triangle:triangles){
        for (int i =0;i<3;i++){
            x = triangle->GetPoint(i)->x;
            y = triangle->GetPoint(i)->y;
            foundIndx = false;
            indx= 0;
            // std::cout<<"TRIANGGLE"<<std::endl;
            for (auto point: points){
                // std::cout<<x<<", "<<y<<": "<<point->x<<", "<<point->y<<std::endl;
                d = sqrt(pow( x*x - point->x*point->x , 2 )+pow( y*y - point->y*point->y , 2 ));
                // std::cout<< d<<std::endl;
                if (d<=0.00001){
                    // std::cout<<"ok nishant"<<std::endl;
                    foundIndx = true;
                    break;
                }
                indx++;
            }
            indices.push_back(indx);
        }
        makeHalfEdgeFace(indices[0],indices[1],indices[2],vertices,faces);
        indices.clear();
    }
}

static void pruneTriangles(std::vector<vertex *> vertices, std::vector<face *> faces){
    edge *prevE;
    edge *e;
    edge *opp;
    float fanPoint[3];
    std::vector<vertex *> uneccVertices;
    for (auto face:faces){
        if (face->triangleType==2){
            prevE = face->e;
            e = face->e;
            while (e->opposite!=NULL || e != prevE){
                e = e->opposite;
            }
            vertex *v1 = e->v;
            vertex *v2 = e->next->v;
            vertex *v3 = e->next->next->v;
            uneccVertices.push_back(v3);
            if (e->opposite->f->triangleType ==0){
                opp = e ->opposite;
                fanPoint[0] = (v1->x+v2->x + opp->next->next->v->x)/3;
                fanPoint[1] = (v1->y+v2->y + opp->next->next->v->y)/3;
                fanPoint[2] = (v1->y+v2->y + opp->next->next->v->x)/3;
                // for (auto unnVert : uneccVertices){
                    
                // }
            }


        }
    }

}
