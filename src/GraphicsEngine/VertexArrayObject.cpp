#include "Classes/GraphicsEngine/VertexArrayObject.hpp"

VertexArrayObject::VertexArrayObject() {
    glGenVertexArrays(1, &ID);
}

void VertexArrayObject::LinkAttrib(VertexBufferObject &VBO, GLuint Layout, GLuint NumComponents, GLenum Type, GLsizeiptr Stride, void* Offset) {
    VBO.Bind();
    
    glVertexAttribPointer(Layout, NumComponents, Type, GL_FALSE, Stride, Offset);
    glEnableVertexAttribArray(Layout);
    
    VBO.Unbind();
}

void VertexArrayObject::Bind() {
    glBindVertexArray(ID);
}

void VertexArrayObject::Unbind() {
    glBindVertexArray(0);
}

void VertexArrayObject::Delete() {
    glDeleteVertexArrays(1, &ID);
}