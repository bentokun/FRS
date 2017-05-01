#include "Buffer.h"

namespace FRS {

	bool Block::operator==(Block const &block) {

		if (memory == block.memory &&
			offSet == block.offSet &&
			size == block.size)
		return true;

		return false;

	}

	Chunk::Chunk(Device& device, VkDeviceSize size, int memType) :
		mDevice(device),
		mSize(size),
		mMemoryTypeIndex(memType) {

		VkMemoryAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		info.allocationSize = mSize;
		info.memoryTypeIndex = mMemoryTypeIndex;

		Block block;
		block.free = true;
		block.offSet = 0;
		block.size = 0;
		
		VkResult result = vkAllocateMemory(mDevice.logicalDevice, &info,
			nullptr, &mMem);

		if (result != VK_SUCCESS) {
			throw std::runtime_error("Cant allocate memory by Vulkan, breaking bug");
		}

		block.memory = mMem;

		VkPhysicalDeviceMemoryProperties properties;
		vkGetPhysicalDeviceMemoryProperties(mDevice.physicalDevice, &properties);

		if ((properties.memoryTypes[mMemoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			vkMapMemory(mDevice.logicalDevice, block.memory, block.offSet, VK_WHOLE_SIZE, 0 , &dataPointer);
			vkUnmapMemory(mDevice.logicalDevice, block.memory);
		}

		mBlocks.emplace_back(block);
	}

	void Chunk::Deallocated(Block const &block) {

		auto findABlock{ std::find(mBlocks.begin(), mBlocks.end(), block) };
		FRS_S_ASSERT(findABlock != mBlocks.end());
		findABlock->free = true;
	}

	bool Chunk::Allocate(VkDeviceSize size, VkDeviceSize align, Block& block) {
		if (size > mSize) {
			std::cout << "Cant allocate!" << std::endl;
			return false;
		}

		for (auto mBlock : mBlocks) {
			if (mBlock.free) {
				uint32_t newSize = mBlock.size;

				//Align the block size
				//Remove the align part and add up the module 

				if (mBlock.offSet % align != 0)
					newSize -= align - mBlock.offSet % align;

				if (newSize >= size) {
					mBlock.size = newSize;
					if (mBlock.offSet % align != 0)
						mBlock.offSet += align - mBlock.offSet % align;
				}

				if (dataPointer != nullptr)
					//Point to the offset
					mBlock.ptr = (char*)dataPointer + block.offSet;
				}

				if (mBlock.size == size) {
					mBlock.free = false;
					block = mBlock;
					return true;
				}

				//If the size allocate match then exit, else
				//Allocate it to next block
				Block next;
				next.free = true;
				next.offSet = mBlock.offSet + size;
				next.memory = mMem;
				next.size = mBlock.size - size;
				mBlocks.emplace_back(next);


				mBlock.size = size;
				mBlock.free = false;

				block = mBlock;

		
				return true;
			
		}

		return false;
	}

	void DestroyChunk(Chunk* chunk) {
		vkFreeMemory(chunk->mDevice.logicalDevice,
			chunk->mMem,
			nullptr);
	}

	bool Chunk::HasAllocated(Block const &block) {
		
		auto find = std::find(mBlocks.begin(), mBlocks.end(), block);

		if (find != mBlocks.end())
			return true;

		return false;
	}

	Counter::Counter(Counter const &counter) : count(counter.count) {
		++(*count);
	}

	Counter &Counter::operator =(Counter counter) {
		std::swap(count, counter.count);
		return *this;
	}

	uint32_t Counter::getCount() const {
		return *(count);
	}

	Resource::Resource(Device const &device) : mDevice((device))
	{};

	
	ChunkAllocator::ChunkAllocator(Device &device, VkDeviceSize size):
		mDevice(device),
		mSize(size)
	{

		FRS_MESSAGE(mSize);
	};


	VkDeviceSize nextPowerOfTwo(VkDeviceSize size) {
		VkDeviceSize power = (VkDeviceSize)std::log2l(size) + 1;
		return (VkDeviceSize)1 << power;
	}

	bool isPowerOfTwo(VkDeviceSize size) {
		VkDeviceSize mask = 0;
		VkDeviceSize power = (VkDeviceSize)std::log2l(size);

		for (VkDeviceSize i = 0; i < power; ++i)
			mask += (VkDeviceSize)1 << i;

		return !(size & mask);
	}


	Chunk ChunkAllocator::allocate(VkDeviceSize size,
		int memType) {
		
		size = (size > mSize) ? nextPowerOfTwo(size) : mSize;

		return Chunk(mDevice, size, memType);
	}

	DeviceAllocator::DeviceAllocator(Device device, VkDeviceSize size)
		: Allocator(device),
		mChunkAlloc(device, size)
	{
		
	};

	void CreateDeviceAllocator(DeviceAllocator *allocator,
		Device device, VkDeviceSize size) {

		CreateChunkAllocator(&allocator->mChunkAlloc, device, size);

	}

	void DestroyDeviceAllocator(DeviceAllocator allocator) {
		std::cout << "DESTROY ALLOCATOR" << std::endl;

		for (auto &mChunk : allocator.mChunks) {
			DestroyChunk(&mChunk);
		}
	}

	Block DeviceAllocator::allocate(VkDeviceSize size, VkDeviceSize alignment, int memType) {
		Block block;

		for (auto &chunk : mChunks) {
			if (chunk.MemoryTypeIndex() == memType)
				if (chunk.Allocate(size, alignment, block))
					return block;
		}

		mChunks.emplace_back(mChunkAlloc.allocate(size, memType));
		bool result = mChunks.back().Allocate(size, alignment, block);

		if (result == false) {
			throw std::runtime_error("Cant alloc, may issue breaking cpu");
		}

		return block;

	}

	void DeviceAllocator::deallocate(Block &block) {
		for (auto& chunk : mChunks) {
			if (chunk.HasAllocated(block)) {
				chunk.Deallocated(block);
				return;
			}
		}
	}

	void CreateBuffer(Buffer &buffer, Device &device, VkBufferUsageFlags usage,
		VkDeviceSize size, bool localQ, DeviceAllocator allocator) 

	{
		
		buffer.mSize = size; buffer.deviceLocal = localQ;
		buffer.mUsageFlag = usage; buffer.mAllocator = allocator;
		buffer.mDevice = device;

		VkBufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.usage = usage;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.size = size;

		VkResult res = vkCreateBuffer(device.logicalDevice, &info, nullptr, &buffer.buffer);

		FRS_S_ASSERT(res!=VK_SUCCESS);

		vkGetBufferMemoryRequirements(device.logicalDevice,
			buffer.buffer, &buffer.mMemReq);

		vkGetPhysicalDeviceMemoryProperties(device.physicalDevice,
			&buffer.mMemProp);

		auto lamdaFunc = [&](VkMemoryPropertyFlags flag) -> int {
			for (uint32_t i = 0; i < buffer.mMemProp.memoryTypeCount; ++i)
				if (buffer.mMemReq.memoryTypeBits & (1 << i) &&
					((buffer.mMemProp.memoryTypes[i].propertyFlags & flag) == flag)) {
					return i;
				}
				return -1;
		};

		int memType;

		if (!localQ) {
			VkMemoryPropertyFlags optimal = VK_MEMORY_PROPERTY_HOST_CACHED_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

			VkMemoryPropertyFlags required = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

			memType = lamdaFunc(optimal);
			if (memType = -1) {
				memType = lamdaFunc(required);
				FRS_ASSERT(memType == -1, "Memory type failed");
			}
		}
		else {
			memType = lamdaFunc(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		}

		buffer.block = buffer.mAllocator.allocate(buffer.mMemReq.size, buffer.mMemReq.alignment, memType);
		vkBindBufferMemory(device.logicalDevice, buffer.buffer, buffer.block.memory, buffer.block.offSet);

	}

	void DestroyBuffer(Device device, Buffer* Tbuffer) {

		vkDestroyBuffer(device.logicalDevice, Tbuffer->buffer,
			nullptr);

		vkFreeMemory(device.logicalDevice, Tbuffer->block.memory, nullptr);

	}





}