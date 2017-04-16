#pragma once

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "../Shader.h"
#include "../Rules.h"


#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	Shader TFSAPI ReadModuleShader(Device device, std::string path, std::string path2);

}
