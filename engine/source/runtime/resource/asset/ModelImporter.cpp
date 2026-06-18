#include "runtime/resource/asset/ModelImporter.h"

#include "runtime/function/global/GlobalContext.h"
#include "runtime/resource/asset/AssetManager.h"
#include "runtime/resource/asset/Serializer.h"
#include "runtime/resource/asset/AssetMetadata.h"
#include "runtime/resource/asset/TextureImporter.h"
#include "runtime/resource/data/Model.h"
#include "runtime/resource/data/Mesh.h"
#include "runtime/resource/data/Material.h"
#include "runtime/core/Log.h"
#include "runtime/core/Macro.h"

//#define ASSIMP_DLL
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

namespace Lumen
{
	struct ModelData
	{
		// mesh
		uint32_t curMesh = 0;
		std::vector<std::string> MeshNames;

		std::vector<Submesh> Submeshes;
		std::vector<std::vector<AssetHandle>> Materials;

		std::vector<Vertex> Vertices;
		std::vector<uint32_t> Indices;

		// material
		std::vector<std::string> MiNames;

		// animation
	};

	static ModelData s_ModelData;
	
	static const uint32_t s_MeshImportFlags =
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_GenUVCoords |
		aiProcess_GenBoundingBoxes;

	struct LogStream : public Assimp::LogStream
	{
		static void Initialize()
		{
			if (Assimp::DefaultLogger::isNullLogger())
			{
				Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
				Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
			}
		}

		virtual void write(const char* message) override
		{
			LUMEN_RUNTIME_ERROR("Assimp error: {0}", message);
		}
	};

    Shared<Asset> ModelImporter::Import()
    {
		LogStream::Initialize();
		LUMEN_RUNTIME_INFO("Loading model: {0}", m_AssetPath.filename().string());

		auto m_Importer = std::make_unique<Assimp::Importer>();
		const aiScene* scene = m_Importer->ReadFile(m_AssetPath.string(), s_MeshImportFlags);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LUMEN_RUNTIME_ERROR("Can not load the model: {0}", m_AssetPath.string());
			return nullptr;
		}

		directory = m_AssetPath.parent_path();

		s_ModelData = ModelData();

		ProcessMaterials(scene);
		ProcessNode(scene->mRootNode, scene);

		ModelInfo info;
		info.Meshes = m_ModelImportSettings.Meshes;
		info.Materials = m_ModelImportSettings.Materials;
		info.SubMaterials = std::move(s_ModelData.Materials);
		info.MeshesName = std::move(s_ModelData.MeshNames);
		info.MaterialsName = std::move(s_ModelData.MiNames);
		Shared<Model> model = MakeShared<Model>(std::move(info));

        return model;
    }

	void ModelImporter::ProcessMaterials(const aiScene* scene)
	{
		if (scene->HasMaterials())
		{
			LUMEN_RUNTIME_INFO("---- Materials ----");

			s_ModelData.MiNames.resize(scene->mNumMaterials);

			for (uint32_t i = 0; i < scene->mNumMaterials; i++)
			{
				AssetHandle miHandle;
				if (m_ModelImportSettings.Materials.size() != scene->mNumMaterials)
				{
					m_ModelImportSettings.Materials.emplace_back(miHandle);
					AssetMetadata miMetadata;
					miMetadata.assetFilePath = std::filesystem::relative(m_AssetPath, LUMEN_ASSET_MANAGER->GetAssetDirectory());
					miMetadata.type = AssetType::Material;
					LUMEN_ASSET_MANAGER->AddRegisterAsset(miHandle, miMetadata);
				}
				else
				{
					miHandle = m_ModelImportSettings.Materials[i];
				}

				Shared<Material> mi = MakeShared<Material>();
				LUMEN_ASSET_MANAGER->AddLoadedAsset(miHandle, mi);

				mi->SetShader("PBRStaticShader");

				auto aiMaterial = scene->mMaterials[i];
				auto aiMaterialName = aiMaterial->GetName();
				s_ModelData.MiNames[i] = aiMaterialName.C_Str();
				LUMEN_RUNTIME_TRACE("  {0} (Index = {1})", aiMaterialName.data, i);

				glm::vec3 color(0.8f);
				aiColor3D aiColor;
				if (aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor) == aiReturn_SUCCESS)
					color = { aiColor.r, aiColor.g, aiColor.b };
				
				mi->Set("u_Material.Color", color);
				LUMEN_RUNTIME_TRACE("    Base Color = {0}, {1}, {2}", color.r, color.g, color.b);

				color = glm::vec3(1.f);
				if (aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, aiColor) == aiReturn_SUCCESS)
					color = { aiColor.r, aiColor.g, aiColor.b };
				
				//mi->Set("u_Material.Specular", color);
				LUMEN_RUNTIME_TRACE("    Specular Color = {0}, {1}, {2}", color.r, color.g, color.b);

				float shininess, metalness;
				if (aiMaterial->Get(AI_MATKEY_SHININESS, shininess) != aiReturn_SUCCESS)
					shininess = 80.0f; // Default value
				if (aiMaterial->Get(AI_MATKEY_REFLECTIVITY, metalness) != aiReturn_SUCCESS)
					metalness = 0.0f; // Default value
				float roughness = 1.f - glm::sqrt(shininess / 100.f);

				mi->Set("u_Material.Metalness", metalness);
				mi->Set("u_Material.Roughness", roughness);
				LUMEN_RUNTIME_TRACE("    Metalness = {0}", metalness);
				LUMEN_RUNTIME_TRACE("    Roughness = {0}", roughness);

				aiString aiTexPath;

				bool hasAlbedoMap = aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexPath) == AI_SUCCESS;
				mi->Set("u_AlbedoTexture", AssetHandle(0));
				mi->Set("u_Material.UseAlbedoTexture", hasAlbedoMap);
				if (hasAlbedoMap)
				{
					std::filesystem::path mapPath = directory / std::filesystem::path(aiTexPath.data);
					LUMEN_RUNTIME_TRACE("    Albedo texture path: {0}", mapPath.generic_string());
					TextureImporter importer(mapPath);
					AssetHandle handle = importer.Deserialize();
					if (handle)
						mi->Set("u_AlbedoTexture", handle);
					else
						LUMEN_RUNTIME_ERROR("Could not load the albedo texture: {0}", mapPath.generic_string());
				}
				else
					LUMEN_RUNTIME_TRACE("    No albedo texture");

				bool hasMetalnessTexture = aiMaterial->GetTexture(aiTextureType_METALNESS, 0, &aiTexPath) == AI_SUCCESS;
				mi->Set("u_MetalnessTexture", AssetHandle(0));
				mi->Set("u_Material.UseMetalnessTexture", hasMetalnessTexture);
				if (hasMetalnessTexture)
				{
					std::filesystem::path mapPath = directory / std::filesystem::path(aiTexPath.data);
					LUMEN_RUNTIME_TRACE("    Metalness texture path: {0}", mapPath.generic_string());
					TextureImporter importer(mapPath);
					AssetHandle handle = importer.Deserialize();
					if (handle)
						mi->Set("u_MetalnessTexture", handle);
					else
						LUMEN_RUNTIME_ERROR("Could not load the metalness texture: {0}", mapPath.generic_string());
				}
				else
					LUMEN_RUNTIME_TRACE("    No metalness texture");

				bool hasRoughnessTexture = aiMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &aiTexPath) == AI_SUCCESS;
				mi->Set("u_RoughnessTexture", AssetHandle(0));
				mi->Set("u_Material.UseRoughnessTexture", hasMetalnessTexture);
				if (hasRoughnessTexture)
				{
					std::filesystem::path mapPath = directory / std::filesystem::path(aiTexPath.data);
					LUMEN_RUNTIME_TRACE("    Roughness texture path: {0}", mapPath.generic_string());
					TextureImporter importer(mapPath);
					AssetHandle handle = importer.Deserialize();
					if (handle)
						mi->Set("u_RoughnessTexture", handle);
					else
						LUMEN_RUNTIME_ERROR("Could not load the roughness texture: {0}", mapPath.generic_string());
				}
				else
					LUMEN_RUNTIME_TRACE("    No roughness texture");

				bool hasNormalMap = aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath) == AI_SUCCESS;
				mi->Set("u_NormalTexture", AssetHandle(0));
				mi->Set("u_Material.UseNormalTexture", hasNormalMap);
				if (hasNormalMap)
				{
					std::filesystem::path mapPath = directory / std::filesystem::path(aiTexPath.data);
					LUMEN_RUNTIME_TRACE("    Normal texture path: {0}", mapPath.generic_string());
					TextureImporter importer(mapPath);
					AssetHandle handle = importer.Deserialize();
					if (handle)
						mi->Set("u_NormalTexture", handle);
					else
						LUMEN_RUNTIME_ERROR("Could not load the normal texture: {0}", mapPath.generic_string());
				}
				else
					LUMEN_RUNTIME_TRACE("    No normal texture");

				bool hasHeightMap = aiMaterial->GetTexture(aiTextureType_HEIGHT, 0, &aiTexPath) == AI_SUCCESS;
				mi->Set("u_HeightTexture", AssetHandle(0));
				mi->Set("u_Material.UseHeightTexture", hasHeightMap);
				if (hasHeightMap)
				{
					std::filesystem::path mapPath = directory / std::filesystem::path(aiTexPath.data);
					LUMEN_RUNTIME_TRACE("    Height texture path: {0}", mapPath.generic_string());
					TextureImporter importer(mapPath);
					AssetHandle handle = importer.Deserialize();
					if (handle)
						mi->Set("u_HeightTexture", handle);
					else
						LUMEN_RUNTIME_ERROR("Could not load the height texture: {0}", mapPath.generic_string());
				}
				else
					LUMEN_RUNTIME_TRACE("    No height texture");

				bool hasOcclusionTexture = aiMaterial->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &aiTexPath) == AI_SUCCESS;
				mi->Set("u_OcclusionTexture", AssetHandle(0));
				mi->Set("u_Material.UseOcclusionTexture", hasOcclusionTexture);
				if (hasHeightMap)
				{
					std::filesystem::path mapPath = directory / std::filesystem::path(aiTexPath.data);
					LUMEN_RUNTIME_TRACE("    Occlusion texture path: {0}", mapPath.generic_string());
					TextureImporter importer(mapPath);
					AssetHandle handle = importer.Deserialize();
					if (handle)
						mi->Set("u_OcclusionTexture", handle);
					else
						LUMEN_RUNTIME_ERROR("Could not load the occlusion texture: {0}", mapPath.generic_string());
				}
				else
					LUMEN_RUNTIME_TRACE("    No occlusion texture");
			}
			LUMEN_RUNTIME_INFO("------------------------");
		}
		else
		{
			LUMEN_RUNTIME_WARN("---- No Materials ----");
		}
	}

	void ModelImporter::ProcessNode(aiNode* node, const aiScene* scene)
	{
		if (node->mNumMeshes > 0)
		{
			s_ModelData.Indices.clear();
			s_ModelData.Vertices.clear();
			s_ModelData.Submeshes.clear();

			s_ModelData.Materials.emplace_back();
			for (uint32_t i = 0; i < node->mNumMeshes; ++i)
			{
				Submesh& submesh = s_ModelData.Submeshes.emplace_back();
				submesh.StartIndex = static_cast<uint32_t>(s_ModelData.Indices.size());
				submesh.StartVertex = static_cast<uint32_t>(s_ModelData.Vertices.size());

				ProcessSubmesh(scene->mMeshes[node->mMeshes[i]], scene);

				submesh.IndexCount =
					static_cast<uint32_t>(s_ModelData.Indices.size()) - submesh.StartIndex;
				submesh.VertexCount =
					static_cast<uint32_t>(s_ModelData.Vertices.size()) - submesh.StartVertex;
			}
		
			AssetHandle meshHandle;
			if (m_ModelImportSettings.Meshes.size() <= s_ModelData.curMesh)
			{
				m_ModelImportSettings.Meshes.emplace_back(meshHandle);
				AssetMetadata meshMetadata;
				meshMetadata.type = AssetType::Mesh;
				meshMetadata.assetFilePath = std::filesystem::relative(m_AssetPath, LUMEN_ASSET_MANAGER->GetAssetDirectory());
				LUMEN_ASSET_MANAGER->AddRegisterAsset(meshHandle, meshMetadata);
			}
			else
				meshHandle = m_ModelImportSettings.Meshes[s_ModelData.curMesh];

			Shared<Mesh> mesh = MakeShared<Mesh>(
				s_ModelData.Submeshes, s_ModelData.Vertices, s_ModelData.Indices);
			LUMEN_ASSET_MANAGER->AddLoadedAsset(meshHandle, mesh);
			s_ModelData.curMesh++;

			s_ModelData.MeshNames.emplace_back(node->mName.C_Str());
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
			ProcessNode(node->mChildren[i], scene);
	}

	void ModelImporter::ProcessSubmesh(aiMesh* submesh, const aiScene* scene)
	{
		ASSERT(submesh->HasPositions());
		ASSERT(submesh->HasNormals());

		for (unsigned int i = 0; i < submesh->mNumVertices; i++)
		{
			Vertex& vertex = s_ModelData.Vertices.emplace_back();

			vertex.Position = { submesh->mVertices[i].x, submesh->mVertices[i].y, submesh->mVertices[i].z };
			vertex.Normal = { submesh->mNormals[i].x, submesh->mNormals[i].y, submesh->mNormals[i].z };

			if (submesh->HasTextureCoords(0))
				vertex.Texcoord = { submesh->mTextureCoords[0][i].x, submesh->mTextureCoords[0][i].y };

			if (submesh->HasTangentsAndBitangents())
			{
				vertex.Tangent = { submesh->mTangents[i].x, submesh->mTangents[i].y, submesh->mTangents[i].z };
				vertex.Binormal = { submesh->mBitangents[i].x, submesh->mBitangents[i].y, submesh->mBitangents[i].z };
			}
		}

		for (unsigned int i = 0; i < submesh->mNumFaces; i++)
		{
			ASSERT(submesh->mFaces[i].mNumIndices == 3);

			for (unsigned int j = 0; j < submesh->mFaces[i].mNumIndices; j++)
				s_ModelData.Indices.emplace_back(submesh->mFaces[i].mIndices[j]);

			// TODO: cache triangle
		}

		s_ModelData.Materials.back().emplace_back(m_ModelImportSettings.Materials[submesh->mMaterialIndex]);
	}

	bool ModelImporter::Serialize(AssetHandle handle)
	{
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "UUID" << YAML::Value << handle;
		out << YAML::Key << "ModelImporter" << YAML::Value;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "GenerateColliders" << YAML::Value << m_ModelImportSettings.GenerateColliders;
			
			out << YAML::Key << "Meshes" << YAML::Value;
			out << YAML::BeginSeq;
			for (const auto& handle : m_ModelImportSettings.Meshes)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Handle" << YAML::Value << handle;
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::Key << "Materials" << YAML::Value;
			out << YAML::BeginSeq;
			for (const auto& handle : m_ModelImportSettings.Materials)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Handle" << YAML::Value << handle;
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(m_AssetPath.string() + ".meta");
		ASSERT(fout.is_open());
		fout << out.c_str();

		return true;
	}

	AssetHandle ModelImporter::Deserialize()
	{
		m_ModelImportSettings.Meshes.clear();
		m_ModelImportSettings.Materials.clear();
		m_ModelImportSettings.Animations.clear();

		auto data = YAML::LoadFile(m_AssetPath.string() + ".meta");
		ASSERT(data);

		AssetHandle handle = data["UUID"].as<UUID>();
		auto settings = data["ModelImporter"];
		ASSERT(settings);
		m_ModelImportSettings.GenerateColliders = settings["GenerateColliders"].as<bool>();
		
		auto meshes = settings["Meshes"];
		for (const auto& node : meshes)
		{
			m_ModelImportSettings.Meshes.emplace_back(node["Handle"].as<UUID>());
		}

		auto Materials = settings["Materials"];
		for (const auto& node : Materials)
		{
			m_ModelImportSettings.Materials.emplace_back(node["Handle"].as<UUID>());
		}

		return handle;
	}
}


