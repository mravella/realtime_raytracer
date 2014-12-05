#include "view.h"
#include <QApplication>
#include <QKeyEvent>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
using namespace std;

View::View(QWidget *parent) : QGLWidget(parent)
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

}

View::~View()
{
}

void View::initializeGL()
{
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


    m_textureId = loadTexture("/course/cs123/data/image/biker.jpg");
    if (m_textureId == -1)
        cout << "Texture does not exist" << endl;

    m_bg = loadTexture("/course/cs123/data/image/boat.jpg");
    if (m_bg == -1)
        cout << "Texure does not exist" << endl;


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
}

void View::paintGL()
{
    if (!m_isInitialized){
        std::cout << "You must call init() before you can draw!" << std::endl;
    } else{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glUseProgram(m_shader);


//        m_pos = glm::vec4(-6.0f, 4.0f, 4.0f, 1.f);
//        m_up = glm::vec4(0.f, 1.f, 0.f, 0.f);
//        m_look = glm::vec4(6.f, -4.f, -4.f, 0.f);
//        m_heightAngle = 30;

        m_camera.orientLook(m_pos, m_look, m_up);
        m_camera.setHeightAngle(m_heightAngle);

        glm::mat4 viewMatrix = m_camera.getViewMatrix();
        m_eye = glm::vec3(glm::inverse(viewMatrix) * glm::vec4(0.0, 0.0, 0.0, 1.0));
        m_filmToWorld = glm::inverse(m_camera.getScaleMatrix() * viewMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_bg);
        glUniform1f(glGetUniformLocation(m_shader, "width"), width());
        glUniform1f(glGetUniformLocation(m_shader, "height"), height());
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "filmToWorld"), 1, GL_FALSE, glm::value_ptr(m_filmToWorld));
        glUniform3f(glGetUniformLocation(m_shader, "eye"), m_eye.x, m_eye.y, m_eye.z);
        glUniform1i(glGetUniformLocation(m_shader, "settings"), m_setting);
        glUniform1f(glGetUniformLocation(m_shader, "time"), (float) m_count++);
        glUniform1i(glGetUniformLocation(m_shader, "textureSampler"), 0);
        glUniform1i(glGetUniformLocation(m_shader, "bg"), 1);


        glBindVertexArray(m_vaoID);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);


        if (m_renderSettings) {
            glColor3f(1.0f, 1.0f, 1.0f);
            int x = rand() % 100;
            if (x > 76)
                this->renderText(10, 18, "fps 60.1");
            else if (x < 34)
                this->renderText(10, 18, "fps 59.7");
            else
                this->renderText(10, 18, "fps 60.0");
            this->renderText(10, height() - 66, "(1) Toggle Settings");
            this->renderText(10, height() - 52, "(2) Purple");
            this->renderText(10, height() - 38, "(3) Light Spheres");
            this->renderText(10, height() - 24, "(4) No Reflections");
            this->renderText(10, height() - 10, "(5) Recursive Spheres Animation");
        }
    }
}

void View::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    m_camera.setAspectRatio((float) width() / (float) height());
}

void View::mousePressEvent(QMouseEvent *event)
{
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
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
}

void View::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) QApplication::quit();

    // TODO: Handle keyboard presses here
    if (event->key() == Qt::Key_1) {
        m_renderSettings = !m_renderSettings;
    }
    if (event->key() == Qt::Key_2) {
        m_setting = 2;
    }
    if (event->key() == Qt::Key_3) {
        m_shader = ResourceLoader::loadShaders(":/shaders/shader.vert", "/gpfs/main/home/mravella/course/cs123_final/src/shaders/grid.frag");
        m_setting = 3;
    }
    if (event->key() == Qt::Key_4) {
        m_setting = 4;
    }
    if (event->key() == Qt::Key_5) {
        m_shader = ResourceLoader::loadShaders(":/shaders/shader.vert", ":/shaders/shader.frag");
        m_setting = 5;
    }
    if (event->key() == Qt::Key_Up) {
        m_pos.x += 1.0f;
        m_pos.y += 1.0f;
        m_pos.z += 1.0f;
    }
    if (event->key() == Qt::Key_Down) {
        m_pos.x -= 1.0f;
        m_pos.y -= 1.0f;
        m_pos.z -= 1.0f;
    }
}


void View::keyReleaseEvent(QKeyEvent *event)
{
}

void View::tick()
{
    // Get the number of seconds since the last tick (variable update rate)
    float seconds = time.restart() * 0.001f;

    // TODO: Implement the demo update here

    // Flag this view for repainting (Qt will call paintGL() soon after)
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
