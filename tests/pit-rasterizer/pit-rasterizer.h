#ifndef PITRASTERIZER_H
#define PITRASTERIZER_H

#include <SDL2/SDL.h>

#include "types.h"

using namespace SW3D;

class PitRasterizer
{
  public:
    void Init(SDL_Renderer* rendererRef);
    void Rasterize(const TriangleSimple& t, bool wireframe = false);

  private:
    SDL_Renderer* _renderer = nullptr;

    bool _initialized = false;
};

#endif // PITRASTERIZER_H
