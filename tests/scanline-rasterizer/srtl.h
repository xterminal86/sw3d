#ifndef SRTL_H
#define SRTL_H

#include "scanline-rasterizer.h"

//
// "Scanline Rasterizer Top Left"
//
class SRTL final : public ScanlineRasterizer
{
  protected:
    void PerformRasterization(BLG& first,
                              BLG& second,
                              const TriangleSimple& t,
                              TriangleType tt) override;

    void PerformRasterizationWireframe(BLG& first,
                                       BLG& second,
                                       const TriangleSimple& t,
                                       TriangleType tt) override;
};

#endif // SRTL_H
