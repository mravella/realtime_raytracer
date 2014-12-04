/**
 * @file   CamtransCamera.cpp
 *
 * This is the perspective camera class you will need to fill in for the Camtrans lab.  See the
 * lab handout for more details.
 */

#include "CamtransCamera.h"
#include <Settings.h>
#include <assert.h>
#include <QDebug>

CamtransCamera::CamtransCamera()
{
    m_position = glm::vec3(2, 2, 2);
    m_look = glm::vec3(0, -0.5, -0.5);
    m_up = glm::vec3(0, 1, 0);
    m_nearClip = 1.0;
    m_farClip = 30.0;
    m_aspectRatio = 1.0;
    m_heightAngle = glm::radians(60.0);
    CamtransCamera::updateCameraAxes();
}

void CamtransCamera::setAspectRatio(float a)
{
    m_aspectRatio = a;
}

glm::mat4x4 CamtransCamera::getProjectionMatrix() const
{
    float c = -m_nearClip / m_farClip;
    float farInverse = 1.0/m_farClip;
    float tanH = glm::tan(m_heightAngle/2.0);
    glm::mat4x4 unhinging = glm::mat4x4(1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, -1.0/(c+1), c/(c+1), 0, 0, -1.0, 0);
    glm::mat4x4 scaling = glm::mat4x4(farInverse / (m_aspectRatio*tanH), 0, 0, 0, 0, farInverse / tanH, 0, 0, 0, 0, farInverse, 0, 0, 0, 0, 1.0);
    return glm::transpose(scaling * unhinging);
}

glm::mat4x4 CamtransCamera::getViewMatrix() const
{
    glm::mat4x4 rotation = glm::mat4x4(m_u.x, m_u.y, m_u.z, 0, m_v.x, m_v.y, m_v.z, 0, m_w.x, m_w.y, m_w.z, 0, 0, 0, 0, 1.0);
    glm::mat4x4 translation = glm::mat4x4(1.0, 0, 0, -m_position.x, 0, 1.0, 0, -m_position.y, 0, 0, 1.0, -m_position.z, 0, 0, 0, 1.0);

    return glm::transpose(translation * rotation);
}

glm::mat4x4 CamtransCamera::getScaleMatrix() const
{
    float farInverse = 1.0/m_farClip;
    float tanH = glm::tan(m_heightAngle/2.0);
    return glm::transpose(glm::mat4x4(farInverse / (m_aspectRatio*tanH), 0, 0, 0, 0, farInverse / tanH, 0, 0, 0, 0, farInverse, 0, 0, 0, 0, 1.0));
}

glm::mat4x4 CamtransCamera::getPerspectiveMatrix() const
{
    float c = -m_nearClip / m_farClip;
    return glm::transpose(glm::mat4x4(1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, -1.0/(c+1.0), c/(c+1.0), 0, 0, -1.0, 0));
}

glm::vec4 CamtransCamera::getPosition() const
{
    return glm::vec4(m_position, 1.0);
}

glm::vec4 CamtransCamera::getLook() const
{
    return glm::vec4(m_look, 0);
}

glm::vec4 CamtransCamera::getUp() const
{
    return glm::vec4(m_v, 0);
}

float CamtransCamera::getAspectRatio() const
{
    return m_aspectRatio;
}

float CamtransCamera::getHeightAngle() const
{
    return m_heightAngle;
}

void CamtransCamera::orientLook(const glm::vec4 &eye, const glm::vec4 &look, const glm::vec4 &up)
{
    m_position = glm::vec3(eye);
    m_look = glm::vec3(look);
    m_up = glm::vec3(up);
    CamtransCamera::updateCameraAxes();
}

void CamtransCamera::setHeightAngle(float h)
{
    m_heightAngle = glm::radians(h);
}

void CamtransCamera::translate(const glm::vec4 &v)
{
    m_position += glm::vec3(v);
}

void CamtransCamera::rotateU(float degrees)
{
    float rads = glm::radians(degrees);
    glm::vec3 oldv = glm::vec3(m_v);
    m_v = m_v*glm::cos(rads) + m_w*glm::sin(rads);
    m_w = -oldv*glm::sin(rads) + m_w*glm::cos(rads);
}

void CamtransCamera::rotateV(float degrees)
{
    float rads = glm::radians(degrees);
    glm::vec3 oldu = glm::vec3(m_u);
    m_u = m_u*glm::cos(rads) - m_w*glm::sin(rads);
    m_w = oldu*glm::sin(rads) + m_w*glm::cos(rads);
}

void CamtransCamera::rotateW(float degrees)
{
    float rads = glm::radians(degrees);
    glm::vec3 oldu = glm::vec3(m_u);
    m_u = m_v*glm::sin(rads) + m_u*glm::cos(rads);
    m_v = m_v*glm::cos(rads) - oldu*glm::sin(rads);
}

void CamtransCamera::setClip(float nearPlane, float farPlane)
{
    m_nearClip = nearPlane;
    m_farClip = farPlane;
}

void CamtransCamera::updateCameraAxes()
{
    m_w = -glm::normalize(m_look);
    m_v = glm::normalize(m_up - glm::dot(m_up, m_w) * m_w);
    m_u = glm::normalize(glm::cross(m_v, m_w));

    assert(glm::dot(m_u, m_v) <= 0.00001 && glm::dot(m_w, m_v) >= -0.00001);
    assert(glm::dot(m_w, m_u) <= 0.00001 && glm::dot(m_w, m_v) >= -0.00001);
//    qDebug() << m_w.x << m_w.y << m_w.z << m_v.x << m_v.y << m_v.z;
//    qDebug() << glm::dot(m_w, m_v);
    assert(glm::dot(m_w, m_v) <= 0.00001 && glm::dot(m_w, m_v) >= -0.00001);
}

