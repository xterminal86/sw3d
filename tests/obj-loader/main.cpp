#include <cstdio>
#include <fstream>

#include "model-loader.h"
#include "sw3d.h"

const std::string kCubeFilename         = "models/cube.obj";
const std::string kCubeTexturedFilename = "models/cube-textured.obj";
const std::string kTwoObjsFilename      = "models/two.obj";

const std::string kDecor(80, '=');

// =============================================================================

int main(int argc, char* argv[])
{
  {
    SW3D::ModelLoader loader;

    bool ok = loader.Load(kCubeFilename);

    if (ok)
    {
      printf("%s\n", SW3D::ToString(loader.GetScene()).data());
    }
    else
    {
      printf("%s\n", SW3D::ErrorToString());
    }
  }
  // ---------------------------------------------------------------------------
  printf("%s\n", kDecor.data());
  // ---------------------------------------------------------------------------
  {
    SW3D::ModelLoader loader;

    bool ok = loader.Load(kTwoObjsFilename);

    if (ok)
    {
      printf("%s\n", SW3D::ToString(loader.GetScene()).data());
    }
    else
    {
      printf("%s\n", SW3D::ErrorToString());
    }
  }

  return 0;
}
