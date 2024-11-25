#ifndef SRTLCHILI_H
#define SRTLCHILI_H

#include "sw3d.h"
#include "types.h"

using namespace SW3D;

//
// "Scanline Rasterizer Top Left using DDA line drawing"
//
class SRTLCHILI
{
  public:
    void Init(SDL_Renderer* rendererRef);
    void Rasterize(const TriangleSimple& t, bool wireframe = false);

  protected:
    void PerformRasterization();

    void SortVertices();
    void CheckAndFixWinding();

    WindingOrder GetWindingOrder(const TriangleSimple& t);
    TriangleType GetTriangleType(const TriangleSimple& t);

  private:
    void DrawFT(const TriangleSimple& t);
    void DrawFB(const TriangleSimple& t);
    void DrawMR(const TriangleSimple& t);
    void DrawML(const TriangleSimple& t);
    void DrawVL();
    void DrawHL();

    TriangleSimple _copy;

    SDL_Renderer* _renderer = nullptr;

    bool _initialized = false;
    bool _wireframe   = false;
};

#endif // SRTLCHILI_H
