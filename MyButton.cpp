//
// Created by Michal on 15/04/16.
//

#include "MyButton.h"

MyButton::MyButton(QWidget *parent)
        : QWidget(parent) {

    QPushButton *modeCamera = new QPushButton("Mode Camera Real", this);
    modeCamera->setGeometry(20, 40, 150, 50);

    QPushButton *modeVideoReal = new QPushButton("Mode Video Real", this);
    modeVideoReal->setGeometry(180, 40, 150, 50);

    QPushButton *modeVideoStatic = new QPushButton("Mode Video Static", this);
    modeVideoStatic->setGeometry(340, 40, 150, 50);

    connect(modeCamera, SIGNAL(clicked()), this, (SLOT(handleClickMan(CAMERA_SOURCE_MODE))));
    connect(modeVideoReal, SIGNAL(clicked()), this, SLOT(handleClickMan(VIDEO_REAL_SOURCE_MODE)));
    connect(modeVideoStatic, SIGNAL(clicked()), this, SLOT(handleClickMan(VIDEO_STATIC_SOURCE_MODE)));
}


void MyButton::handleClickMan(int mode) {
    this->mode = mode;
}

#include "moc_MyButton.cpp"