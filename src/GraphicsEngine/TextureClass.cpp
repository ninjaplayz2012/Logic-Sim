#include "Classes/GraphicsEngine/TextureClass.hpp"

Texture::Texture(GLenum TextureType, GLenum TexturePosition, const char *Path, const int DesiredChannels, bool FlipVertically) {
    stbi_set_flip_vertically_on_load(FlipVertically);
    Bytes = stbi_load(Path, &ImgWidth, &ImgHeight, &NumberColorChannels, DesiredChannels);
    
    Position = TexturePosition;
    glGenTextures(1, &ID);
    glActiveTexture(TexturePosition);
    
    Created = true;
}

void Texture::Create(GLenum TextureType, GLenum TexturePosition, const char *Path, const int DesiredChannels, bool FlipVertically) {
    stbi_set_flip_vertically_on_load(FlipVertically);
    Bytes = stbi_load(Path, &ImgWidth, &ImgHeight, &NumberColorChannels, DesiredChannels);
    
    Position = TexturePosition;
    glGenTextures(1, &ID);
    glActiveTexture(TexturePosition);
    
    Created = true;
}

void Texture::SetParameteri(GLenum TextureType, GLenum Parameter, GLint Value) {
    glTexParameteri(TextureType, Parameter, Value);
}

void Texture::SetParameterfv(GLenum TextureType, GLenum Parameter, GLfloat *Value) {
    glTexParameterfv(TextureType, Parameter, Value);
}


void Texture::GenImage2d(GLenum TextureType, GLint Level, GLint InternalColorType, GLint Border, GLenum ColorType) {
    GLenum Format = GL_RGB;
    GLenum InternalFormat = GL_RGB;
    if (NumberColorChannels == 4) {
        Format = GL_RGBA;
        InternalFormat = GL_RGBA;
    } else if (NumberColorChannels == 1) {
        Format = GL_RED;
        InternalFormat = GL_RED;
    }

    glTexImage2D(TextureType, Level, InternalFormat, ImgWidth, ImgHeight, Border, Format, GL_UNSIGNED_BYTE, Bytes);
    glGenerateMipmap(GL_TEXTURE_2D);
}


void Texture::Bind(GLenum TextureType) {
    glBindTexture(TextureType, ID);
}

void Texture::Unbind() {
    stbi_image_free(Bytes);
    Bytes = nullptr;
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Delete() {
    glDeleteTextures(1, &ID);
}