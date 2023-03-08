#include "TextureManager.hpp"
#include "Common.hpp"
#include "IdleTextures.hpp"
#include "SOIL2/SOIL2.h"
#include "Texture.hpp"
#include "projectM-opengl.h"
#include <algorithm>
#include <memory>
#include <vector>

#include "MilkdropPreset/MilkdropNoise.hpp"

// Missing in macOS SDK. Query will most certainly fail, but then use the default format.
#ifndef GL_TEXTURE_IMAGE_FORMAT
#define GL_TEXTURE_IMAGE_FORMAT 0x828F
#endif


TextureManager::TextureManager(std::vector<std::string>& textureSearchPaths)
    : m_textureDirectories(textureSearchPaths)
{
    FileScanner fileScanner = FileScanner(m_textureDirectories, m_extensions);

    // scan for textures
    using namespace std::placeholders;
    fileScanner.scan(std::bind(&TextureManager::loadTexture, this, _1, _2));

    Preload();

    auto noise = std::make_unique<MilkdropNoise>();

#if !USE_GLES
    // Query preferred internal texture format. GLES 3 only supports GL_RENDERBUFFER here, no texture targets.
    // That's why we use GL_BGRA as default, as this is the best general-use format according to Khronos.
    GLint preferredInternalFormat{GL_BGRA};
    glGetInternalformativ(GL_TEXTURE_2D, GL_RGBA8, GL_TEXTURE_IMAGE_FORMAT, sizeof(preferredInternalFormat), &preferredInternalFormat);
#else
    // GLES only supports GL_RGB and GL_RGBA, so we always use the latter.
    GLint preferredInternalFormat{GL_RGBA};
#endif

    GLuint noise_texture_lq_lite;
    glGenTextures(1, &noise_texture_lq_lite);
    glBindTexture(GL_TEXTURE_2D, noise_texture_lq_lite);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 32, 32, 0, preferredInternalFormat, GL_UNSIGNED_BYTE, noise->noise_lq_lite);
    Texture* textureNoise_lq_lite = new Texture("noise_lq_lite", noise_texture_lq_lite, GL_TEXTURE_2D, 32, 32, false);
    textureNoise_lq_lite->Sampler(GL_REPEAT, GL_LINEAR);
    m_textures["noise_lq_lite"] = textureNoise_lq_lite;

    GLuint noise_texture_lq;
    glGenTextures(1, &noise_texture_lq);
    glBindTexture(GL_TEXTURE_2D, noise_texture_lq);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, preferredInternalFormat, GL_UNSIGNED_BYTE, noise->noise_lq);
    Texture* textureNoise_lq = new Texture("noise_lq", noise_texture_lq, GL_TEXTURE_2D, 256, 256, false);
    textureNoise_lq->Sampler(GL_REPEAT, GL_LINEAR);
    m_textures["noise_lq"] = textureNoise_lq;

    GLuint noise_texture_mq;
    glGenTextures(1, &noise_texture_mq);
    glBindTexture(GL_TEXTURE_2D, noise_texture_mq);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, preferredInternalFormat, GL_UNSIGNED_BYTE, noise->noise_mq);
    Texture* textureNoise_mq = new Texture("noise_mq", noise_texture_mq, GL_TEXTURE_2D, 256, 256, false);
    textureNoise_mq->Sampler(GL_REPEAT, GL_LINEAR);
    m_textures["noise_mq"] = textureNoise_mq;

    GLuint noise_texture_hq;
    glGenTextures(1, &noise_texture_hq);
    glBindTexture(GL_TEXTURE_2D, noise_texture_hq);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, preferredInternalFormat, GL_UNSIGNED_BYTE, noise->noise_hq);
    Texture* textureNoise_hq = new Texture("noise_hq", noise_texture_hq, GL_TEXTURE_2D, 256, 256, false);
    textureNoise_hq->Sampler(GL_REPEAT, GL_LINEAR);
    m_textures["noise_hq"] = textureNoise_hq;

    GLuint noise_texture_lq_vol;
    glGenTextures(1, &noise_texture_lq_vol);
    glBindTexture(GL_TEXTURE_3D, noise_texture_lq_vol);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 32, 32, 32, 0, preferredInternalFormat, GL_UNSIGNED_BYTE, noise->noise_lq_vol);
    Texture* textureNoise_lq_vol = new Texture("noisevol_lq", noise_texture_lq_vol, GL_TEXTURE_3D, 32, 32, false);
    textureNoise_lq_vol->Sampler(GL_REPEAT, GL_LINEAR);
    m_textures["noisevol_lq"] = textureNoise_lq_vol;

    GLuint noise_texture_hq_vol;
    glGenTextures(1, &noise_texture_hq_vol);
    glBindTexture(GL_TEXTURE_3D, noise_texture_hq_vol);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 32, 32, 32, 0, preferredInternalFormat, GL_UNSIGNED_BYTE, noise->noise_hq_vol);

    Texture* textureNoise_hq_vol = new Texture("noisevol_hq", noise_texture_hq_vol, GL_TEXTURE_3D, 32, 32, false);
    textureNoise_hq_vol->Sampler(GL_REPEAT, GL_LINEAR);
    m_textures["noisevol_hq"] = textureNoise_hq_vol;
}

TextureManager::~TextureManager()
{
    Clear();
}

void TextureManager::Preload()
{
    int width, height;

    unsigned int tex = SOIL_load_OGL_texture_from_memory(
        M_data,
        M_bytes,
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MULTIPLY_ALPHA, &width, &height);


    Texture* newTex = new Texture("m", tex, GL_TEXTURE_2D, width, height, true);
    newTex->Sampler(GL_CLAMP_TO_EDGE, GL_LINEAR);
    m_textures["m"] = newTex;

    //    tex = SOIL_load_OGL_texture_from_memory(
    //                project_data,
    //                project_bytes,
    //                SOIL_LOAD_AUTO,
    //                SOIL_CREATE_NEW_ID,
    //                SOIL_FLAG_POWER_OF_TWO
    //                |  SOIL_FLAG_MULTIPLY_ALPHA
    //                ,&width,&height);

    //    newTex = new Texture("project", tex, GL_TEXTURE_2D, width, height, true);
    //    newTex->getSampler(GL_CLAMP_TO_EDGE, GL_LINEAR);
    //    textures["project"] = newTex;

    tex = SOIL_load_OGL_texture_from_memory(
        headphones_data,
        headphones_bytes,
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MULTIPLY_ALPHA, &width, &height);

    newTex = new Texture("headphones", tex, GL_TEXTURE_2D, width, height, true);
    newTex->Sampler(GL_CLAMP_TO_EDGE, GL_LINEAR);
    m_textures["headphones"] = newTex;
}

void TextureManager::Clear()
{
    for (std::map<std::string, Texture*>::const_iterator iter = m_textures.begin(); iter != m_textures.end(); iter++)
        delete (iter->second);

    m_textures.clear();
}


TextureSamplerDesc TextureManager::getTexture(const std::string fullName, const GLenum defaultWrap, const GLenum defaultFilter)
{
    std::string fileName = fullName;
    std::string unqualifiedName;
    GLint wrap_mode;
    GLint filter_mode;

    // Remove extension
    std::string lowerCaseFileName(fullName);
    std::transform(lowerCaseFileName.begin(), lowerCaseFileName.end(), lowerCaseFileName.begin(), tolower);
    for (auto ext : m_extensions)
    {
        size_t found = lowerCaseFileName.find(ext);
        if (found != std::string::npos)
        {
            fileName.replace(int(found), ext.size(), "");
            break;
        }
    }

    ExtractTextureSettings(fileName, wrap_mode, filter_mode, unqualifiedName);
    if (m_textures.find(unqualifiedName) == m_textures.end())
    {
        return TextureSamplerDesc(NULL, NULL);
    }

    if (fileName == unqualifiedName)
    {
        // Warp and filter mode not specified in sampler name
        // applying default
        wrap_mode = defaultWrap;
        filter_mode = defaultFilter;
    }

    Texture* texture = m_textures[unqualifiedName];
    const auto* sampler = &texture->Sampler(wrap_mode, filter_mode);

    return TextureSamplerDesc(texture, sampler);
}


TextureSamplerDesc TextureManager::tryLoadingTexture(const std::string name)
{
    TextureSamplerDesc texDesc;
    GLint wrapMode{0};
    GLint filterMode{0};
    std::string unqualifiedName;

    ExtractTextureSettings(name, wrapMode, filterMode, unqualifiedName);

    // Search order:
    // 1. User preset dir
    // 2. User texture dir
    // 3. System/default preset dir
    // 4. System/default texture dir
    for (const auto& path : m_textureDirectories)
    {
        std::string fullPath = path;
        fullPath += pathSeparator;
        fullPath += unqualifiedName;

        for (const auto& ext : m_extensions)
        {
            texDesc = loadTexture(fullPath + ext, name);

            if (texDesc.first != NULL)
            {
                std::cerr << "Located texture " << name << std::endl;
                return texDesc;
            }
        }
    }

    std::cerr << "Failed to locate texture " << name << std::endl;

    return {};
}

TextureSamplerDesc TextureManager::loadTexture(const std::string fileName, const std::string name)
{
    int width, height;
    //    std::cout << "Loading texture " << name << " at " << fileName << std::endl;

    unsigned int tex = SOIL_load_OGL_texture(
        fileName.c_str(),
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MULTIPLY_ALPHA, &width, &height);

    if (tex == 0)
    {
        return TextureSamplerDesc(NULL, NULL);
    }

    GLint wrap_mode;
    GLint filter_mode;
    std::string unqualifiedName;

    ExtractTextureSettings(name, wrap_mode, filter_mode, unqualifiedName);
    Texture* newTexture = new Texture(unqualifiedName, tex, GL_TEXTURE_2D, width, height, true);
    const auto* sampler = &newTexture->Sampler(wrap_mode, filter_mode);

    if (m_textures.find(name) != m_textures.end())
    {
        // found duplicate.. this could be optimized
        delete m_textures[name];
    }

    m_textures[name] = newTexture;
    //    std::cout << "Loaded texture " << name << std::endl;

    return TextureSamplerDesc(newTexture, sampler);
}

TextureSamplerDesc TextureManager::getRandomTextureName(std::string random_id)
{
    GLint wrap_mode;
    GLint filter_mode;
    std::string unqualifiedName;

    ExtractTextureSettings(random_id, wrap_mode, filter_mode, unqualifiedName);

    std::vector<std::string> user_texture_names;
    size_t separator = unqualifiedName.find('_');
    std::string textureNameFilter;

    if (separator != std::string::npos)
    {
        textureNameFilter = unqualifiedName.substr(separator + 1);
        unqualifiedName = unqualifiedName.substr(0, separator);
    }

    for (auto& texture : m_textures)
    {
        if (texture.second->IsUserTexture())
        {
            if (textureNameFilter.empty() || texture.first.find(textureNameFilter) == 0)
            {
                user_texture_names.push_back(texture.first);
            }
        }
    }

    if (!user_texture_names.empty())
    {
        std::string random_name = user_texture_names[rand() % user_texture_names.size()];
        m_randomTextures.push_back(random_id);

        auto* randomTexture = m_textures[random_name];
        const auto* sampler = &randomTexture->Sampler(wrap_mode, filter_mode);

        return TextureSamplerDesc(randomTexture, sampler);
    }

    return TextureSamplerDesc(nullptr, nullptr);
}

void TextureManager::clearRandomTextures()
{
    for (std::vector<std::string>::iterator pos = m_randomTextures.begin(); pos != m_randomTextures.end(); ++pos)
    {
        m_textures.erase(*pos);
    }
    m_randomTextures.clear();
}

void TextureManager::ExtractTextureSettings(const std::string qualifiedName, GLint& _wrap_mode, GLint& _filter_mode, std::string& name)
{
    std::string lowerQualifiedName(qualifiedName);
    std::transform(lowerQualifiedName.begin(), lowerQualifiedName.end(), lowerQualifiedName.begin(), tolower);

    _wrap_mode = GL_REPEAT;
    _filter_mode = GL_LINEAR;

    if (lowerQualifiedName.substr(0, 3) == "fc_")
    {
        name = qualifiedName.substr(3);
        _filter_mode = GL_LINEAR;
        _wrap_mode = GL_CLAMP_TO_EDGE;
    }
    else if (lowerQualifiedName.substr(0, 3) == "fw_")
    {
        name = qualifiedName.substr(3);
        _filter_mode = GL_LINEAR;
        _wrap_mode = GL_REPEAT;
    }
    else if (lowerQualifiedName.substr(0, 3) == "pc_")
    {
        name = qualifiedName.substr(3);
        _filter_mode = GL_NEAREST;
        _wrap_mode = GL_CLAMP_TO_EDGE;
    }
    else if (lowerQualifiedName.substr(0, 3) == "pw_")
    {
        name = qualifiedName.substr(3);
        _filter_mode = GL_NEAREST;
        _wrap_mode = GL_REPEAT;
    }
    else
    {
        name = qualifiedName;
    }
}
