#ifndef VIEW_H
#define VIEW_H

#include "GL/glew.h"
#include <qgl.h>
#include <QTime>
#include <QTimer>
#include <QString>
#include "ResourceLoader.h"
#include "camera.h"

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

    GLuint m_renderFBO;
    GLuint m_beautyPass;

    bool m_mouseDown;
    glm::vec2 m_lastMouse;

private slots:
    void tick();
};

#endif // VIEW_H

