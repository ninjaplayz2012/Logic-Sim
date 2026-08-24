#ifndef VERTEX_BUFFER_OBJECT_CLASS_H
#define VERTEX_BUFFER_OBJECT_CLASS_H

#include "C:/glad/glad.h"

class VertexBufferObject {
    public:
        GLuint ID;
        VertexBufferObject(GLfloat *Vertices, GLsizeiptr Size);
        VertexBufferObject(GLuint *Vertices, GLsizeiptr Size);
        VertexBufferObject() = default;

        void Create(GLfloat *Vertices, GLsizeiptr Size);
        void Create(GLuint *Vertices, GLsizeiptr Size);
        
        void Bind();
        void Unbind();
        void Delete();
};

#endif