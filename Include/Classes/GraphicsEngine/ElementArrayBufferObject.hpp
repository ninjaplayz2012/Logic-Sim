#ifndef EBO_CLASS_H
#define EBO_CLASS_H

#include "C:/glad/glad.h"

class ElementArrayBufferObject {
    public:
        GLuint ID = 0;

        ElementArrayBufferObject(GLuint *Indicies, GLsizeiptr Size);
        ElementArrayBufferObject() = default;

        void Create(GLuint *Indicies, GLsizeiptr Size);
        void Bind();
        void Unbind();
        void Delete();
};

#endif