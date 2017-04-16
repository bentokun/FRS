#pragma once

#include "Rules.h"
#include "Device.h"
#include "SwapChain.h"
#include "GraphicPipeline.h"

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	class TFSAPI Framebuffers {

	public:
		
		Framebuffers() {};

		Framebuffers(Swapchain chain, 
			GraphicPipeline pipe, 
			Device device);

		~Framebuffers() {};

		friend void DestroyFramebuffer(Framebuffers* buffer);

		std::vector<VkFramebuffer> frameBuffer;

	private:

		Device device;

	};

}