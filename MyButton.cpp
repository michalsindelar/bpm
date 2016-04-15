//
// Created by Michal on 15/04/16.
//

#include "MyButton.h"
#include "config.h"

MyButton::MyButton(QWidget *parent)
        : QWidget(parent) {

    QPushButton *modeCamera = new QPushButton("Mode Camera Real", this);
    modeCamera->setGeometry(20, 40, 150, 50);

    QPushButton *modeVideoReal = new QPushButton("Mode Video Real", this);
    modeVideoReal->setGeometry(180, 40, 150, 50);

    QPushButton *modeVideoStatic = new QPushButton("Mode Video Static", this);
    modeVideoStatic->setGeometry(340, 40, 150, 50);

    connect(modeCamera, SIGNAL(clicked()), this, (SLOT(handleClickMan())));
    connect(modeVideoReal, SIGNAL(clicked()), this, SLOT(handleClickMan()));
    connect(modeVideoStatic, SIGNAL(clicked()), this, SLOT(handleClickMan()));
}


void MyButton::handleClickMan() {
    this->mode = CAMERA_SOURCE_MODE;
}

#include "moc_MyButton.cpp"