
#include <QPainter>
#include "mandelbrot.h"
#include <QMessageBox>

extern "C" void draw24bitMandelbrot(char *image, uint width, uint height,
                                    double reOffset, double imOffset,
                                    double zoom, uint maxIteration,
                                    uint areaX, uint areaY,
                                    uint areaWidth, uint areaHeight,
                                    uint bytesPerPixel);

Mandelbrot::Mandelbrot(QWidget *parent) : QWidget(parent)
{
  setBackgroundRole(QPalette::Midlight);
  setAutoFillBackground(true);
  image      = NULL;
  reOffset   = 0;
  imOffset   = 0;
  areaX      = 0;
  areaY      = 0;
  areaWidth  = 0;
  areaHeight = 0;
  zoom       = 1;
  this->setMinimumHeight(100);
  this->setMinimumWidth(100);
}


Mandelbrot::~Mandelbrot()
{
  if (image)
    delete image;
}


void Mandelbrot::setZoom(double zoom)
{
  this->zoom = zoom;
}


void Mandelbrot::setOffset(double reOffset, double imOffset)
{
  this->reOffset = reOffset;
  this->imOffset = imOffset;
}


void Mandelbrot::setArea(uint x, uint y, uint width, uint height)
{
  this->areaX      = x;
  this->areaY      = y;
  this->areaWidth  = width;
  this->areaHeight = height;
}


void Mandelbrot::paintEvent(QPaintEvent *event)
{
  if (!image)
    return;
  QPainter painter(this);
  painter.drawImage(0, 0, *image);
}


void Mandelbrot::drawMandelbrot()
{
  QImage *img = new QImage(width(), height(), QImage::Format_RGB32);

  draw24bitMandelbrot((char*)img->bits(),
                      img->width(), img->height(),
                      reOffset, imOffset, zoom, 1000,
                      areaX, areaY,
                      areaWidth ? areaWidth : img->width(),
                      areaHeight ? areaHeight : img->height(),
                      img->numBytes() / (img->width() * img->height()));

  if (image)
    delete image;
  image = img;
}
