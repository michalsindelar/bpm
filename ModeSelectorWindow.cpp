//
// Created by Michal on 15/04/16.
//

// Classes
#include "ModeSelectorWindow.h"
#include "./lib/imageOperation.h"

#include "./config.h"

ModeSelectorWindow::ModeSelectorWindow(QWidget *parent)
        : QWidget(parent) {

    Size buttonSize = Size(10, 12);

    QPushButton *modeCameraButton = new QPushButton("Mode Camera Real", this);
    modeCameraButton->setGeometry(20, 40, 150, 50);

    QPushButton *modeVideoRealButton = new QPushButton("Mode Video Real", this);
    modeVideoRealButton->setGeometry(180, 40, 150, 50);

    QPushButton *modeVideoStaticButton = new QPushButton("Mode Video Static", this);
    modeVideoStaticButton->setGeometry(340, 40, 150, 50);

    QPushButton *runButton = new QPushButton("Run app!", this);
    runButton->setGeometry(180, 150, 150, 50);

    connect(modeCameraButton, SIGNAL(clicked()), this, (SLOT(handleCameraSourceMode())));
    connect(modeVideoRealButton, SIGNAL(clicked()), this, SLOT(handleVideoRealMode()));
    connect(modeVideoStaticButton, SIGNAL(clicked()), this, SLOT(handleVideoStaticMode()));

    // Run button will exit mode selector window and init execution of main app
    connect(runButton, SIGNAL(clicked()), qApp, SLOT(quit()));
}


void ModeSelectorWindow::handleCameraSourceMode() {
    this->mode = CAMERA_SOURCE_MODE;
}

void ModeSelectorWindow::handleVideoRealMode() {
    this->mode = VIDEO_REAL_SOURCE_MODE;
}

void ModeSelectorWindow::handleVideoStaticMode() {
    this->mode = VIDEO_STATIC_SOURCE_MODE;
}

#include "moc_ModeSelectorWindow.cpp"