
#include <QWidget>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>

class Mandelbrot;

class Dialog : public QWidget
{
  Q_OBJECT

  public:
    Dialog(QWidget *parent = 0);

  private slots:
    void handleRedraw();

  private:
    Mandelbrot     *m;
    QPushButton    *redrawButton;
    QDoubleSpinBox *editReOffset;
    QDoubleSpinBox *editImOffset;
    QDoubleSpinBox *editZoom;
    QSpinBox       *editAreaWidth;
    QSpinBox       *editAreaHeight;
    QSpinBox       *editAreaX;
    QSpinBox       *editAreaY;
};
