/**
 * @file   Camera.cpp
 *
 * This is the perspective camera class you will need to fill in for the Camtrans lab.  See the
 * lab handout for more details.
 */

#include "camera.h"

Camera::Camera()
{
    m_aspect = 1.0f;
    m_near = 1.0f;
    m_far = 60.0f;
    m_height = M_PI / 6.0f;
    m_viewMatrix = glm::mat4();
    m_projectionMatrix = glm::mat4();
    m_look = glm::vec4();
    m_up = glm::vec4();
    m_u = glm::vec4();
    m_v = glm::vec4();
    m_w = glm::vec4();
    m_eye = glm::vec4(0.0, 0.0, 0.0, 1.0);
    this->orientLook(glm::vec4(0.5, 0.5, 0.5, 1.0), glm::vec4(-1.0, -1.0, -1.0, 0.0), glm::vec4(0.0, 1.0, 0.0, 0.0));
    this->updateProjection();
}

void Camera::setAspectRatio(float a)
{
    m_aspect = a;
    this->updateProjection();

}

glm::mat4x4 Camera::getProjectionMatrix() const
{
    return m_projectionMatrix;
}

glm::mat4x4 Camera::getViewMatrix() const
{
    return m_viewMatrix;
}

glm::mat4x4 Camera::getScaleMatrix() const
{
    float tanH = tanf(m_height / 2.0f);
    float tanW = m_aspect * tanH;

    float widthScale = 1.0f / (m_far * tanW);
    float heightScale = 1.0f / (m_far * tanH);
    float farScale = 1.0f / m_far;

    return glm::mat4x4(widthScale,         0.0,      0.0, 0.0,
                       0.0,        heightScale,      0.0, 0.0,
                       0.0,                0.0, farScale, 0.0,
                       0.0,                0.0,      0.0, 1.0);
}

glm::mat4x4 Camera::getPerspectiveMatrix() const
{

    float c = -m_near / m_far;
    return glm::mat4(1.0, 0.0,              0.0,  0.0,
                     0.0, 1.0,              0.0,  0.0,
                     0.0, 0.0, -1.0 / (c + 1.0),   -1,
                     0.0, 0.0,    c / (c + 1.0),    0);
}

glm::vec4 Camera::getPosition() const
{
    return m_eye;
}

glm::vec4 Camera::getLook() const
{
    return m_look;
}

glm::vec4 Camera::getUp() const
{
    return m_v;
}

float Camera::getAspectRatio() const
{
    return m_aspect;
}

float Camera::getHeightAngle() const
{
    return m_height;
}

void Camera::orientLook(const glm::vec4 &eye, const glm::vec4 &look, const glm::vec4 &up)
{
    m_eye = eye;
    m_look = glm::normalize(look);
    m_up = glm::normalize(up);
    m_w = glm::normalize(-m_look);
    m_v = glm::normalize((m_up - (glm::dot(m_w, m_up) * m_w)));
    m_u = glm::normalize(glm::vec4(glm::cross(glm::vec3(m_v), glm::vec3(m_w)), 0.0));
    this->updateView();

}


void Camera::setHeightAngle(float h)
{
    m_height = glm::radians(h);
    this->updateProjection();
}

void Camera::translate(const glm::vec4 &v)
{
    glm::mat4 translationMatrix = glm::transpose(glm::mat4(1.0, 0.0, 0.0, v.x,
                                                           0.0, 1.0, 0.0, v.y,
                                                           0.0, 0.0, 1.0, v.z,
                                                           0.0, 0.0, 0.0, 1.0));
    m_eye = translationMatrix * m_eye;
    this->updateView();
}

void Camera::rotateU(float degrees)
{
    float radians = M_PI / 180.0f * degrees;
    glm::vec4 v = m_v;
    glm::vec4 w = m_w;
    m_v = v * (float) cosf(radians) + w * (float) sinf(radians);
    m_w = - v * (float) sinf(radians) + w * (float) cosf(radians);
    m_look = -m_w;
    this->updateView();
}

void Camera::rotateV(float degrees)
{
    float radians = M_PI / 180.0f * degrees;
    glm::vec4 u = m_u;
    glm::vec4 w = m_w;
    m_u = u * (float) cos(radians) - w * (float) sin(radians);
    m_w = u * (float) sin(radians) + w * (float) cos(radians);
    m_look = -m_w;
    this->updateView();
}

void Camera::rotateW(float degrees)
{
    float radians = M_PI / 180.0f * degrees;
    glm::vec4 u = m_u;
    glm::vec4 v = m_v;
    m_u = v * (float) sin(radians) + u * (float) cos(radians);
    m_v = v * (float) cos(radians) - u * (float) sin(radians);
    this->updateView();
}

void Camera::setClip(float nearPlane, float farPlane)
{
    m_near = nearPlane;
    m_far = farPlane;
    this->updateProjection();
}

void Camera::updateProjection()
{
    glm::mat4 normalizing = this->getScaleMatrix();
    float c = -m_near / m_far;
    glm::mat4 unhinging = glm::mat4(1.0, 0.0,              0.0,   0.0,
                                    0.0, 1.0,              0.0,   0.0,
                                    0.0, 0.0, -1.0 / (c + 1.0), -1,
                                    0.0, 0.0,   c / (c + 1.0),    0);
    m_projectionMatrix = unhinging * normalizing;

}

void Camera::updateView()
{
    glm::mat4 translation = glm::transpose(glm::mat4(1.0, 0.0, 0.0, -m_eye.x,
                                                     0.0, 1.0, 0.0, -m_eye.y,
                                                     0.0, 0.0, 1.0, -m_eye.z,
                                                     0.0, 0.0, 0.0,      1.0));
    glm::mat4 rotation = glm::mat4(m_u.x, m_v.x, m_w.x,   0,
                                   m_u.y, m_v.y, m_w.y,   0,
                                   m_u.z, m_v.z, m_w.z,   0,
                                       0,     0,     0, 1.0);
    m_viewMatrix = rotation * translation;


}
