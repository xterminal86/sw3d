#include "model-loader.h"

#include <fstream>

namespace SW3D
{
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
    }
    while (end != -1);

    return res;
  }

  // =============================================================================

  ModelLoader::Model* ModelLoader::Load(const std::string& fname)
  {
    std::ifstream f(fname);

    if (not f.is_open())
    {
      Error = EngineError::FAILED_TO_LOAD_MODEL;
      return nullptr;
    }

    _model = Model();

    std::string line;

    while (std::getline(f, line))
    {
      // skip comments
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
          _model.Name = spl[1];
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

          _model.Vertices.push_back(v);
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

          _model.Normals.push_back(v);
        }
        break;

        // -----------------------------------------------------------------------

        case ObjFileLineType::FACE:
        {
          Model::Face f;

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

          _model.Polygons++;

          _model.Faces.push_back(f);
        }
        break;

        // -----------------------------------------------------------------------

        default:
          break;
      }
    }

    f.close();

    return &_model;
  }
}
