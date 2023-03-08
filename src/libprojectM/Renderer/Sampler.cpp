#include "Sampler.hpp"

Sampler::Sampler(const GLint wrapMode, const GLint filterMode)
    : m_wrapMode(wrapMode)
    , m_filterMode(filterMode)
{
    glGenSamplers(1, &m_samplerId);
    glSamplerParameteri(m_samplerId, GL_TEXTURE_MIN_FILTER, filterMode);
    glSamplerParameteri(m_samplerId, GL_TEXTURE_MAG_FILTER, filterMode);
    glSamplerParameteri(m_samplerId, GL_TEXTURE_WRAP_S, wrapMode);
    glSamplerParameteri(m_samplerId, GL_TEXTURE_WRAP_T, wrapMode);
}


Sampler::~Sampler()
{
    glDeleteSamplers(1, &m_samplerId);
}

void Sampler::Bind(GLuint unit) const
{
    glBindSampler(unit, m_samplerId);
}

void Sampler::Unbind(GLuint unit)
{
    glBindSampler(unit, 0);
}

auto Sampler::WrapMode() const -> GLint
{
    return m_wrapMode;
}

auto Sampler::FilterMode() const -> GLint
{
    return m_filterMode;
}
