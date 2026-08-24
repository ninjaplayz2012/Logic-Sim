#ifndef VAO_CLASS_H
#define VAO_CLASS_H

#include "Classes/GraphicsEngine/VertexBufferObject.hpp"

class VertexArrayObject {
    public:
        GLuint ID;
        VertexArrayObject();

        void LinkAttrib(VertexBufferObject &VBO, GLuint Layout, GLuint NumComponents, GLenum Type, GLsizeiptr Stride, void* Offset);
        void Bind();
        void Unbind();
        void Delete();
};

#endif