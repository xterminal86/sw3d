#ifndef SCANLINERASTERIZER_H
#define SCANLINERASTERIZER_H

#include <SDL2/SDL.h>

#include "sw3d.h"
#include "types.h"
#include "blg.h"

using namespace SW3D;

//
// [scanline-rasterizer] code put into convenience class.
//
class ScanlineRasterizer
{
  public:
    void Init(SDL_Renderer* rendererRef);

    WindingOrder GetWindingOrder(const TriangleSimple& t);
    TriangleType GetTriangleType(const TriangleSimple& t);

    void Rasterize(const TriangleSimple& t, bool wireframe = false);

    void SortVertices(TriangleSimple& t);
    void CheckAndFixWinding(TriangleSimple& t);

  protected:
    void DrawVL();
    void DrawHL();
    void DrawFT(const TriangleSimple& t);
    void DrawFB(const TriangleSimple& t);
    void DrawMR(const TriangleSimple& t);
    void DrawML(const TriangleSimple& t);

    virtual void PerformRasterization(BLG& first,
                                      BLG& second,
                                      const TriangleSimple& t,
                                      TriangleType tt);

    virtual void PerformRasterizationWireframe(BLG& first,
                                               BLG& second,
                                               const TriangleSimple& t,
                                               TriangleType tt);

    SDL_Renderer* _renderer = nullptr;

    TriangleSimple _copy;

    bool _drawWireframe = false;
    bool _initialized   = false;

    BLG _first;
    BLG _second;
};

#endif // SCANLINERASTERIZER_H
