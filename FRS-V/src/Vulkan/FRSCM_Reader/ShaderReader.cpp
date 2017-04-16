#include "ShaderReader.h"

namespace FRS {
	Shader ReadModuleShader(
		Device device, std::string path, std::string path2) {

		std::vector<char> buffer1, buffer2;


		try {

			std::ifstream vertexReader(path, std::ios::ate|std::ios::binary);
	
			std::ifstream fragmentReader(path2, std::ios::ate | std::ios::binary);

			size_t size1 = vertexReader.tellg();
			size_t size2 = fragmentReader.tellg();

			buffer1.resize(size1);
			buffer2.resize(size2);

			vertexReader.seekg(0);
			vertexReader.read(buffer1.data(), size1);

			vertexReader.close();

			fragmentReader.seekg(0);
			fragmentReader.read(buffer2.data(), size2);
		
			fragmentReader.close();
		
		}
		catch (std::ios::failure e)
		{
			std::cout << "Can't load the file" << std::endl;
		}

		return Shader{ device, buffer1, buffer2, path, path2 };

	}
}