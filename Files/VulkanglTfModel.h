#pragma once

#include "vulkan/vulkan.h"
#include "VulkanDevice.h"

#include <ktx.h>
#include <ktxvulkan.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "tiny_gltf.h"

using namespace vks;
using namespace std;
using namespace glm;

namespace vkglTF
{
	extern VkDescriptorSetLayout descriptorSetLayoutImage;
	extern VkDescriptorSetLayout descriptorSetLayoutUbo;
	extern VkMemoryPropertyFlags memoryPropertyFlags;

	struct Node;

	/*************************************************************************
	 * класс для загрузки glTF текстуры
	 *
	***********************************************************************/

	struct Texture
	{
		VulkanDevice* device;
		VkImage image;
		VkImageLayout imageLayout;
		VkDeviceMemory deviceMemory;
		VkImageView view;
		uint32_t width, height;
		uint32_t mipLevels;
		uint32_t layerCount;
		VkDescriptorImageInfo descriptor;
		VkSampler sampler;
		void UpdateDescriptor();
		void Destroy();
		void FromglTfImage(tinygltf::Image& gltfimage, string path, VulkanDevice* device, VkQueue copyQueue);
	};
	
	/*************************************************************************
	 * класс материала glTF текстуры
	 *
	***********************************************************************/
	struct Material
	{
		VulkanDevice* device;
		enum AlphaMode {ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
		AlphaMode alphaMode = ALPHAMODE_OPAQUE;
		float alphaCutoff = 1.0f;
		float metallicFactor = 1.0f;
		float roughnessFactor = 1.0f;
		vec4 baseColorFactor = vec4(1.0f);
		Texture* baseColorTexture = nullptr;
		Texture* metallicRoughnessTexture = nullptr;
		Texture* normalTexture = nullptr;
		Texture* occlusionTexture = nullptr;
		Texture* emissiveTexture = nullptr;

		Texture* specularGlossinessTexture;
		Texture* diffuseTexture;

		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
		Material(vks::VulkanDevice* device) :device(device) {};
		void CreateDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
	};
	
	/*************************************************************************
	 * glTF примитив
	 *
	***********************************************************************/
	struct Primitive
	{
		uint32_t firstIndex;
		uint32_t indexCount;
		uint32_t firstVertex;
		uint32_t vertexCount;
		Material& material;

		struct Dimensions
		{
			vec3 min = vec3(FLT_MAX);
			vec3 max = vec3(-FLT_MAX);
			vec3 size;
			vec3 center;
			float radius;
		}dimensions;

		void SetDimensions(vec3 min, vec3 max);
		Primitive(uint32_t firstIndex, uint32_t indexCount, Material material) :firstIndex(firstIndex), indexCount(indexCount), material(material) {};
	};
	
	/*************************************************************************
	 * glTF грани
	 *
	***********************************************************************/
	struct Mesh
	{
		VulkanDevice* device;
		vector<Primitive*>primitives;
		string name;

		struct UniformBuffer
		{
			VkBuffer buffer;
			VkDeviceMemory memory;
			VkDescriptorBufferInfo descriptor;
			VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
			void* mapped;
		}uniformBuffer;

		struct UniformBlock
		{
			mat4 matrix;
			mat4 jointMatrix[64];
			float jointcount{ 0 };
		}uniformBlock;

		
		Mesh(VulkanDevice* device, mat4 mtrix);
		~Mesh();
	};

	/*************************************************************************
	 * glTF скины
	 *
	***********************************************************************/
	struct Skin
	{
		string name;
		Node* skeletonRoot = nullptr;
		vector<mat4>inverseBindMatrices;
		vector<Node*>joints;
	};

	/*************************************************************************
	 * glTF узлы
	 *
	***********************************************************************/
	struct Node
	{
		Node* parent;
		uint32_t index;
		vector<Node*>children;
		mat4 matrix;
		string name;
		Mesh* mesh;
		Skin* skin;
		int32_t skinIndex = -1;
		vec3 translation{};
		vec3 scale{ 1.0f };
		quat rotation{};
		mat4 localMatrix();
		mat4 getMatrix();
		void Update();
		~Node();
	};
	
	/*************************************************************************
	 * канал анимации glTF 
	 *
	***********************************************************************/
	struct AnimationChannel
	{
		enum PathType{TRANSLATION, ROTATION, SCALE };
		PathType path;
		Node* node;
		uint32_t samplerIndex;
	};
	
	/*************************************************************************
	 * образцы анимации glTF 
	 *
	***********************************************************************/
	struct AnimationSampler
	{
		enum InterpolationType {LINEAR, STEP, CUBICSPLINE };
		InterpolationType interpolation;
		vector<float>inputs;
		vector<vec4>outputsVec4;
	};
	
	/*************************************************************************
	 *  анимация glTF 
	 *
	***********************************************************************/
	struct Animation
	{
		string name;
		vector<AnimationChannel>channels;
		vector<AnimationSampler>samplers;
		float start = numeric_limits<float>::max();
		float end = numeric_limits<float>::min();
	};
	
	/*************************************************************************
	 * слои glTF модели
	 *
	***********************************************************************/
	enum class VertexComponent{Position, Normal, UV, Color, Tangent, Joint0, Weight0};

	struct Vertex
	{
		vec3 pos;
		vec3 normal;
		vec2 uv;
		vec4 color;
		vec4 joint0;
		vec4 weight0;
		vec4 tangent;
		static VkVertexInputBindingDescription vertexInputBindingDescription;
		static vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
		static VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo;

		static VkVertexInputBindingDescription InputBindingDescription(uint32_t binding);
		static VkVertexInputAttributeDescription InputAttributeDescription(uint32_t binding, uint32_t location, VertexComponent component);
		static vector<VkVertexInputAttributeDescription> InputAttributeDescriptions(uint32_t binding, const vector<VertexComponent> components);
		static VkPipelineVertexInputStateCreateInfo* GetPipelineVertexInputState(const vector<VertexComponent> components);
	};

	enum FileLoadingFlags { None = 0x0, PreTransformVertices = 0x1, PreMultiplyVertexColors = 0x2, FlipY = 0x4, DontLoadImages = 0x8 };
	
	enum RenderFlag { BindImages = 0x1 };
	
	/*************************************************************************
	 * класс для загрузки и отображения glTF модели
	 *
	***********************************************************************/
	class Model
	{
		Texture* GetTexture(uint32_t index);

	public:
		VulkanDevice* device;
		VkDescriptorPool descriptorPool;
		
		struct Vertices
		{
			int count;
			VkBuffer buffer;
			VkDeviceMemory memory;
		}vertices;

		struct Indices
		{
			int count;
			VkBuffer buffer;
			VkDeviceMemory memory;
		}indices;

		vector<Node*> nodes;
		vector<Node*> linearNodes;

		vector<Skin*>skins;

		vector<Texture>textures;
		vector<Material>materials;
		vector<Animation>animations;

		struct Dimensions
		{
			vec3 min = vec3(FLT_MAX);
			vec3 max = vec3(FLT_MIN);
			vec3 size;
			vec3 center;
			float radius;
		}dimensions;

		bool metallicRoughnessWorkflow = true;
		bool buffersBound = false;
		string path;
		
		Model();
		~Model();

		void LoadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, vector<uint32_t>
		              & indexBuffer, vector<Vertex>& vertexBuffer, float globalScale);
		void LoadSkins(tinygltf::Model& gltfModel);
		void loadImage(tinygltf::Model& gltfModel, VulkanDevice* device, VkQueue transferQueue);
		void LoadMaterials(tinygltf::Model& gltfModel);
		void LoadAnimations(tinygltf::Model& gltfModel);
		void LoadFromFile(string filename, VulkanDevice* device, VkQueue transferQueue, uint32_t fileLoadingFlags = FileLoadingFlags::None, float scale = 1.0f);
		void BindBuffers(VkCommandBuffer commandBuffer);
		static void DrawNode(Node* node, VkCommandBuffer commandBuffer, uint32_t renderFlags = 0, VkPipelineLayout pipelineLayout = VK_NULL_HANDLE, uint32_t bindImageSet = 1);
		void draw(VkCommandBuffer commandBuffer, uint32_t renderFlags = 0, VkPipelineLayout pipelineLayout = VK_NULL_HANDLE, uint32_t bindImageSet = 1);
		void GetNodeDimensions(Node* node, vec3& min, vec3& max);
		void GetSceneDimensions();
		void UpdateAnimation(uint32_t index, float time);
		static Node* FindNode(Node* parent, uint32_t index);
		Node* nodeFromIndex(uint32_t index);
		void prepareNodeDescriptor(Node* node, VkDescriptorSetLayout descriptorSetlayout);
	};
}
