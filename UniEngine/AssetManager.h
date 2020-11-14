#pragma once
#include "Core.h"
#include "UniEngineAPI.h"
#include "Texture2D.h"
#include "MeshRenderer.h"
#include "Model.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include "FileBrowser.h"

namespace UniEngine {
    class UNIENGINE_API AssetManager : public ManagerBase
    {
        static bool _EnableAssetMenu;
        static FileBrowser _FileBrowser;
        static std::vector<std::shared_ptr<Model>> _Models;
        static std::vector<std::shared_ptr<Texture2D>> _Texture2Ds;
        static std::vector<std::shared_ptr<Cubemap>> _Cubemaps;
        static void ProcessNode(std::string, std::shared_ptr<GLProgram> shader, std::unique_ptr<ModelNode>&, std::vector<std::shared_ptr<Texture2D>>&, aiNode*, const aiScene*);
        static void ReadMesh(unsigned meshIndex, std::unique_ptr<ModelNode>&, std::string directory, std::shared_ptr<GLProgram> shader, std::vector<std::shared_ptr<Texture2D>>& Texture2DsLoaded, aiMesh* aimesh, const aiScene* scene);
        static void AttachChildren(EntityArchetype archetype, std::unique_ptr<ModelNode>& modelNode, Entity parentEntity, std::string parentName);

        static void ModelGuiNode(int i);
        static void TextureGuiNode(int i);
    public:
        static std::shared_ptr<Model> GetModel(int i);
        static std::shared_ptr<Texture2D> GetTexture2D(int i);
    	static std::shared_ptr<Texture2D> LoadTexture(std::string path);
        static void RemoveModel(int index);
        static void RemoveTexture(int index);
        static void LateUpdate();
        static std::shared_ptr<Model> LoadModel(std::string const& path, std::shared_ptr<GLProgram> shader, bool gamma = false);
        static Entity ToEntity(EntityArchetype archetype, std::shared_ptr<Model> model);
    };
}
