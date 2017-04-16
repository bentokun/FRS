#include "Framebuffer.h"

namespace FRS {
	
	Framebuffers::Framebuffers(Swapchain chain,
		GraphicPipeline pipe,
		Device device) {

		this->device = device;
		this->frameBuffer.resize(chain.images.size(),
		{ VK_NULL_HANDLE });

		for (uint32_t i = 0; i < chain.imageViews.size(); i++) {
			VkFramebufferCreateInfo frameBufferInfo = {};

			frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferInfo.renderPass = pipe.mRenderPass;
			frameBufferInfo.width = chain.swapChainExtent.width;
			frameBufferInfo.height = chain.swapChainExtent.height;
			frameBufferInfo.attachmentCount = 1;
			frameBufferInfo.pAttachments = &(chain.imageViews[i]);
			frameBufferInfo.layers = 1;

			if (vkCreateFramebuffer(device.logicalDevice, &frameBufferInfo, nullptr, &(frameBuffer[i])) != VK_SUCCESS) {
				FRS_FATAL_ERROR("Cant create frame buffer");
			}

		}

	}

	void DestroyFramebuffer(Framebuffers* buffers) {

		for (auto& frame : buffers->frameBuffer) {
			vkDestroyFramebuffer(buffers->device.logicalDevice, frame, nullptr);
		}

	}

}