#include "Classes/GraphicsEngine/ElementArrayBufferObject.hpp"

ElementArrayBufferObject::ElementArrayBufferObject(GLuint *Indicies, GLsizeiptr Size) {
    glGenBuffers(1, &ID);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Size, Indicies, GL_STATIC_DRAW);
}

void ElementArrayBufferObject::Create(GLuint *Indicies, GLsizeiptr Size) {
    glGenBuffers(1, &ID);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Size, Indicies, GL_STATIC_DRAW);
}

void ElementArrayBufferObject::Bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

void ElementArrayBufferObject::Unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ElementArrayBufferObject::Delete() {
    glDeleteBuffers(1, &ID);
}
