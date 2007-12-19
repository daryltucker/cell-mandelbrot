
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "dialog.h"
#include "mandelbrot.h"

inline QLabel* rightAlignedLabel(QString text)
{
  QLabel *label = new QLabel(text);
  label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  return label;
}

Dialog::Dialog(QWidget *parent) : QWidget(parent)
{
  QVBoxLayout *vLayout  = new QVBoxLayout;
  QHBoxLayout *hLayoutA = new QHBoxLayout;
  QHBoxLayout *hLayoutB = new QHBoxLayout;
  redrawButton          = new QPushButton;
  editReOffset          = new QDoubleSpinBox;
  editImOffset          = new QDoubleSpinBox;
  editZoom              = new QDoubleSpinBox;
  editAreaWidth         = new QSpinBox;
  editAreaHeight        = new QSpinBox;
  editAreaX             = new QSpinBox;
  editAreaY             = new QSpinBox;
  m                     = new Mandelbrot;

  editAreaWidth->setRange(0, 10000);
  editAreaHeight->setRange(0, 10000);
  editAreaX->setRange(0, 10000);
  editAreaY->setRange(0, 10000);
  editReOffset->setRange(-10, 10);
  editImOffset->setRange(-10, 10);
  editZoom->setRange(0, 100000);
  editReOffset->setSingleStep(0.01);
  editImOffset->setSingleStep(0.01);
  editZoom->setValue(1.0);
  editReOffset->setValue(0.0);
  editImOffset->setValue(0.0);
  redrawButton->setText("Redraw mandelbrot");

  hLayoutA->addWidget(new QLabel("Offsets:"));
  hLayoutA->addWidget(rightAlignedLabel("real"));
  hLayoutA->addWidget(editReOffset);
  hLayoutA->addWidget(rightAlignedLabel("imaginary"));
  hLayoutA->addWidget(editImOffset);
  hLayoutA->addWidget(rightAlignedLabel("Zoom"));
  hLayoutA->addWidget(editZoom);
  hLayoutB->addWidget(new QLabel("Area:"));
  hLayoutB->addWidget(rightAlignedLabel("x"));
  hLayoutB->addWidget(editAreaX);
  hLayoutB->addWidget(rightAlignedLabel("y"));
  hLayoutB->addWidget(editAreaY);
  hLayoutB->addWidget(rightAlignedLabel("width"));
  hLayoutB->addWidget(editAreaWidth);
  hLayoutB->addWidget(rightAlignedLabel("height"));
  hLayoutB->addWidget(editAreaHeight);
  vLayout->addWidget(m);
  vLayout->addLayout(hLayoutA);
  vLayout->addLayout(hLayoutB);
  vLayout->addWidget(redrawButton);
  setLayout(vLayout);

  setWindowTitle(tr("Mandelbrot"));
  resize(400, 300);

  this->connect(redrawButton, SIGNAL(clicked()), this, SLOT(handleRedraw()));
}


void Dialog::handleRedraw()
{
  m->setZoom(editZoom->value());
  m->setOffset(editReOffset->value(), editImOffset->value());
  m->setArea(editAreaX->value(), editAreaY->value(),
             editAreaWidth->value(), editAreaHeight->value());
  m->drawMandelbrot();
  m->repaint();
  repaint();
}
