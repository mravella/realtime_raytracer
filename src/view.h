#ifndef VIEW_H
#define VIEW_H

#include "GL/glew.h"
#include <qgl.h>
#include <QTime>
#include <QTimer>
#include <QString>
#include "ResourceLoader.h"
#include "camera.h"
#include <string.h>

class View : public QGLWidget
{
    Q_OBJECT

public:
    View(QWidget *parent);
    ~View();

private:
    QTime time;
    QTimer timer;

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    int loadTexture(const QString &filename);
    void createBlurKernel(int, int, int, GLfloat*, GLfloat*);

    GLuint m_shader;
    GLuint m_vaoID;
    bool m_isInitialized;

    bool m_renderSettings;
    int  m_setting;

    glm::vec3 m_eye;
    glm::mat4 m_filmToWorld;
    Camera m_camera;
    glm::vec4 m_pos, m_look, m_up;
    float m_heightAngle;
    int m_count;
    int m_textureId;
    int m_bg;
    int m_noise;
    GLint m_arraySize;
    GLfloat *m_kernel;
    GLfloat *m_offsets;

    float m_focalDepth;

    GLuint m_renderFBO;
    GLuint m_beautyPass;

    bool m_middleMouseDown;
    bool m_rightMouseDown;
    bool m_leftMouseDown;
    glm::vec2 m_lastMouse;

    std::map<std::string, GLuint> m_shaders;

private slots:
    void tick();
};

#endif // VIEW_H

