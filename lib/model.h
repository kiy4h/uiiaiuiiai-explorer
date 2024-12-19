// Refer to the learnopengl library

#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stb/stb_image.h>

#include "mesh.h"
#include "shader.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#pragma comment(lib, "assimp.lib")

inline unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

class Model {
public:
    vector<Texture> textures_loaded;
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;

    /*  ����  */
    // ���캯������·���ж�ȡģ��
    Model(string const &path, bool gamma = false)
        : gammaCorrection(gamma) {
        loadModel(path);
    }

    // ����ģ�͵���������
    void Draw(Shader shader) {
        for (unsigned int i = 0; i < meshes.size(); i++) {
            meshes[i].Draw(shader);
        }
    }
    void SetPosition(const glm::vec3 &position) {
        this->position = position;
    }

    glm::vec3 GetPosition() const {
        return position;
    }

    void SetRotation(const glm::vec3 &rotation) {
        this->rotation = rotation;
    }

    glm::vec3 GetRotation() const {
        return rotation;
    }

    // Method to retrieve the first diffuse texture ID
    unsigned int GetTextureID() const {
        for (const auto &mesh : meshes) {
            for (const auto &texture : mesh.textures) {
                if (texture.type == "texture_diffuse") {
                    return texture.id; // Return the ID of the first diffuse texture
                }
            }
        }
        return 0; // Return 0 if no diffuse texture is found
    }

private:
    glm::vec3 position;
    glm::vec3 rotation;
    /*  ����  */
    // ���ļ�����ģ��֧�� ASSIMP ��չ���洢���������ɵ���������
    void loadModel(string const &path) {
        // ͨ�� ASSIMP ����ģ���ļ�
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

        // �ж��Ƿ��д���
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // �����ļ�·����Ŀ¼·��
        directory = path.substr(0, path.find_last_of('/'));

        // �ݹ鴦�� ASSIMP �ĸ��ڵ�
        processNode(scene->mRootNode, scene);
    }

    // �Եݹ鷽ʽ�����ڵ�
    // �����ڵ���ÿ�����������񣬲��������ӽڵ㣨����У��ظ��˹���
    // scene �����������ݣ�node �������ﲿ��֮��Ĺ�ϵ
    void processNode(aiNode *node, const aiScene *scene) {
        // �����ڵ����������
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // �������������ӽڵ��ظ���һ����
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            glm::vec3 vector;

            // Position
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // Normal
            if (mesh->mNormals) {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            } else {
                vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);
            }

            // Texture Coordinates
            if (mesh->mTextureCoords[0]) {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y; // Check for flipping issues
                vertex.TexCoords = vec;
            } else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            // Tangents and Bitangents
            if (mesh->mTangents) {
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
            } else {
                vertex.Tangent = glm::vec3(0.0f, 0.0f, 0.0f);
            }

            if (mesh->mBitangents) {
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            } else {
                vertex.Bitangent = glm::vec3(0.0f, 0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        // Process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // Process material textures
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // ����һ�� Mesh ����
        return Mesh(vertices, indices, textures);
    }

    // ���ز�������
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName) {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            // ���ṹ�Ƿ񱻼��أ������������
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++) {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // ��ͬ������·���Ѿ����ع��ˣ���������
                    break;
                }
            }
            // δ���ع�
            if (!skip) {
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture); // ���ӵ��Ѽ��ص�������
            }
        }
        return textures;
    }
};

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma) {
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA; // Support alpha channel

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture wrapping and filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } 
    else {
        stbi_image_free(data);
    }

    return textureID;
}

#endif