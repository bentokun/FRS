#include "ContentManager.h"

namespace FRS {

	ContentManager::ContentManager(Device device,
		DeviceAllocator* allocator)
		:device(device),
		allocator(allocator){

	}

	void CreateContentManager(ContentManager* manager, Device device,
		DeviceAllocator* allocator) {
		manager->device = device;
		manager->allocator = allocator;
	}
}