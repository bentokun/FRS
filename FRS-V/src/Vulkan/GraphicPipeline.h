#pragma once

#include "Rules.h"
#include "Shader.h"
#include "Device.h"
#include "SwapChain.h"
#include "Buffer.h"

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	class TFSAPI GraphicPipeline {
	public:

		GraphicPipeline() {};

		friend void CreateGraphicPipeline(GraphicPipeline* pipe,
			Device device, 
			Swapchain swapChain,
			Shader shader,
			std::vector<Buffer> &buffers);

		friend void DestroyGraphicPipeline(
			GraphicPipeline pipe);
		
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout mLayout = VK_NULL_HANDLE;
		VkRenderPass mRenderPass = VK_NULL_HANDLE;

		std::vector<Buffer>& GetBuffers() {
			return buffers;
		}

	private:
		
		std::vector<Buffer> buffers;
		Device device;

	};

}