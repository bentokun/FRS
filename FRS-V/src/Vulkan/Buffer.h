#pragma once

#include "Rules.h"
#include "Assert.h"
#include "Device.h"

#include <Vulkan.h>
#include <vector>
#include <algorithm>
#include <memory>

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFKAPI __declspec(dllexport)
#else
#define TFKAPI __declspec(dllimport)
#endif
#endif

namespace FRS {


	struct TFKAPI Block {

		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDeviceSize size = VK_NULL_HANDLE;
		VkDeviceSize offSet = VK_NULL_HANDLE;

		bool free;
		void *ptr = nullptr;

		bool operator ==(Block const &block);
	};

	class TFKAPI Chunk {
	public:

		Chunk() {};
		Chunk(Device& device, VkDeviceSize size, int memType);

		bool Allocate(VkDeviceSize size, VkDeviceSize align, Block& block);
		bool HasAllocated(Block const &block);
		void Deallocated(Block const &block);

		int MemoryTypeIndex() const {
			return mMemoryTypeIndex;
		}

		bool operator ==(Chunk chunk) {
			if (mMem == chunk.mMem && mMemoryTypeIndex == chunk.mMemoryTypeIndex) {
				for (uint32_t i = 0; i < mBlocks.size(); i++) {
					if (!(mBlocks[i] == chunk.mBlocks[i])) {
						return false;
					}
				}

				return true;
			}

			return false;
		}

		~Chunk() {};

		friend void DestroyChunk(Chunk* chunk);

	private:
		Device mDevice;
		VkDeviceSize mSize = VK_NULL_HANDLE;
		VkDeviceMemory mMem = VK_NULL_HANDLE;
		int mMemoryTypeIndex;
		void *mPtr = nullptr;
		std::vector<Block> mBlocks;
	};

	class TFKAPI ChunkAllocator {
	public:

		ChunkAllocator() = default;
		ChunkAllocator(Device &device, VkDeviceSize size);

		Chunk allocate(VkDeviceSize size,
			int memType);

		bool operator ==(ChunkAllocator allocator) {
			return mSize == allocator.mSize;
		}

	private:
		Device mDevice;
		VkDeviceSize mSize;
	};

	class TFKAPI Allocator {
	public:

		Allocator() = default;

		Allocator(Device const &device) :
			mDevice(device) {};

		virtual Block allocate(VkDeviceSize size, VkDeviceSize offset, int aligment) { return Block(); };
		virtual void deallocate(Block &block) {};

		virtual ~Allocator() {};

	private:
		Device mDevice;

	};

	class TFKAPI DeviceAllocator : public Allocator {
	public:

		DeviceAllocator() {};
		DeviceAllocator(Device device, VkDeviceSize size);

		Block allocate(VkDeviceSize size, VkDeviceSize alignment, int memType);
		void deallocate(Block &block);

		bool operator ==(DeviceAllocator ant) {
			if (mChunkAlloc == ant.mChunkAlloc) {
				for (uint32_t i = 0; i < mChunks.size();i++) {
					if (!(mChunks[i] == ant.mChunks[i])) {
						return false;
					}
				}
				return true;
			}

			return false;
		}

		void DestroyDeviceAllocator() {
			for (auto &mChunk : mChunks) {
				DestroyChunk(&mChunk);
			}
		}

	private:
		ChunkAllocator mChunkAlloc{};
		std::vector<Chunk> mChunks;
	};

	//Shared_ptr
	class TFKAPI Counter {
	public:
		Counter() = default;
		Counter(Counter const &counter);
		Counter(Counter &&counter) = default;
		Counter &operator =(Counter counter);

		uint32_t getCount() const;

		virtual ~Counter() {};

	protected:
		std::shared_ptr<uint32_t> count = std::make_shared<uint32_t>(1);
	};

	//Different from ResourceManager, represent a resource instead
	class TFKAPI Resource: Counter {
	public:
		Resource() = default;
		Resource(Device const &device);
		Resource(Resource &&resource) = default;
		Resource &operator =(Resource &&resource) = default;
		Resource &operator =(Resource const &resource) = default;

		Device &getDevice() const {
			return *(mDevice);
		}

	protected:
		std::shared_ptr<Device> mDevice;
	};

	class TFKAPI Buffer: public Resource {
	public:
		Buffer() {};

		//Vertex of uniform?
		friend void CreateBuffer(Buffer &buffer, Device &device, VkBufferUsageFlags usage,
			VkDeviceSize size, bool localQ, DeviceAllocator allocator);

		Buffer(Buffer &&buffer) {

			this->buffer = buffer.buffer;
			
			bindingDes = buffer.bindingDes;

			for (auto& attribute : buffer.attributeDescriptions) {
				attributeDescriptions.push_back(attribute);
			}

			mAllocator = buffer.mAllocator;
			mDevice = buffer.mDevice;
			mSize = buffer.mSize;
			mUsageFlag = buffer.mUsageFlag;
			mPtr = buffer.mPtr; 
			mMemReq = buffer.mMemReq;
			mMemProp = buffer.mMemProp;

			block = buffer.block;
			deviceLocal = buffer.deviceLocal;	

		}

		Buffer(Buffer const &buffer) {

			this->buffer = buffer.buffer;

			bindingDes = buffer.bindingDes;

			for (auto& attribute : buffer.attributeDescriptions) {
				attributeDescriptions.push_back(attribute);
			}

			mAllocator = buffer.mAllocator;
			mDevice = buffer.mDevice;
			mSize = buffer.mSize;
			mUsageFlag = buffer.mUsageFlag;
			mPtr = buffer.mPtr;
			mMemReq = buffer.mMemReq;
			mMemProp = buffer.mMemProp;

			block = buffer.block;
			deviceLocal = buffer.deviceLocal;

		}

		void Bind(VkVertexInputRate rate, VkDeviceSize binding,
			VkDeviceSize stride) {
			bindingDes.binding = binding;
			bindingDes.inputRate = rate;
			bindingDes.stride = stride;
		}

		void Attribute(
			VkDeviceSize location,
			VkDeviceSize offset,
			VkFormat format) {

			VkVertexInputAttributeDescription des = {};
			des.offset = offset;
			des.location = location;
			des.binding = bindingDes.binding;
			des.format = format;

			attributeDescriptions.push_back(des);

		}
		
		Buffer &operator =(Buffer const &tBuffer) {
			Buffer nBuffer;

			nBuffer.mSize == tBuffer.mSize;
			nBuffer.mAllocator == tBuffer.mAllocator;
			nBuffer.mPtr == tBuffer.mPtr;
			nBuffer.block == tBuffer.block;
			nBuffer.mUsageFlag == tBuffer.mUsageFlag;
			nBuffer.deviceLocal == tBuffer.deviceLocal;
			nBuffer.buffer == tBuffer.buffer;
				
			return nBuffer;
		}

		VkDeviceSize getSize() const {
			return mSize;
		}

		VkBufferUsageFlags getUsage() const{
			return mUsageFlag;
		}

		bool isDeviceLocal() const {
			return deviceLocal;
		}

		void *getPtr() {
			return mPtr;
		}
		
		~Buffer() {};

		friend void DestroyBuffer(Device& device, Buffer* Tbuffer) {
			vkDestroyBuffer(device.logicalDevice, Tbuffer->buffer,
				nullptr);
		}

		std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions() {
			return attributeDescriptions;
		}

		VkVertexInputBindingDescription GetBindingDescriptions() {
			return bindingDes;
		}

		VkBuffer buffer;

		bool operator == (Buffer& buff) {
			if (buff.buffer == buffer 
				&& buff.block == block
				&& buff.mUsageFlag == mUsageFlag
				&& buff.mAllocator == mAllocator
				&& buff.mSize == mSize
				&& deviceLocal == buff.deviceLocal
				&& mPtr == buff.mPtr) {
				return true;
			}

			return false;
		}

		Block &GetBlock() {
			return block;
		}

	private:

		
		Block block;

		DeviceAllocator mAllocator{};
		VkDeviceSize mSize = 0;
		VkBufferUsageFlags mUsageFlag = VK_NULL_HANDLE;
		VkMemoryRequirements mMemReq{};
		VkPhysicalDeviceMemoryProperties mMemProp{};
		
		bool deviceLocal;
		void* mPtr = nullptr;
	
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		VkVertexInputBindingDescription bindingDes = {};

	};


}