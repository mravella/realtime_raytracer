#include "painterlyrender.h"
#include <QDebug>
#include <iostream>
#define VAR_E 2.71828182845904523536
#define VAR_PI 3.14159265359

PainterlyRender::PainterlyRender()
{
        QImage* canvasTextureImage = new QImage(QString::fromStdString(":/images/canvas_texture.jpg"));
        //ensure format is right
        if(canvasTextureImage->format() != QImage::Format_RGB32) {
            QImage *old = canvasTextureImage;
            canvasTextureImage = new QImage(old->convertToFormat(QImage::Format_RGB32));
            delete old;
        }

        m_canvasTexture = (BGRA*) canvasTextureImage->bits();
        m_texWidth = canvasTextureImage->width();
        m_texHeight = canvasTextureImage->height();
}

PainterlyRender::~PainterlyRender()
{
    delete[] m_canvasTexture;
}

GLubyte* PainterlyRender::paintImage(GLubyte* imageData, int width, int height, QList<int>* brushes)
{
    m_data = imageData;

    m_dataWidth = width;
    m_dataHeight = height;

//    m_depthData = new GLubyte[width*height*4];
//    memcpy(m_depthData, imageData, width*height*4);

//    for(int i=0; i<width*height; i++) {
//        if(imageData[i*4+3] < 255)
//        qDebug() << imageData[i*4+3];
//    }

    GLubyte* resultCanvas = new GLubyte[m_dataWidth*m_dataHeight*4];

    for(int i=0; i<brushes->size(); i++) {
        int brush = brushes->at(i);
        GLubyte* blurredCanvas = PainterlyRender::getBlur(imageData, brush*1); //<-- increasing this number drastically increases runtime
        PainterlyRender::paintLayer(resultCanvas, blurredCanvas, brush);
    }

//    //apply the canvas filter
    BGRA* scaledCanvas = PainterlyRender::scaleImage(m_canvasTexture, m_texWidth, m_texHeight, m_dataWidth, m_dataHeight);
    for(int i=0; i<m_dataWidth; i++) {
        for(int j=0; j<m_dataHeight; j++) {
            int index = j*m_dataWidth + i;
            //multiply in the canvas texture
            int lightener = 50;
            resultCanvas[4*index+0] *= std::min((scaledCanvas[index].b+lightener)/255.0, 1.0);
            resultCanvas[4*index+1] *= std::min((scaledCanvas[index].g+lightener)/255.0, 1.0);
            resultCanvas[4*index+2] *= std::min((scaledCanvas[index].r+lightener)/255.0, 1.0);
        }
    }

    delete brushes;
    delete[] scaledCanvas;

    return resultCanvas;
}

void PainterlyRender::paintLayer(GLubyte *canvas, GLubyte *blurredImage, int brush)
{
    //create a list of zVals, all initially set to 0
    //zVals are in (0,1), with a higher number being drawn closer to the screen
    float zVals[m_dataWidth*m_dataHeight];
    memset(zVals, 0, m_dataWidth*m_dataHeight*sizeof(float));

    float threshold = 25.0; //20 works well too

    //create an array of differences between the canvas and the blurred image
    int difs[m_dataWidth*m_dataHeight];
    memset(difs, 0, m_dataWidth*m_dataHeight*sizeof(int));
    for(int i=0; i<m_dataHeight; i++) {
        for(int j=0; j<m_dataWidth; j++) {
            int index = i*m_dataWidth + j;
            int bRoot=0;
            int gRoot=0;
            int rRoot=0;
            if(canvas[index*4+3] == 0) {
                bRoot = 1000;
                gRoot = 1000;
                rRoot = 1000;
            }
            else {
                bRoot = canvas[index*4+0] - blurredImage[index*4+0];
                gRoot = canvas[index*4+1] - blurredImage[index*4+1];
                rRoot = canvas[index*4+2] - blurredImage[index*4+2];
            }
            difs[index] = std::sqrt(bRoot*bRoot + gRoot*gRoot + rRoot*rRoot);
        }
    }

    for(int y=0; y<m_dataHeight; y+=brush) {
        for(int x=0; x<m_dataWidth; x+=brush) {
            int totalError = 0;
            int valsUsed = 0;
            int bestVal = 0;
            glm::vec2 best = glm::vec2(0.0,0.0);
            for(int i=std::max(x, 0); i<=std::min(x+brush, m_dataWidth-1); i++) {
                for(int j=std::max(y, 0); j<=std::min(y+brush, m_dataHeight-1); j++) {
                    int index = j*m_dataWidth + i;
                    totalError += difs[index];
                    valsUsed++;
                    if(difs[index] > bestVal) {
                        bestVal = difs[index];
                        best = glm::vec2(i, j);
                    }
                }
            }
            if((float)totalError/(float)valsUsed > threshold) {
                //make a stroke at the pixel with the greatest difference, with a random Z index
                this->makeStroke(canvas, zVals, best, brush);
            }
        }
    }
}

void PainterlyRender::makeStroke(GLubyte* canvas, float zVals[], glm::vec2 best, int brush)
{
    int sourceIndex = best.y*m_dataWidth + best.x;
    float zVal = ((float) std::rand() / RAND_MAX);

    //find the brush data to multiply in
//    glm::vec2 brushTexCenter = glm::vec2(std::rand() % (m_brushWidth - 2*brush) + brush, std::rand() % (m_brushHeight - 2*brush) + brush);
//    int lightener = 50;

    for(int x=std::max(0, (int)best.x-brush); x<=std::min(m_dataWidth-1, (int)best.x+brush); x++) {
        for(int y=std::max(0, (int)best.y-brush); y<=std::min(m_dataHeight-1, (int)best.y+brush); y++) {
            //check to see if pixel is in brush circle range
            if(std::sqrt((best.x-x)*(best.x-x) + (best.y-y)*(best.y-y)) < brush) {
                int index = y*m_dataWidth + x;
                if(zVals[index] < zVal) {
                    zVals[index] = zVal;
//                    if(brush > 4) {
//                        glm::vec2 newTexCoords = brushTexCenter + (best - glm::vec2(x, y));
//                        int texIndex = newTexCoords.y*m_canvasWidth + newTexCoords.x;
//                        canvas[index].b = m_data[sourceIndex].b * min((m_brushTexture[texIndex].b+lightener)/255.0, 1.0);
//                        canvas[index].g = m_data[sourceIndex].g * min((m_brushTexture[texIndex].g+lightener)/255.0, 1.0);
//                        canvas[index].r = m_data[sourceIndex].r * min((m_brushTexture[texIndex].r+lightener)/255.0, 1.0);
//                    }
//                    else {
                    canvas[index*4+0] = m_data[sourceIndex*4+0];
                    canvas[index*4+1] = m_data[sourceIndex*4+1];
                    canvas[index*4+2] = m_data[sourceIndex*4+2];
//                    }
                    canvas[index*4+3] = 255;

                }
            }
        }
    }
}

GLubyte* PainterlyRender::getBlur(GLubyte* imageData, int radius)
{
    //calculate the gaussian filter
    int filterWidth = radius * 2 + 1;
    double* filter = new double[filterWidth];
    double sigma = radius / 3.0;
    double sum = 0;
    for(int i=0; i<filterWidth; i++) {
        int x = i - radius;
        filter[i] = pow(VAR_E, -(x*x)/(2*sigma*sigma)) / (sigma*sqrt(2*VAR_PI));
        sum += filter[i];
    }

    //normalize the filter
    for(int i=0; i<filterWidth; i++) {
        filter[i] = filter[i] / sum;
    }

    GLubyte *passData = new GLubyte[m_dataWidth*m_dataHeight*4];
    memcpy(passData, imageData, m_dataWidth*m_dataHeight*4);

    //apply the filter in both directions
    GLubyte* temp = PainterlyRender::pass1DFilter(passData, m_dataWidth, m_dataHeight, filter, filterWidth, true);
    delete[] passData;
    passData = temp;
    temp = PainterlyRender::pass1DFilter(passData, m_dataWidth, m_dataHeight, filter, filterWidth, false);
    delete[] passData;
    passData = temp;

    //cleanup
    delete[] filter;

    return passData;
}

BGRA* PainterlyRender::scaleImage(BGRA *image, int oldW, int oldH, int newW, int newH)
{
    double a = newW/(double)oldW;
    double radius = a < 1 ? 1.0/a : 1.0;
    BGRA* temp = PainterlyRender::staticScalePass(newW, oldH, true, image, oldW, oldH, a, radius);

    a = newH/(double)oldH;
    radius = a < 1 ? 1.0/a : 1.0;

    BGRA* temp2 = PainterlyRender::staticScalePass(newW, newH, false, temp, newW, oldH, a, radius); //check this
    delete[] temp;
    return temp2;
}

BGRA* PainterlyRender::staticScalePass(int newWidth, int newHeight, bool horizontal, BGRA *image, int oldWidth, int oldHeight, double a, double radius)
{
    BGRA* resultData = new BGRA[newWidth * newHeight];

    int iLength = horizontal ? newHeight : newWidth;
    int jLength = horizontal ? newWidth : newHeight;
    //scale horizontally
    for(int i=0; i<iLength; i++) {
        for(int j=0; j<jLength; j++) {

            double center = j / a + (1-a)/2.0;
            int left = ceil(center - radius);
            int right = floor(center + radius);

            //if any values are cut off, figure out how to normalize the remaining ones
            double externalVals = 0;

            while(left <= 0) {
                externalVals += PainterlyRender::staticTriangleVal(center - left, radius);
                left++;
            }

            int imageSize = horizontal ? oldWidth : oldHeight;
            while(right >= imageSize) {
                externalVals += PainterlyRender::staticTriangleVal(right - center, radius);
                right--;
            }

            double normalizer = 1 - externalVals;

            //calculate the actual value, at the pixel, using the new normalized values
            double resultB=0, resultG=0, resultR=0, weightsB=0, weightsG=0, weightsR=0;

            for(int k=left; k<=right; k++) {
                int index = horizontal ? k + i*oldWidth : i + k*oldWidth;

                double filtNorm = PainterlyRender::staticTriangleVal(k - center, radius) / normalizer;
                resultB += image[index].b * filtNorm;
                resultG += image[index].g * filtNorm;
                resultR += image[index].r * filtNorm;

                if(radius != 1.0) { //normalize values if scaling down
                    weightsB += filtNorm;
                    weightsG += filtNorm;
                    weightsR += filtNorm;
                }

            }

            int index = horizontal ? j + i*newWidth : i + j*newWidth;
            resultData[index].b = radius == 1.0 ? resultB : resultB / weightsB;
            resultData[index].g = radius == 1.0 ? resultG : resultG / weightsG;
            resultData[index].r = radius == 1.0 ? resultR : resultR / weightsR;
        }
    }

    return resultData;
}

double PainterlyRender::staticTriangleVal(double x, double radius)
{
    if(x < -radius || x > radius) {
        return 0;
    }

    return (1 - fabs(x)/radius) / radius;
}

/**
 * @brief EdgeDetect::pass1DFilter
 * @param filter: An array filter to pass over the image
 * @param filterWidth: The width of the filter (must be odd)
 * @param horizontal: Whether or not the filter is being passed horizontally
 * @return
 */
GLubyte* PainterlyRender::pass1DFilter(GLubyte* data, int dataWidth, int dataHeight, double *filter, int filterWidth, bool horizontal)
{
    GLubyte* resultData = new GLubyte[dataWidth * dataHeight * 4];
    int filterExtend = (filterWidth - 1)/2;

    int height = horizontal ? dataHeight : dataWidth;
    int width = horizontal ? dataWidth : dataHeight;

    for(int i=0; i<height; i++) {
        for(int j=0; j<width; j++) {
            //figure out where the filters should start and end (avoid indexing OOB of the data array)
            int start = 0;
            int end = filterWidth - 1;
            if(j < filterExtend) {
                start = filterExtend - j;
            }
            int rightExtend = width - j - 1;
            if(rightExtend < filterExtend) {
                end = filterExtend + rightExtend;
            }

            //if any values are cut off, figure out how to normalize the remaining ones
            double externalVals = 0;
            for(int k=0; k<start; k++) {
                externalVals += filter[k];
            }
            for(int k=end+1; k<filterWidth - 1; k++) {
                externalVals += filter[k];
            }
            double normalizer = 1 - externalVals;

            //calculate the actual value, at the pixel, using the new normalized values
            double resultB = 0;
            double resultG = 0;
            double resultR = 0;

            for(int k=start; k<=end; k++) {
                int index = horizontal ? j-filterExtend+k + i*dataWidth : i + (j-filterExtend+k)*dataWidth;

                double filtNorm = filter[k] / normalizer;

                resultB += data[index*4+0] * filtNorm;
                resultG += data[index*4+1] * filtNorm;
                resultR += data[index*4+2] * filtNorm;
            }

            int index = horizontal ? j + i*dataWidth : i + j*dataWidth;
            resultData[index*4+0] = resultB;
            resultData[index*4+1] = resultG;
            resultData[index*4+2] = resultR;
        }
    }

    return resultData;
}
