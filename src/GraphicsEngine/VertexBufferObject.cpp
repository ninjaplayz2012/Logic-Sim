#include "Classes/GraphicsEngine/VertexBufferObject.hpp"

VertexBufferObject::VertexBufferObject(GLfloat* Vertices, GLsizeiptr Size) {
    glGenBuffers(1, &ID);
    
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, Size, Vertices, GL_STATIC_DRAW);
}

VertexBufferObject::VertexBufferObject(GLuint* Vertices, GLsizeiptr Size) {
    glGenBuffers(1, &ID);
    
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, Size, Vertices, GL_STATIC_DRAW);
}

void VertexBufferObject::Create(GLfloat *Vertices, GLsizeiptr Size) {
    glGenBuffers(1, &ID);
    
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, Size, Vertices, GL_STATIC_DRAW);
}

void VertexBufferObject::Create(GLuint *Vertices, GLsizeiptr Size) {
    glGenBuffers(1, &ID);
    
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, Size, Vertices, GL_STATIC_DRAW);
}

void VertexBufferObject::Bind() {
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VertexBufferObject::Unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBufferObject::Delete() {
    glDeleteBuffers(1, &ID);
}