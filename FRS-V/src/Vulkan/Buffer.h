#pragma once
#pragma warning (disable: 4251 4267)

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
		void *dataPointer = nullptr;
		std::vector<Block> mBlocks;
	};

	class TFKAPI ChunkAllocator {
	public:

		ChunkAllocator() = default;
		ChunkAllocator(Device &device, VkDeviceSize size);

		friend void CreateChunkAllocator(ChunkAllocator *allocator,
			Device device, VkDeviceSize size) {

			allocator->mDevice = device;
			allocator->mSize = size;

		}

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

		friend void CreateDeviceAllocator(DeviceAllocator *allocator,
			Device device, VkDeviceSize size);

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

		friend void DestroyDeviceAllocator(DeviceAllocator allocator);

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

		Device &getDevice() {
			return (mDevice);
		}

	protected:
		Device mDevice;
	};

	class TFKAPI Buffer: public Resource {
	public:
		Buffer() {
			directData = nullptr;
		};

		//Vertex of uniform?
		friend void CreateBuffer(Buffer &buffer,
			Device &device, VkBufferUsageFlags usage,
			VkDeviceSize size, bool localQ, 
			DeviceAllocator* allocator);

		Buffer(Buffer &&buffer) {

			this->buffer = buffer.buffer;
			
			dataPointer = buffer.dataPointer; 

			mAllocator = buffer.mAllocator;
			mDevice = buffer.mDevice;
			mSize = buffer.mSize;
			mUsageFlag = buffer.mUsageFlag;
			dataPointer = buffer.dataPointer; 
			mMemReq = buffer.mMemReq;
			mMemProp = buffer.mMemProp;
			directData = buffer.directData;
			transferSize = buffer.transferSize;
			setLayoutBinding = buffer.setLayoutBinding;

			range = buffer.range;
			offset = buffer.offset;

			block = buffer.block;
			deviceLocal = buffer.deviceLocal;	
			setPoolSize = buffer.setPoolSize;

		}

		Buffer(Buffer const &buffer) {

			this->buffer = buffer.buffer;

			dataPointer = buffer.dataPointer;

			mAllocator = buffer.mAllocator;
			mDevice = buffer.mDevice;
			mSize = buffer.mSize;
			mUsageFlag = buffer.mUsageFlag;
			dataPointer = dataPointer;
			mMemReq = buffer.mMemReq;
			mMemProp = buffer.mMemProp;

			setLayoutBinding = buffer.setLayoutBinding;
			block = buffer.block;
			deviceLocal = buffer.deviceLocal;
			range = buffer.range;
			offset = buffer.offset;
			length = buffer.length;
			dstSet = buffer.dstSet;
			directData = buffer.directData;
			transferSize = buffer.transferSize;
			setPoolSize = buffer.setPoolSize;
		}

		/*
		void Bind(VkVertexInputRate rate, VkDeviceSize binding,
			VkDeviceSize stride);

		void Attribute(
			VkDeviceSize location,
			VkDeviceSize offset,
			VkFormat format);
		

		void LayoutUniform(VkDeviceSize binding, VkDeviceSize dataArrayLength,
			Stage stage) {

			VkDescriptorSetLayoutBinding layout = {};

			layout.binding = binding;
			layout.descriptorCount = dataArrayLength;
			layout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layout.stageFlags = stage;
			layout.pImmutableSamplers = nullptr;

			this->setUniformLayoutBinding = layout;

			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = dataArrayLength;

		}
		*/
		Buffer &operator =(Buffer const &tBuffer) {
			Buffer nBuffer;

			nBuffer.mSize = tBuffer.mSize;
			nBuffer.mAllocator = tBuffer.mAllocator;
			nBuffer.dataPointer = tBuffer.dataPointer;
			nBuffer.block = tBuffer.block;
			nBuffer.mUsageFlag = tBuffer.mUsageFlag;
			nBuffer.deviceLocal = tBuffer.deviceLocal;
			nBuffer.buffer = tBuffer.buffer;
			nBuffer.directData = tBuffer.directData;
			nBuffer.transferSize = tBuffer.transferSize;
			nBuffer.length = tBuffer.length;
			nBuffer.dstSet = tBuffer.dstSet;
			nBuffer.offset = tBuffer.offset;
			nBuffer.range = tBuffer.range;	
			nBuffer.setLayoutBinding = tBuffer.setLayoutBinding;
			nBuffer.setPoolSize = tBuffer.setPoolSize;
		
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
	
		void* dataPointer = nullptr;

		~Buffer() {};
		
		friend void DestroyBuffer(Device device, Buffer* Tbuffer);

		/*
		std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions() {
			return attributeDescriptions;
		}

		VkVertexInputBindingDescription GetBindingDescriptions() {
			return bindingDes;
		}
		*/
	
		VkBuffer buffer;

		bool operator == (Buffer& buff) {

			if (buff.buffer == buffer 
				&& buff.block == block) {
				return true;
			}

			return false;
		}

		Block &GetBlock() {
			return block;
		}

		std::vector<uint32_t> offset = { 0 };
		uint32_t transferSize = 0;
		std::vector<uint32_t> range = { 0 };
		uint32_t length = { 1 };
		uint32_t dstSet = { 0 };

		VkDescriptorSetLayoutBinding setLayoutBinding{};
		VkDescriptorPoolSize setPoolSize{};

		void* directData = nullptr;

	private:

		
		Block block;

		DeviceAllocator* mAllocator{};
		VkDeviceSize mSize = 0;
		VkBufferUsageFlags mUsageFlag = VK_NULL_HANDLE;
		VkMemoryRequirements mMemReq{};
		VkPhysicalDeviceMemoryProperties mMemProp{};
		
		bool deviceLocal;

	

	};


}