#ifndef PITRASTERIZER_H
#define PITRASTERIZER_H

#include <SDL2/SDL.h>

#include "sw3d.h"
#include "types.h"

using namespace SW3D;

class PitRasterizerTLR
{
  public:
    void Init(SDL_Renderer* rendererRef);
    void Rasterize(const TriangleSimple& t, bool wireframe = false);

  private:
    void SortVertices();
    void CheckAndFixWinding();

    bool IsTopLeft(const SDL_Point& start, const SDL_Point& end);

    SDL_Renderer* _renderer = nullptr;

    bool _initialized = false;

    TriangleSimple _tmp;
};

#endif // PITRASTERIZER_H
