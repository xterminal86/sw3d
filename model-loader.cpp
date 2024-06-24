#include "model-loader.h"

#include <fstream>
#include <memory>

namespace SW3D
{
  void ModelLoader::ToTriangles(Scene::Object& obj)
  {
    obj.Triangles.clear();

    for (auto& face : obj.Faces)
    {
      Triangle t;
      for (size_t i = 0; i < 3; i++)
      {
        int32_t vertexInd  = face.Indices[i][0];
        int32_t textureInd = face.Indices[i][1];
        int32_t normalInd  = face.Indices[i][2];

        if (vertexInd != -1)
        {
          t.Points[i].Position = _scene.Vertices[vertexInd];
        }

        if (textureInd != -1)
        {
          t.Points[i].UV = _scene.UV[textureInd];
        }

        if (normalInd != -1)
        {
          t.Points[i].Normal = _scene.Normals[normalInd];
        }
      }

      obj.Triangles.push_back(t);
    }
  }

  // ===========================================================================

  StringV ModelLoader::StringSplit(const std::string& str, char delim)
  {
    StringV res;

    int start, end = -1;

    do
    {
      start = end + 1;
      end = str.find(delim, start);
      if (end != -1)
      {
        std::string word = str.substr(start, end - start);
        res.push_back(word);
      }
      else
      {
        if (start < (int)str.length())
        {
          res.push_back(str.substr(start, str.length()));
        }
        else
        {
          res.push_back("");
        }
      }
    } while (end != -1);

    return res;
  }

  // =============================================================================

  bool ModelLoader::Load(const std::string& fname)
  {
    _scene = Scene();

    std::ifstream f(fname);

    if (not f.is_open())
    {
      Error = EngineError::FAILED_TO_LOAD_MODEL;
      return false;
    }

    std::string line;

    size_t lineCount = 0;

    std::unique_ptr<Scene::Object> obj;

    while (std::getline(f, line))
    {
      lineCount++;

      //
      // Skip comments.
      //
      if (line.find_first_of('#') != std::string::npos
       or line.length() == 0)
      {
        continue;
      }

      ObjFileLineType type = ObjFileLineType::UNDEFINED;

      //
      // Not doing syntax check or anything because fuck it.
      //
      StringV spl = StringSplit(line, ' ');
      if (LineTypeByString.count(spl[0]) != 0)
      {
        type = LineTypeByString.at(spl[0]);
      }

      switch (type)
      {
        case ObjFileLineType::OBJECT:
        {
          //
          // If single object only, don't push it right away.
          //
          if (obj != nullptr)
          {
            _scene.Objects.push_back(*obj.get());
          }

          obj = std::make_unique<Scene::Object>();
          obj->Name = spl[1];
        }
        break;

        // -----------------------------------------------------------------------

        case ObjFileLineType::VERTEX:
        {
          SW3D::Vec3 v =
          {
            std::stod(spl[1]),
            std::stod(spl[2]),
            std::stod(spl[3])
          };

          _scene.Vertices.push_back(v);
        }
        break;

        // -----------------------------------------------------------------------

        case ObjFileLineType::VERTEX_NORMAL:
        {
          SW3D::Vec3 v =
          {
            std::stod(spl[1]),
            std::stod(spl[2]),
            std::stod(spl[3])
          };

          _scene.Normals.push_back(v);
        }
        break;

        // -----------------------------------------------------------------------

        case ObjFileLineType::FACE:
        {
          Scene::Object::Face f;

          for (size_t i = 0; i < 3; i++)
          {
            auto& faceData = spl[i + 1];
            auto sd = StringSplit(faceData, '/');
            for (size_t j = 0; j < sd.size(); j++)
            {
              //
              // Indices are 1 based.
              //
              f.Indices[i][j] = sd[j].empty() ? -1 : (std::stoi(sd[j]) - 1);
            }
          }

          obj->Faces.push_back(f);
        }
        break;

        // -----------------------------------------------------------------------

        default:
          break;
      }
    }

    //
    // In case of multiple objects we need not to forget to add last parsed
    // one.
    //
    if (obj != nullptr)
    {
      _scene.Objects.push_back(*obj.get());
    }

    f.close();

    for (Scene::Object& obj : _scene.Objects)
    {
      ToTriangles(obj);
    }

    return true;
  }

  // =============================================================================

  const ModelLoader::Scene& ModelLoader::GetScene()
  {
    return _scene;
  }
}
