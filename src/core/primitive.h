#pragma once

#include <shape.h>

class BSDF;
class SurfaceInteraction;
class Material;
class Pixel;
class PhaseFunction;

class Primitive{
    public:
    Bounds3f worldBound;
    Shape *shape;
    __bidevice__ Primitive(Shape *shape);
    __bidevice__ virtual bool Intersect(const Ray &r, SurfaceInteraction *) const;
    __bidevice__ virtual void Release() const{ delete shape; }
    __bidevice__ virtual void ComputeScatteringFunctions(BSDF *bsdf, SurfaceInteraction *si,
                                                         TransportMode mode, bool mLobes) 
        const = 0;
    __bidevice__ virtual void PrintSelf() const = 0;
};

class GeometricPrimitive : public Primitive{
    public:
    Material *material;
    
    __bidevice__ GeometricPrimitive() : Primitive(nullptr){}
    __bidevice__ GeometricPrimitive(Shape *shape, Material *material);
    
    __bidevice__ virtual 
        void ComputeScatteringFunctions(BSDF *bsdf, SurfaceInteraction *si,
                                        TransportMode mode, bool mLobes) const override;
    
    __bidevice__ virtual void PrintSelf() const override{
        printf("Geometric ( ");
        PrintShape(shape);
        printf(" ) \n");
    }
};

class Aggregator{
    public:
    Primitive **primitives;
    int length;
    int head;
    Node *root;
    PrimitiveHandle *handles;
    int totalNodes;
    
    Mesh **meshPtrs;
    int nAllowedMeshes;
    int nMeshes;
    
    __bidevice__ Aggregator();
    __bidevice__ void Reserve(int size);
    __bidevice__ void Insert(Primitive *pri);
    __bidevice__ bool Intersect(const Ray &r, SurfaceInteraction *, Pixel *) const;
    __bidevice__ void Release();
    __bidevice__ void PrintHandle(int which=-1);
    __bidevice__ Mesh *AddMesh(const Transform &toWorld, int nTris, int *_indices,
                               int nVerts, Point3f *P, vec3f *S, Normal3f *N, 
                               Point2f *UV);
    __host__ void ReserveMeshes(int n);
    __host__ void Wrap();
    
    private:
    __bidevice__ bool IntersectNode(Node *node, const Ray &r, SurfaceInteraction *) const;
};