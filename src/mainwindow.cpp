#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowFlags(windowFlags() ^ Qt::WindowMaximizeButtonHint);
    this->setMaximumSize(1024, 720);
}

MainWindow::~MainWindow()
{
    delete ui;
}


