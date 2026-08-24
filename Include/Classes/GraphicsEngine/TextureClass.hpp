#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include "C:/glad/glad.h"
#include "C:/stb/stb_image.h"

class Texture {
    public:
        GLuint ID;
        unsigned char *Bytes;
        int ImgWidth, ImgHeight, NumberColorChannels;
        GLuint Position;
        bool Created = false;
        
        Texture(GLenum TextureType, GLenum TexturePosition, const char *Path, const int DesiredChannels, bool FlipVertically);
        Texture() = default;
        
        void Create(GLenum TextureType, GLenum TexturePosition, const char *Path, const int DesiredChannels, bool FlipVertically);
        
        void SetParameteri(GLenum TextureType, GLenum Parameter, GLint Value);
        void SetParameterfv(GLenum TextureType, GLenum Parameter, GLfloat *Value);
        
        void GenImage2d(GLenum TextureType, GLint Level, GLint InternalColorType, GLint Border, GLenum ColorType);
        
        void Bind(GLenum TextureType);
        void Unbind();
        void Delete();
};

#endif