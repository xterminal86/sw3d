#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <string>
#include <vector>
#include <unordered_map>

#include "types.h"

using StringV = std::vector<std::string>;

namespace SW3D
{
  class ModelLoader
  {
    public:
      struct Scene
      {
        struct Object
        {
          struct Face
          {
            //
            //   0 1 2
            // 0 v t n
            // 1 v t n
            // 2 v t n
            //
            // -1 means coordinate index is not defined
            // (e.g. no texture coordinates specified).
            // Actual index value is 1 based and continuous across several objects
            // in one .obj  file.
            //
            int32_t Indices[3][3] =
            {
              { -1, -1, -1 },
              { -1, -1, -1 },
              { -1, -1, -1 }
            };
          };

          std::string       Name;
          std::vector<Face> Faces;
        };

        std::vector<Object> Objects;

        std::vector<SW3D::Vec3> Vertices;
        std::vector<SW3D::Vec3> Normals;
        std::vector<SW3D::Vec2> UV;
      };

      bool Load(const std::string& fname);

      const Scene& GetScene();

    private:
      enum class ObjFileLineType
      {
        UNDEFINED = 0,
        OBJECT,
        VERTEX,
        VERTEX_NORMAL,
        VERTEX_TEXTURE,
        SHADING,
        FACE
      };

      const std::unordered_map<std::string, ObjFileLineType> LineTypeByString =
      {
        { "o",  ObjFileLineType::OBJECT         },
        { "v",  ObjFileLineType::VERTEX         },
        { "vn", ObjFileLineType::VERTEX_NORMAL  },
        { "vt", ObjFileLineType::VERTEX_TEXTURE },
        { "s",  ObjFileLineType::SHADING        },
        { "f",  ObjFileLineType::FACE           }
      };

      StringV StringSplit(const std::string& str, char delim);

      Scene _scene;
  };
}

#endif // MODELLOADER_H
