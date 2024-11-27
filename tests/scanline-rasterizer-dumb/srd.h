#ifndef SRD_H
#define SRD_H

#include "sw3d.h"
#include "types.h"
#include "blg.h"

using namespace SW3D;

//
// "Scanline Rasterizer Dumb"
//
class SRD
{
  public:
    void Init(SDL_Renderer* rendererRef);
    void Rasterize(const TriangleSimple& t, bool wireframe = false);

  private:
    void RasterizeImpl();

    void SortVertices();
    void CheckAndFixWinding();

    WindingOrder GetWindingOrder(const TriangleSimple& t);
    TriangleType GetTriangleType(const TriangleSimple& t);

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

    std::unordered_map<int, int> _leftLineXByScanline;
    std::unordered_map<int, int> _rightLineXByScanline;

    BLG _lineGen;
};

#endif // SRD_H
