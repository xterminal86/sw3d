#ifndef PITRASTERIZERTLR_H
#define PITRASTERIZERTLR_H

#include <SDL2/SDL.h>

#include "sw3d.h"
#include "types.h"

using namespace SW3D;

class PitRasterizerTLR
{
  public:
    enum class FillConvention
    {
      NONE = 0,
      TOP_LEFT,
      BOTTOM_RIGHT,
      TOP_RIGHT,
      BOTTOM_LEFT
    };

    void Init(SDL_Renderer* rendererRef);
    void Rasterize(const TriangleSimple& t, bool wireframe = false);
    void SetFillConvention(FillConvention c);

  private:
    void SortVertices();
    void CheckAndFixWinding();

    bool IsTopLeft(const SDL_Point& start, const SDL_Point& end);
    bool IsBottomRight(const SDL_Point& start, const SDL_Point& end);
    bool IsTopRight(const SDL_Point& start, const SDL_Point& end);
    bool IsBottomLeft(const SDL_Point& start, const SDL_Point& end);

    SDL_Renderer* _renderer = nullptr;

    bool _initialized = false;

    FillConvention _fillConvention = FillConvention::TOP_LEFT;

    TriangleSimple _tmp;
};

#endif // PITRASTERIZERTLR_H
