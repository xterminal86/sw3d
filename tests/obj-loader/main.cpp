#include <cstdio>
#include <fstream>

#include "model-loader.h"
#include "sw3d.h"

const std::string kCubeFilename         = "models/cube.obj";
const std::string kCubeTexturedFilename = "models/cube-textured.obj";

// =============================================================================

int main(int argc, char* argv[])
{
  SW3D::ModelLoader loader;

  SW3D::ModelLoader::Model* m = loader.Load(kCubeFilename);

  if (m != nullptr)
  {
    printf("%s\n", SW3D::ToString(*m).data());
  }
  else
  {
    printf("%s\n", SW3D::ErrorToString());
  }

  return 0;
}
