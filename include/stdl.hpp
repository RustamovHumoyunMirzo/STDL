#pragma once
#include "scene.hpp"
#include <memory>
#include <string>

namespace STDL {

using ScenePtr = std::shared_ptr<Scene>;

ScenePtr LoadFile(const std::string& path);

ScenePtr LoadString(const std::string& content);

bool SaveFile(const ScenePtr& scene, const std::string& path);

std::string ToString(const ScenePtr& scene);

}