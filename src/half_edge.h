#include "utils.h"
#include <iostream>
#include <math.h>
#include <map>
#include<bits/stdc++.h> 
#include<algorithm>
extern std::map<std::pair<int ,int>,struct halfedge *> dictionary_edges;
extern std::vector<p2t::Point*> points;
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
    bool boundary;
    int vNum;
    int samplePoints;
} vertex;

static void deleteFaces(std::vector<face*> &faces) {

    int totalFaces = faces.size();

    for(int i = 0; i < totalFaces; ++i)
    {
        delete faces[i]->e->next->next;
        delete faces[i]->e->next;
        delete faces[i]->e;
        delete faces[i];
    }
    faces.clear();
}

static vertex * makeHalfEdgeVertex(float x, float y, float z, int vertexNumber, bool boundary){
    vertex *v = new vertex;
    v->x = x; v->y = y; v->z = z;
    v->e = NULL;  v->vNum = vertexNumber;
    v->boundary=boundary;
    return v;
}

static void addFace(int i1,int i2,int i3, const std::vector<vertex *> &vertices, std::vector<face *> &prunedFaces){
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
    }    

    prunedFaces.push_back(f);

}

static void makeHalfEdgeFace(int i1,int i2,int i3, const std::vector<vertex *> &vertices, std::vector<face *> &faces){
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

    std::vector<int> indices;
    float x,y,z;
    float d;
    int indx;
    bool foundIndx;

    for (auto point: points){
        vertices.push_back(makeHalfEdgeVertex(point->x,point->y,0,vertices.size(),true));
    }

    for (auto triangle:triangles){
        for (int i =0;i<3;i++){
            x = triangle->GetPoint(i)->x;
            y = triangle->GetPoint(i)->y;
            foundIndx = false;
            indx= 0;
            for (auto point: points){
                d = sqrt(pow( x*x - point->x*point->x , 2 )+pow( y*y - point->y*point->y , 2 ));
                if (d<=0.00001){
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

static int indexOfVertex(float x, float y, std::vector<vertex *> *vertices){

    int indx=0;
    bool foundIndx=false;
    int centre;
    float d;
    for (auto point: *vertices){
        d = sqrt(pow( x*x  - point->x*point->x , 2 )+pow( y*y  - point->y*point->y , 2 ));
        if (d<=0.00001){
            foundIndx = true;
            break;
        }
        indx++;
    }
    if (!foundIndx){
        centre = vertices->size();
        vertices->push_back(makeHalfEdgeVertex(x,y,0.0,vertices->size(),false));
    }
    else{
        centre = indx;
    }

    return centre;

}

static bool outsideCircle(vertex *v3,float centerX,float centerY, float radius){
    return pow(v3->x-centerX,2)+pow(v3->y-centerY,2)>pow(radius,2);
}


static std::vector<face *> pruneTriangles(std::vector<vertex *> &vertices, std::vector<face *> &faces){
    edge *prevE;
    edge *e;
    edge *opp;
    int indx;
    float fanPoint[3];
    float d;
    int centre;
    bool outside=false;
    bool foundIndx =false;
    int cnt;
    vertex *v1;
    vertex *v2;
    vertex *v3;
    std::vector<face *> pruned_faces; 
    std::deque<vertex *> uneccVertices;
    for (auto face:faces){
        if (face->triangleType==2){ //terminal
            prevE = face->e; 
            e = face->e;
            uneccVertices.clear();
            outside=false;
            while (!outside){

                e = e->next;
                cnt =0;
                while (e->opposite==NULL && cnt<=3){
                    e = e->next;

                    cnt+=1;
                }

                face->visit=1;
                e->f->visit=1;
                v1 = e->v;
                v2 = e->next->v;
                v3 = e->next->next->v;
                if (e->f->triangleType ==0){
                    fanPoint[0] = (v1->x+v2->x + v3->x)/3;
                    fanPoint[1] = (v1->y+v2->y + v3->y)/3;
                    fanPoint[2] = 0.0;
                    centre = indexOfVertex(fanPoint[0],fanPoint[1],&vertices);
                    break;
                }


                if (count(uneccVertices.begin(), uneccVertices.end(), v3) == 0)
                    uneccVertices.push_front(v3);

                float centerX = (v1->x+v2->x)/2.0;
                float centerY = (v1->y+v2->y)/2.0;
                float radius = sqrt(pow(v1->x-v2->x,2) + pow(v1->y-v2->y,2))/2.0;
                for (int i=1;i<=uneccVertices.size()-1;i++){
                    vertex * v = uneccVertices.at(i);
                    if (outsideCircle(v,centerX,centerY,radius)){
                        centre = vertices.size();
                        vertices.push_back(makeHalfEdgeVertex(centerX,centerY,0.0,vertices.size(),false));
                        outside = true;
                        break;
                    }
                }
                if (count(uneccVertices.begin(), uneccVertices.end(), v2) == 0)
                    uneccVertices.push_front(v2);

                if (count(uneccVertices.begin(), uneccVertices.end(), v1) == 0)
                    uneccVertices.push_back(v1);
                e = e->opposite;

                
            }

            if (uneccVertices.size() !=0){
                for (int i=0;i<uneccVertices.size()-1;i++){
                    makeHalfEdgeFace(uneccVertices.at(i)->vNum,uneccVertices.at(i+1)->vNum,centre,vertices, pruned_faces);
                }
            }
        }   
    }

    for (auto face: faces){
        if (face->triangleType==0){
            int centroid;            
            e = face->e;
            for (int i =0;i<3;i++){
                fanPoint[0] = (e->v->x + e->next->v->x)/2.0;
                fanPoint[1] = (e->v->y + e->next->v->y)/2.0;
                fanPoint[2] = 0.0;
                foundIndx = false;
                v1 = e->v;
                v2 = e->next->v;
                v3 = e->next->next->v;
                for (auto point: vertices){
                    d = sqrt(pow( fanPoint[0]*fanPoint[0]  - point->x*point->x , 2 )+pow( fanPoint[1]*fanPoint[1]  - point->y*point->y , 2 ));
                    if (d<=0.00001){
                        foundIndx = true;
                        break;
                    }
                }
                if(e->opposite!=NULL && (e->opposite->f->triangleType==0 || e->opposite->f->visit==0 || foundIndx)){

                    fanPoint[0] = (v1->x + v2->x + v3->x)/3.0;
                    fanPoint[1] = (v1->y + v2->y + v3->y)/3.0;
                    fanPoint[2] = 0.0;
                    centroid = indexOfVertex(fanPoint[0],fanPoint[1],&vertices);
                    centre = indexOfVertex((e->v->x+e->next->v->x)/2.0, (e->v->y+e->next->v->y)/2.0,&vertices);
                    makeHalfEdgeFace(e->v->vNum,centre,centroid,vertices,pruned_faces);
                    makeHalfEdgeFace(centre,e->next->v->vNum,centroid,vertices,pruned_faces);
                }
                e = e->next;
            }
        }

        else if (face->visit==0){
            e = face->e;
            e = e->next;
            cnt =0;
            while (e->opposite!=NULL && cnt<=5){
                e = e->next;
                cnt +=1;
            }
            face->visit=1;
            e->f->visit=1;
            v1 = e->v;
            v2 = e->next->v;
            v3 = e->next->next->v;
            int center1 = indexOfVertex((v2->x+v3->x)/2.0,(v2->y+v3->y)/2.0,&vertices);
            int center2 = indexOfVertex((v1->x+v3->x)/2.0,(v1->y+v3->y)/2.0,&vertices);
            makeHalfEdgeFace(v1->vNum,v2->vNum,center1,vertices, pruned_faces);
            makeHalfEdgeFace(v1->vNum,center1,center2,vertices, pruned_faces);
            makeHalfEdgeFace(center1,v3->vNum,center2, vertices, pruned_faces);
        }
    }
    deleteFaces(faces);
    return pruned_faces;
}

static void lengthElevate(edge **hEdge, float &len, int &n){
    float dir[2];
    if((*hEdge)->next->v->boundary){
        dir[0]=(*hEdge)->v->x - (*hEdge)->next->v->x;
        dir[1]=(*hEdge)->v->y - (*hEdge)->next->v->y;
        len+=sqrt(dir[0]*dir[0]+dir[1]*dir[1]);
        n+=1;
    }
    (*hEdge)=(*hEdge)->opposite->next;
}

static void samplePointsLengthElevate(edge **hEdge, int samplePoints, std::vector<vertex *> &newPts, std::map<std::pair<int, int>, std::vector<int>> *samplePtsPerEdge){
    vertex *v1, *v2;
    std::vector<int> sampleIndices;
    if(((*hEdge)->v->boundary && !(*hEdge)->next->v->boundary) || (!(*hEdge)->v->boundary && (*hEdge)->next->v->boundary)){
        if(!(*hEdge)->v->boundary){
            v1=(*hEdge)->v;
            v2=(*hEdge)->next->v;
        }
        else{
            v2=(*hEdge)->v;
            v1=(*hEdge)->next->v;
        }
        glm::vec3 ellipsoid_axis(v2->x-v1->x, v2->y-v1->y, 0.0);
        float a= ellipsoid_axis.length();
        float b= v1->z;
        float t = 1.0;
        for(int i=1;i<samplePoints+1;i++){
            t=((float)i/(float)(samplePoints+1));
            vertex *v = new vertex;
            v->x = v1->x + t*ellipsoid_axis[0];
            v->y = v1->y + t*ellipsoid_axis[1];
            v->z = b*sqrt(1-(t*t));
            v->vNum = newPts.size();
            newPts.push_back(v);
            sampleIndices.push_back(newPts.size()-1);    
        }
        std::pair<int,int> p;
        p.first = std::min(v1->vNum, v2->vNum);
        p.second = std::max(v1->vNum, v2->vNum);
        (*samplePtsPerEdge)[p]=sampleIndices;
    }
    (*hEdge)=(*hEdge)->next;
}

static std::vector<face *> erection(std::vector<vertex *> &vertices, std::vector<face *> &faces){
    
    int samplePoints=0;
    std::vector<face *> erected_faces; 
    edge *hEdge;
    
    for(auto v:vertices){
        if(!v->boundary && v->e->opposite!=NULL){
            int n=0;
            float len=0.0;
            int cnt = 0;
            hEdge=v->e;
            bool x = hEdge==v->e;
            lengthElevate(&hEdge, len, n);
            while(hEdge->opposite!=NULL && hEdge!=v->e){
                lengthElevate(&hEdge, len, n);
                cnt ++;
            }
            v->z=len/n;
            samplePoints=int(len/n/0.15);
        }
    }

    std::vector<vertex *> newPts(vertices);

    std::map<std::pair<int, int>, std::vector<int>> samplePtsPerEdge;

    for(auto face:faces){
        hEdge=face->e;
        vertex *v1, *v2;
        std::vector<int> sampleIndices;
        if(hEdge->v->boundary && !(hEdge->next->v->boundary) || (!hEdge->v->boundary) && (hEdge->next->v->boundary)){
            if(!(hEdge)->v->boundary){
                v1=(hEdge)->v;
                v2=(hEdge)->next->v;
            }
            else{
                v2=(hEdge)->v;
                v1=(hEdge)->next->v;
            }
            glm::vec3 ellipsoid_axis(v2->x-v1->x, v2->y-v1->y, 0.0);
            float a= ellipsoid_axis.length();
            float b= v1->z;
            float t = 1.0;
            for(int i=1;i<samplePoints+1;i++){
                t=((float)i/(float)(samplePoints+1));
                vertex *v = new vertex;
                v->x = v1->x + t*ellipsoid_axis[0];
                v->y = v1->y + t*ellipsoid_axis[1];
                v->z = b*sqrt(1-(t*t));
                v->vNum = newPts.size();
                newPts.push_back(v);
                sampleIndices.push_back(newPts.size()-1);    
            }
            std::pair<int,int> p;
            p.first = std::min(v1->vNum, v2->vNum);
            p.second = std::max(v1->vNum, v2->vNum);
            samplePtsPerEdge[p]=sampleIndices;
        }
        hEdge=hEdge->next;

        int count=0;
        while(hEdge!=face->e && count++<3){
            sampleIndices.clear();
            if(hEdge->v->boundary && !(hEdge->next->v->boundary) || (!hEdge->v->boundary) && (hEdge->next->v->boundary)){
                if(!(hEdge)->v->boundary){
                    v1=(hEdge)->v;
                    v2=(hEdge)->next->v;
                }
                else{
                    v2=(hEdge)->v;
                    v1=(hEdge)->next->v;
                }
                glm::vec3 ellipsoid_axis(v2->x-v1->x, v2->y-v1->y, 0.0);
                float a= ellipsoid_axis.length();
                float b= v1->z;
                float t = 1.0;
                for(int i=1;i<samplePoints+1;i++){
                    t=((float)i/(float)(samplePoints+1));
                    vertex *v = new vertex;
                    v->x = v1->x + t*ellipsoid_axis[0];
                    v->y = v1->y + t*ellipsoid_axis[1];
                    v->z = b*sqrt(1-(t*t));
                    v->vNum = newPts.size();
                    newPts.push_back(v);
                    sampleIndices.push_back(newPts.size()-1);    
                }
                std::pair<int,int> p;
                p.first = std::min(v1->vNum, v2->vNum);
                p.second = std::max(v1->vNum, v2->vNum);
                samplePtsPerEdge[p]=sampleIndices;
            }
            hEdge=hEdge->next;
            
        }
    }

    int cnt=0;
    std::vector<int> edge1SamplePts;
    std::vector<int> edge2SamplePts;
    
    // for(auto face:faces){
    //     hEdge=face->e;
    //     std::cout<<"NEW POINTS KA SIZE: "<<newPts.size()<<"     "<<vertices.size()<<std::endl;
    //     // cnt=0;
    //     hEdge=hEdge->next;
    //     while(hEdge!=face->e){
    //         std::cout<<"amistuckhere\n";
    //         if((hEdge->v->boundary && hEdge->next->v->boundary) || (!hEdge->v->boundary && !hEdge->next->v->boundary))
    //             break;
    //         hEdge=hEdge->next;
    //     }
    //     std::cout<<"cool\n";
    //     std::pair<int,int> p;
    //     p.first = std::min(hEdge->next->v->vNum, hEdge->next->next->v->vNum);
    //     p.second = std::max(hEdge->next->v->vNum, hEdge->next->next->v->vNum);
    //     std::cout<<"p first: "<<p.first<<" p second: "<<p.second<<" size of map: "<< samplePtsPerEdge.size()<<"\n";
    //     std::cout<<"how?\n";
    //     std::cout<<"   size: "<<samplePtsPerEdge[p].size()<<"\n";
    //     edge1SamplePts = samplePtsPerEdge.at(p);
    //     std::cout<<"doggo\n";
    //     std::pair<int,int> p1;
    //     p1.first = std::min(hEdge->next->next->v->vNum, hEdge->next->next->next->v->vNum);
    //     p1.second = std::max(hEdge->next->next->v->vNum, hEdge->next->next->next->v->vNum);
    //     std::cout<<"p1 first: "<<p1.first<<" p1 second: "<<p1.second<<" size of map: "<< samplePtsPerEdge.size()<<"\n";
    //     std::cout<<"how?\n";
    //     std::cout<<"   size: "<<samplePtsPerEdge[p1].size()<<"\n";
    //     edge2SamplePts = samplePtsPerEdge.at(p1);
    //     std::cout<<"how again?\n";
    //     // for (int indx: edge2SamplePts){
    //     //     vertex *v = newPts.at(indx);
    //     //     std::cout<<"X: "<<v->x<<"  Y:   "<< v->y<<"  V->Boundary:  "<<v->boundary<<" OPPOSiTE: "<<v->e->opposite<<"   VNUM: "<<v->vNum<<" "<<std::endl;
    //     // }
    //     std::cout<<"catto\n";
    //     if(hEdge->v->boundary && hEdge->next->v->boundary){
    //         std::reverse(edge1SamplePts.begin(), edge1SamplePts.end());
    //         std::reverse(edge2SamplePts.begin(), edge2SamplePts.end());
    //     }

    //     std::cout<<"edge1 Sample point: "<<edge1SamplePts[0]<<"\thedgeV:"<<hEdge->v->vNum<<"\tedge2 Sample pt:"<<edge2SamplePts[0]<<"\t hEdge Next ka v:"<<hEdge->next->v->vNum<<"\n";

    //     makeHalfEdgeFace(hEdge->v->vNum, hEdge->next->v->vNum, edge1SamplePts[0], newPts, erected_faces);

    //     makeHalfEdgeFace(edge2SamplePts[0], edge1SamplePts[0], hEdge->next->v->vNum,newPts, erected_faces);

    //     int i=0;
    //     while(i<samplePoints-1){

            
    //         std::cout<<"pls check\n";
    //         makeHalfEdgeFace(edge1SamplePts[i], edge2SamplePts[i], edge1SamplePts[i+1], newPts, erected_faces);

    //         makeHalfEdgeFace(edge2SamplePts[i], edge2SamplePts[i+1], edge1SamplePts[i+1], newPts, erected_faces);

    //         i++;
    //     }

    //     makeHalfEdgeFace(edge1SamplePts[i], hEdge->next->next->v->vNum, edge2SamplePts[i], newPts, erected_faces);

    // }

    for(auto face:faces){
        hEdge=face->e;
        
        cnt=0;
        hEdge=hEdge->next;

        while(hEdge!=face->e && cnt++<3){
            if((hEdge->v->boundary && hEdge->next->v->boundary) || (!hEdge->v->boundary && !hEdge->next->v->boundary))
                break;
            hEdge=hEdge->next;
        }

        std::pair<int,int> p;
        p.first = std::min(hEdge->next->v->vNum, hEdge->next->next->v->vNum);
        p.second = std::max(hEdge->next->v->vNum, hEdge->next->next->v->vNum);

        std::vector<int> e1SamplePts(samplePtsPerEdge.at(p));

        std::pair<int,int> p1;
        p1.first = std::min(hEdge->next->next->v->vNum, hEdge->next->next->next->v->vNum);
        p1.second = std::max(hEdge->next->next->v->vNum, hEdge->next->next->next->v->vNum);

        std::vector<int> e2SamplePts(samplePtsPerEdge.at(p1));

        if(hEdge->v->boundary && hEdge->next->v->boundary){
            std::reverse(e1SamplePts.begin(), e1SamplePts.end());
            std::reverse(e2SamplePts.begin(), e2SamplePts.end());
        }

        addFace(hEdge->v->vNum, hEdge->next->v->vNum, e1SamplePts[0], newPts, erected_faces);

        addFace(e2SamplePts[0], e1SamplePts[0], hEdge->next->v->vNum,newPts, erected_faces);

        int ppts=0;
        while(ppts<samplePoints-1){

            addFace(e1SamplePts[ppts], e2SamplePts[ppts], e1SamplePts[ppts+1], newPts, erected_faces);

            addFace(e2SamplePts[ppts], e2SamplePts[ppts+1], e1SamplePts[ppts+1], newPts, erected_faces);

            ppts++;
        }

        addFace(e1SamplePts[ppts], hEdge->next->next->v->vNum, e2SamplePts[ppts], newPts, erected_faces);

    }

    std::cout<<"Elevation Done\n";
    deleteFaces(faces);
    return erected_faces;

}