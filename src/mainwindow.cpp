#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dataBind()
{
#define BIND(b) { DataBinding *_b = (b); m_bindings.push_back(_b); assert(connect(_b, SIGNAL(dataChanged()), this, SLOT(settingsChanged()))); }

//    BIND(ChoiceBinding::bindRadioButtons(3, 1,
//                                    ui->fillModePoints,
//                                    ui->fillModeWireframe,
//                                    ui->fillModeShaded));
//    BIND(ChoiceBinding::bindRadioButtons(3, 1,
//                                    ui->shadingModeFlat,
//                                    ui->shadingModeSmooth));
//    BIND(BoolBinding::bindCheckbox(ui->lightingEnabled, 1));

#undef BIND
}
