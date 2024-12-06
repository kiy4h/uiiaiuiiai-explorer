#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ����ṹ��
struct Vertex {
    // λ��
    glm::vec3 Position;
    // ����
    glm::vec3 Normal;
    // ��������
    glm::vec2 TexCoords;
    // Tangent
    glm::vec3 Tangent;
    // Bitangent
    glm::vec3 Bitangent;
};
// ���ڽṹ������ԣ������������Ų����ڴ棬�Ӷ��ڴ�������ʱ����ת��������ĳ� float ����

// �����ṹ��
struct Texture {
    unsigned int id;
    string type;
    string path;
};

// mesh
class Mesh {
public:
    /*  ��������  */
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    unsigned int VAO;

    /*  ����  */
    // ���캯��
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures) {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // ȥ���ö��㻺����ָ�����������
        setupMesh();
    }

    // ��Ⱦ mesh
    void Draw(Shader shader) {
        // �󶨺��ʵ�����
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++) {
            // �ڰ�֮ǰ������Ӧ��������Ԫ
            glActiveTexture(GL_TEXTURE0 + i);
            // ��ȡ������ţ�diffuse_textureN �е� N��
            string number;
            string name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++); // �޷�������ת�Ƶ����У���ͬ
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);
            else if (name == "texture_normal")
                number = std::to_string(normalNr++);
            else if (name == "texture_height")
                number = std::to_string(heightNr++);

            // ��ȡ��������Ϊ��ȷ��������Ԫ
            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            // ������
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // ���� mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // ����ϰ�ߣ�����Ĭ������
        glActiveTexture(GL_TEXTURE0);
    }

private:
    /*  ��Ⱦ����  */
    unsigned int VBO, EBO;

    /*  ����  */
    // ��ʼ�����еĻ���������/���飨VBO/VAO��
    void setupMesh() {
        // ���� buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // �������ݽ��� vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // ����vertex����ָ��
        // vertex λ��
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        // ��һ������ָ��������0��ʼȡ���ݣ��붥����ɫ����layout(location=0)��Ӧ��
        // �ڶ�������ָ���������Դ�С��
        // ����������ָ���������͡�
        // ���ĸ����������Ƿ�ϣ�����ݱ���׼������һ������ֻ��ʾ���򲻱�ʾ��С��
        // ����������ǲ�����Stride����ָ���������Ķ�������֮��ļ�������洫0�ʹ�4Ч����ͬ�������1ȡֵ��ʽΪ0123��1234��2345����
        // ������������ʾ���ǵ�λ�������ڻ�������ʼλ�õ�ƫ������

        // vertex ����
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
        // vertex ��������
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));
        // vertex Tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));
        // vertex Bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);
    }
};
#endif