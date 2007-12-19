
#include <QWidget>
#include <QImage>

class Mandelbrot : public QWidget
{
  Q_OBJECT

  public:
    Mandelbrot(QWidget *parent = 0);
    ~Mandelbrot();
    void setZoom(double zoom);
    void setOffset(double reOffset, double imOffset);
    void setArea(uint x, uint y, uint width, uint height);

  public slots:
    void drawMandelbrot();

  protected:
    virtual void paintEvent(QPaintEvent *event);

  private:
    QImage *image;
    double reOffset;
    double imOffset;
    double zoom;
    uint   areaX;
    uint   areaY;
    uint   areaWidth;
    uint   areaHeight;
};
