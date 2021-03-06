 #include "view.h"
#include <QApplication>
#include <QKeyEvent>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <QDebug>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;

// Render Pass Numbers
#define BEAUTY_PASS 0
#define AO_PASS 1
#define NORMAL_PASS 2
#define DEPTH_PASS 3
#define AMBIENT_PASS 4
#define DIFFUSE_PASS 5
#define SPEC_PASS 6

// Toggle Key
#define SHADOWS_KEY Qt::Key_A
#define TEXTURE_KEY Qt::Key_S
#define REFLECTIONS_KEY Qt::Key_D
#define AO_KEY Qt::Key_F
#define BUMP_KEY Qt::Key_G
#define DOF_KEY Qt::Key_H
#define FOG_KEY Qt::Key_J

#define PI 3.1415927

View::View(QWidget *parent) : QGLWidget(parent), m_timer(this), m_fps(60.0f), m_increment(0)

{
    // View needs all mouse move events, not just mouse drag events
    setMouseTracking(true);

    // Hide the cursor since this is a fullscreen app
//    setCursor(Qt::BlankCursor);

    // View needs keyboard focus
    setFocusPolicy(Qt::StrongFocus);

    // The game loop is implemented using a timer
    connect(&timer, SIGNAL(timeout()), this, SLOT(tick()));

    // Instantiate camera
    m_camera = Camera();

    m_renderSettings = true;
    m_setting = 5;

    // Defaults for camera
    m_pos = glm::vec4(10.f, 4.1f, 16.f, 1.f);
    m_up = glm::vec4(0.f, 1.f, 0.f, 0.f);
    m_look = glm::vec4(-9.f, -3.2f, -16.f, 0.f);
    m_heightAngle = 45;
    m_count = 0;
    m_focalDepth = 0.4f;

    m_middleMouseDown = false;
    m_rightMouseDown = false;
    m_leftMouseDown = false;

    m_renderPass = BEAUTY_PASS;
    m_shadowsToggle = false;
    m_textureToggle = false;
    m_reflectionsToggle = false;
    m_aoToggle = false;
    m_bumpToggle = false;
    m_dofToggle = false;
    m_fogToggle = false;
    m_paintMode = false;

    m_textures = std::map<string, int>();
    m_scene = 0;

    // Start the timer for updating the screen
    m_timer.start( 1000.0f / m_fps );

    m_lastUpdate = QTime(0,0).msecsTo(QTime::currentTime());
    m_numFrames = 0;

    m_painter = new PainterlyRender();
}

View::~View()
{
}

void View::initializeGL()
{
    QGLFormat glFormat;
    glFormat.setVersion(3, 3);
    glFormat.setProfile(QGLFormat::CoreProfile);

    QGLFormat::setDefaultFormat(glFormat);

    // All OpenGL initialization *MUST* be done during or after this
    // method. Before this method is called, there is no active OpenGL
    // context and all OpenGL calls have no effect.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
      /* Problem: glewInit failed, something is seriously wrong. */
      fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

    m_isInitialized = true;

    // Load the shader
    m_shader = ResourceLoader::loadShaders(":/shaders/shader.vert", ":/shaders/shader.frag");

    GLfloat vertexBufferData[] = {
        -1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f
    };

    // VAO init
    glGenVertexArrays(1, &m_vaoID);
    glBindVertexArray(m_vaoID);

    // Vertex buffer init
    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW);

    // Expose vertices to shader
    glEnableVertexAttribArray(glGetAttribLocation(m_shader, "position"));
    glVertexAttribPointer(
       glGetAttribLocation(m_shader, "position"),
       3,                  // num vertices per element (3 for triangle)
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)0            // array buffer offset
    );

    //Clean up -- unbind things
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);


    m_textures["marble"] = loadTexture("/course/cs123/data/image/marble.png");
    if (m_textures["marble"] == -1)
        cout << "Texture marble does not exist" << endl;

    m_textures["warehouse"] = loadTexture(":/images/warehouse.jpg");
    if (m_textures["warehouse"] == -1)
        cout << "Texture warehouse does not exist" << endl;

    m_textures["rusty_bump"] = loadTexture(":/images/rusty_bump.jpg");
    if (m_textures["rusty_bump"] == -1)
        cout << "Texture rusty bump does not exist" << endl;

    m_textures["rusty_texture"] = loadTexture(":/images/rusty_texture.jpg");
    if (m_textures["rusty_texture"] == -1)
        cout << "Texture rusty texture does not exist" << endl;

    m_textures["rusty_spec"] = loadTexture(":/images/rusty_spec.jpg");
    if (m_textures["rusty_spec"] == -1)
        cout << "Texture rusty spec does not exist" << endl;

    m_textures["circus"] = loadTexture(":/images/circus.jpg");
    if (m_textures["circus"] == -1)
        cout << "Texture circus does not exist" << endl;

    m_textures["plastic"] = loadTexture(":/images/matte_plastic_bump.jpg");
    if (m_textures["plastic"] == -1)
        cout << "Texture plastic does not exist" << endl;

    m_textures["ball_color"] = loadTexture(":/images/ball_texture.jpg");
    if (m_textures["ball_color"] == -1)
        cout << "Texture ball color does not exist" << endl;

    m_textures["apples"] = loadTexture(":/images/apples.jpg");
    if (m_textures["apples"] == -1)
        cout << "Texture apples does not exist" << endl;

    m_textures["skiff"] = loadTexture(":/images/skiff.jpg");
    if (m_textures["skiff"] == -1)
        cout << "Texture skiff does not exist" << endl;

    m_rustyScene = ResourceLoader::loadShaders(":/shaders/shader.vert", ":/shaders/shader.frag");
    m_blurShader = ResourceLoader::loadShaders(":/shaders/shader.vert", ":/shaders/blur.frag");
    m_paintShader = ResourceLoader::loadShaders(":/shaders/shader.vert", ":/shaders/draw.frag");
    m_circusScene = ResourceLoader::loadShaders(":/shaders/shader.vert",":/shaders/refract.frag");
    m_imageScene = ResourceLoader::loadShaders(":/shaders/shader.vert", ":/shaders/image.frag");
    m_superRustyScene = ResourceLoader::loadShaders(":/shaders/shader.vert", ":/shaders/superShader.frag");

    // Start a timer that will try to get 60 frames per second (the actual
    // frame rate depends on the operating system and other running programs)
    time.start();
    timer.start(1000 / 60);

    // Center the mouse, which is explained more in mouseMoveEvent() below.
    // This needs to be done here because the mouse may be initially outside
    // the fullscreen window and will not automatically receive mouse move
    // events. This occurs if there are two monitors and the mouse is on the
    // secondary monitor.
    QCursor::setPos(mapToGlobal(QPoint(width() / 2, height() / 2)));

    // Create framebuffer object
    glGenFramebuffers( 1, &m_renderFBO );
    glBindFramebuffer( GL_FRAMEBUFFER, m_renderFBO );

    glActiveTexture( GL_TEXTURE0 );
    glGenTextures( 1, &m_beautyPass);
    glBindTexture( GL_TEXTURE_2D, m_beautyPass );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_beautyPass, 0);

    glBindFramebuffer( GL_FRAMEBUFFER, 0);

}

void View::paintGL()
{
    if (!m_isInitialized){
        std::cout << "You must call init() before you can draw!" << std::endl;
    } else{
        //draw the painterly render
        if(m_paintMode) {
            if(!m_painted) {
                m_painted = true;

                QList<int>* brushes = new QList<int>();
                brushes->append(32);
                brushes->append(8);
                brushes->append(4);
                brushes->append(1);

                int storedWidth = width();
                int storedHeight = height();
                GLubyte pixelData[storedWidth*storedHeight*4];
                glReadPixels(0, 0, storedWidth, storedHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
                GLubyte* render = m_painter->paintImage(pixelData, storedWidth, storedHeight, brushes);

                QImage image = QImage(render, storedWidth, storedHeight, QImage::Format_ARGB32);
                QImage texture = QGLWidget::convertToGLFormat(image);

    //            glDrawPixels(width(), height(), GL_RGB, GL_UNSIGNED_BYTE, render);

                // Generate a new OpenGL texture ID to put our image into
                GLuint id = 0;
                glGenTextures(1, &id);

                // Make the texture we just created the new active texture
                glBindTexture(GL_TEXTURE_2D, id);

                // Copy the image data into the OpenGL texture
                gluBuild2DMipmaps(GL_TEXTURE_2D, 3, texture.width(), texture.height(), GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

                // Set filtering options
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                // Set coordinate wrapping options
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

                m_shader = m_paintShader;
                glUseProgram(m_shader);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, id);

                glUniform1i(glGetUniformLocation(m_shader, "tex"), 0);
                glUniform1f(glGetUniformLocation(m_shader, "width"), storedWidth);
                glUniform1f(glGetUniformLocation(m_shader, "height"), storedHeight);

                glBindVertexArray(m_vaoID);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glBindVertexArray(0);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glUseProgram(0);
            }
        }
        else {
            m_numFrames++;
            int time = QTime(0,0).msecsTo(QTime::currentTime());

            if (time - m_lastUpdate > 1000) {
                m_currentFPS = (float)m_numFrames / (float)((time - m_lastUpdate)/1000.f);
                m_numFrames = 0;
                m_lastUpdate = time;
            }


            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            if (m_dofToggle && m_renderPass == BEAUTY_PASS)
                glBindFramebuffer(GL_FRAMEBUFFER, m_renderFBO);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            m_camera.orientLook(m_pos, m_look, m_up);
            m_camera.setHeightAngle(m_heightAngle);

            glm::mat4 viewMatrix = m_camera.getViewMatrix();
            m_eye = glm::vec3(glm::inverse(viewMatrix) * glm::vec4(0.0, 0.0, 0.0, 1.0));
            m_filmToWorld = glm::inverse(m_camera.getScaleMatrix() * viewMatrix);

            if (m_scene == 0) {
                m_shader = m_rustyScene;

                glUseProgram(m_shader);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_textures["rusty_texture"]);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, m_textures["rusty_bump"]);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, m_textures["rusty_spec"]);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, m_textures["warehouse"]);

                glUniform1f(glGetUniformLocation(m_shader, "width"), width());
                glUniform1f(glGetUniformLocation(m_shader, "height"), height());
                glUniformMatrix4fv(glGetUniformLocation(m_shader, "filmToWorld"), 1, GL_FALSE, glm::value_ptr(m_filmToWorld));
                glUniform3f(glGetUniformLocation(m_shader, "eye"), m_eye.x, m_eye.y, m_eye.z);
                glUniform1f(glGetUniformLocation(m_shader, "time"), (float) m_count++);
                glUniform1i(glGetUniformLocation(m_shader, "textureMap0"), 0);
                glUniform1i(glGetUniformLocation(m_shader, "bumpMap0"), 1);
                glUniform1i(glGetUniformLocation(m_shader, "specMap0"), 2);
                glUniform1i(glGetUniformLocation(m_shader, "environmentMap"), 3);

                // Settings
                glUniform1i(glGetUniformLocation(m_shader, "renderPass"), m_renderPass);
                glUniform1i(glGetUniformLocation(m_shader, "shadows"), m_shadowsToggle);
                glUniform1i(glGetUniformLocation(m_shader, "textureMapping"), m_textureToggle);
                glUniform1i(glGetUniformLocation(m_shader, "reflections"), m_reflectionsToggle);
                glUniform1i(glGetUniformLocation(m_shader, "ambientOcclusion"), m_aoToggle);
                glUniform1i(glGetUniformLocation(m_shader, "bump"), m_bumpToggle);
                glUniform1i(glGetUniformLocation(m_shader, "dof"), m_dofToggle);
                glUniform1i(glGetUniformLocation(m_shader, "fog"), m_fogToggle);


                glBindVertexArray(m_vaoID);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glBindVertexArray(0);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, 0);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                glUseProgram(0);
            }
            else if (m_scene == 1) {
                m_shader = m_circusScene;

                glUseProgram(m_shader);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_textures["ball_color"]);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, m_textures["plastic"]);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, m_textures["rusty_spec"]);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, m_textures["circus"]);

                glUniform1f(glGetUniformLocation(m_shader, "width"), width());
                glUniform1f(glGetUniformLocation(m_shader, "height"), height());
                glUniformMatrix4fv(glGetUniformLocation(m_shader, "filmToWorld"), 1, GL_FALSE, glm::value_ptr(m_filmToWorld));
                glUniform3f(glGetUniformLocation(m_shader, "eye"), m_eye.x, m_eye.y, m_eye.z);
                glUniform1f(glGetUniformLocation(m_shader, "time"), (float) m_count++);
                glUniform1i(glGetUniformLocation(m_shader, "textureMap0"), 0);
                glUniform1i(glGetUniformLocation(m_shader, "bumpMap0"), 1);
                glUniform1i(glGetUniformLocation(m_shader, "specMap0"), 2);
                glUniform1i(glGetUniformLocation(m_shader, "environmentMap"), 3);

                // Settings
                glUniform1i(glGetUniformLocation(m_shader, "renderPass"), m_renderPass);
                glUniform1i(glGetUniformLocation(m_shader, "shadows"), m_shadowsToggle);
                glUniform1i(glGetUniformLocation(m_shader, "textureMapping"), m_textureToggle);
                glUniform1i(glGetUniformLocation(m_shader, "reflections"), m_reflectionsToggle);
                glUniform1i(glGetUniformLocation(m_shader, "ambientOcclusion"), m_aoToggle);
                glUniform1i(glGetUniformLocation(m_shader, "bump"), m_bumpToggle);
                glUniform1i(glGetUniformLocation(m_shader, "dof"), m_dofToggle);
                glUniform1i(glGetUniformLocation(m_shader, "fog"), m_fogToggle);

                glBindVertexArray(m_vaoID);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glBindVertexArray(0);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, 0);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                glUseProgram(0);
            }
            else if (m_scene >= 8) {
                m_shader = m_imageScene;

                glUseProgram(m_shader);

                glActiveTexture(GL_TEXTURE0);
                if (m_scene == 8)
                    glBindTexture(GL_TEXTURE_2D, m_textures["apples"]);
                if (m_scene == 9)
                    glBindTexture(GL_TEXTURE_2D, m_textures["skiff"]);
                if (m_scene == 10)
                    glBindTexture(GL_TEXTURE_2D, m_textures["ball_color"]);

                glUniform1f(glGetUniformLocation(m_shader, "width"), width());
                glUniform1f(glGetUniformLocation(m_shader, "height"), height());
                glUniform1f(glGetUniformLocation(m_shader, "tex"), 0);

                glBindVertexArray(m_vaoID);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glBindVertexArray(0);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, 0);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                glUseProgram(0);
            }
            if (m_dofToggle && m_renderPass == BEAUTY_PASS) {
                m_shader = m_blurShader;
                glUseProgram(m_shader);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

                glActiveTexture(GL_TEXTURE5);
                glBindTexture(GL_TEXTURE_2D, m_beautyPass);

                glUniform1i(glGetUniformLocation(m_shader, "beautyPass"), 5);
                glUniform1f(glGetUniformLocation(m_shader, "width"), width());
                glUniform1f(glGetUniformLocation(m_shader, "height"), height());
                glUniform1f(glGetUniformLocation(m_shader, "focalDepth"), m_focalDepth);

                glBindVertexArray(m_vaoID);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glBindVertexArray(0);

                glActiveTexture(GL_TEXTURE5);
                glBindTexture(GL_TEXTURE_2D, 0);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glUseProgram(0);
            }

            // QGLWidget's renderText takes xy coordinates, a string, and a font
            if (m_renderSettings) {
                glColor3f(1.0f, 1.0f, 1.0f);
                this->renderText( 10, 20, "FPS: " + QString::number(((int)(m_currentFPS*10.0)/10.0)));
            }
        }
    }
}

void View::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    m_camera.setAspectRatio((float) width() / (float) height());

    glGenFramebuffers( 1, &m_renderFBO );
    glBindFramebuffer( GL_FRAMEBUFFER, m_renderFBO );

    glActiveTexture( GL_TEXTURE0 );
    glGenTextures( 1, &m_beautyPass);
    glBindTexture( GL_TEXTURE_2D, m_beautyPass );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_beautyPass, 0);

    glBindFramebuffer( GL_FRAMEBUFFER, 0);

}

void View::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        m_middleMouseDown = true;
        m_lastMouse = glm::vec2(event->x(), event->y());
    }
    if (event->button() == Qt::RightButton) {
        m_rightMouseDown = true;
        m_lastMouse = glm::vec2(event->x(), event->y());
    }
    if (event->button() == Qt::LeftButton) {
        m_lastMouse = glm::vec2(event->x(), event->y());
        m_leftMouseDown = true;
    }
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    // This starter code implements mouse capture, which gives the change in
    // mouse position since the last mouse movement. The mouse needs to be
    // recentered after every movement because it might otherwise run into
    // the edge of the screen, which would stop the user from moving further
    // in that direction. Note that it is important to check that deltaX and
    // deltaY are not zero before recentering the mouse, otherwise there will
    // be an infinite loop of mouse move events.
//    int deltaX = event->x() - width() / 2;
//    int deltaY = event->y() - height() / 2;
//    if (!deltaX && !deltaY) return;
//    QCursor::setPos(mapToGlobal(QPoint(width() / 2, height() / 2)));

    // TODO: Handle mouse movements here
    glm::vec2 mouseChange = glm::vec2(event->x(), event->y()) - m_lastMouse;

    if (m_middleMouseDown) {
        m_pos += glm::vec4(-mouseChange.x * glm::normalize(glm::cross(glm::vec3(m_camera.getUp()), glm::vec3(m_look))) - mouseChange.y * glm::normalize(glm::vec3(m_camera.getUp())), 0.0) / 500.f;
    }

    if (m_leftMouseDown) {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(), -mouseChange.x * 0.005f, glm::vec3(m_camera.getUp()));
        m_look = rotationMatrix * m_look;
        rotationMatrix = glm::rotate(glm::mat4(), mouseChange.y * 0.005f, glm::cross(glm::vec3(m_camera.getUp()), glm::vec3(m_look)));
        m_look = rotationMatrix * m_look;
    }

    if (m_rightMouseDown) {
        glm::vec4 translate = m_look * .001f * mouseChange.x;
        m_pos += translate;
    }
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
        m_middleMouseDown = false;
    if (event->button() == Qt::RightButton)
        m_rightMouseDown = false;
    if (event->button() == Qt::LeftButton) {
        m_leftMouseDown = false;
    }
}

void View::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) QApplication::quit();

    // Scenes
    if (event->key() == Qt::Key_1) {
        m_renderSettings = !m_renderSettings;
    }
    if (event->key() == Qt::Key_2) {
        m_scene = 0;
    }
    if (event->key() == Qt::Key_3) {
        m_scene = 1;
    }
    if (event->key() == Qt::Key_8) {
        m_scene = 8;
    }
    if (event->key() == Qt::Key_9) {
        m_scene= 9;
    }
    if (event->key() == Qt::Key_0) {
        m_scene = 10;
    }

    // Painterly Render
    if (event->key() == Qt::Key_P) {
        m_paintMode = !m_paintMode;
        m_painted = false;
    }

    // Render Passes
    if (event->key() == Qt::Key_Q)
        m_renderPass = BEAUTY_PASS;
    if (event->key() == Qt::Key_W)
        m_renderPass = AO_PASS;
    if (event->key() == Qt::Key_E)
        m_renderPass = NORMAL_PASS;
    if (event->key() == Qt::Key_R)
        m_renderPass = DEPTH_PASS;
    if (event->key() == Qt::Key_T)
        m_renderPass = AMBIENT_PASS;
    if (event->key() == Qt::Key_Y)
        m_renderPass = DIFFUSE_PASS;
    if (event->key() == Qt::Key_U)
        m_renderPass = SPEC_PASS;

    // Toggles
    if (event->key() == Qt::Key_A)
        m_shadowsToggle = !m_shadowsToggle;
    if (event->key() == Qt::Key_S)
        m_textureToggle = !m_textureToggle;
    if (event->key() == Qt::Key_D)
        m_reflectionsToggle = !m_reflectionsToggle;
    if (event->key() == Qt::Key_F)
        m_aoToggle = !m_aoToggle;
    if (event->key() == Qt::Key_G)
        m_bumpToggle = !m_bumpToggle;
    if (event->key() == Qt::Key_H)
        m_dofToggle = !m_dofToggle;
    if (event->key() == Qt::Key_J)
        m_fogToggle = !m_fogToggle;

    if (event->key() == Qt::Key_Up)
        if (m_focalDepth < 1.0f) m_focalDepth += 0.05f;
    if (event->key() == Qt::Key_Down)
        if (m_focalDepth > 0.0f) m_focalDepth -= 0.05f;

    if (event->key() == Qt::Key_Space)
    {
        m_pos = glm::vec4(10.f, 4.1f, 16.f, 1.f);
        m_up = glm::vec4(0.f, 1.f, 0.f, 0.f);
        m_look = glm::vec4(-9.f, -3.2f, -16.f, 0.f);
        m_heightAngle = 45;
    }
}


void View::keyReleaseEvent(QKeyEvent *event)
{
}

void View::tick()
{
    // Get the number of seconds since the last tick (variable update rate)
    float seconds = time.restart() * 0.001f;

    update();
}

int View::loadTexture(const QString &filename)
{
    // Make sure the image file exists
    QFile file(filename);
    if (!file.exists())
        return -1;

    // Load the file into memory
    QImage image;
    image.load(file.fileName());
    image = image.mirrored(false, true);
    QImage texture = QGLWidget::convertToGLFormat(image);

    // Generate a new OpenGL texture ID to put our image into
    GLuint id = 0;
    glGenTextures(1, &id);

    // Make the texture we just created the new active texture
    glBindTexture(GL_TEXTURE_2D, id);

    // Copy the image data into the OpenGL texture
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, texture.width(), texture.height(), GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

    // Set filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Set coordinate wrapping options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    return id;
}
