#ifndef PAINTERLYRENDER_H
#define PAINTERLYRENDER_H
#include "GL/glew.h"
#include <qgl.h>
#include <QImage>
#include <glm/glm.hpp>

// A structure for a color.  Each channel is 8 bits [0-255].
struct BGRA
{
    BGRA() : b(0), g(0), r(0), a(0) {}
    BGRA(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255)
        : b(blue), g(green), r(red), a(alpha) {}

    // C++ TIP:
    // A union struct. Essentially, this makes b <==> channels[0],
    // g <==> channels[1], etc. In other words: b, g, r, and a are
    // stored at the same memory location as channels[4]. Note that
    // sizeof(b)+sizeof(g)+sizeof(r)+sizeof(a) = sizeof(channels)
    // so the memory overlaps **exactly**.
    //
    // You might want to take advantage of union structs in later
    // assignments, although we don't require or expect you to do so.
    //
    union {
        struct { unsigned char b, g, r, a; };
        unsigned char channels[4];
    };

    // @TODO: [OPTIONAL] You can implement some operators here for color arithmetic.
    bool isEmpty() {
        return r==1 && g==2 && b==3 && a==4;
    }

};

class PainterlyRender
{
public:
    PainterlyRender();
    virtual ~PainterlyRender();
    GLubyte* paintImage(GLubyte* imageData, int width, int height);

protected:
    BGRA* m_canvasTexture;
    int m_texWidth;
    int m_texHeight;

    int m_dataWidth;
    int m_dataHeight;

    GLubyte* m_data;

    GLubyte* getBlur(GLubyte* imageData, int radius);
    void paintLayer(GLubyte* resultCanvas, GLubyte* blurredCanvas, int brush);
    void makeStroke(GLubyte* canvas, float zVals[], glm::vec2 best, int brush);

    static BGRA* scaleImage(BGRA* image, int oldW, int oldH, int newW, int newH);
    static BGRA* staticScalePass(int newWidth, int newHeight, bool horizontal, BGRA* image, int oldWidth, int oldHeight, double a, double radius);
    static double staticTriangleVal(double x, double radius);

    GLubyte* pass1DFilter(GLubyte* data, int dataWidth, int dataHeight, double *filter, int filterWidth, bool horizontal);

};

#endif // PAINTERLYRENDER_H
