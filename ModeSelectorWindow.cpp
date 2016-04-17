//
// Created by Michal on 15/04/16.
//

// Classes
#include "ModeSelectorWindow.h"

ModeSelectorWindow::ModeSelectorWindow(QWidget *parent)
        : QWidget(parent) {

    Size buttonSize = Size(150, 50);

    QPushButton *modeCameraButton = new QPushButton("Mode Camera Real", this);
    modeCameraButton->setGeometry(20, 40, buttonSize.width, buttonSize.height);

    QPushButton *modeVideoRealButton = new QPushButton("Mode Video Real", this);
    modeVideoRealButton->setGeometry(180, 40, buttonSize.width, buttonSize.height);

    QPushButton *modeVideoStaticButton = new QPushButton("Mode Video Static", this);
    modeVideoStaticButton->setGeometry(340, 40, buttonSize.width, buttonSize.height);

    QPushButton *fileDialogButton = new QPushButton("Choose video file", this);
    fileDialogButton->setGeometry(180, 95, buttonSize.width, buttonSize.height - 15);

    QPushButton *fileDialogButtonCopy = new QPushButton("Choose video file", this);
    fileDialogButtonCopy->setGeometry(340, 95, buttonSize.width, buttonSize.height - 15);


    QPushButton *runButton = new QPushButton("Run app!", this);
    runButton->setGeometry(180, 150, buttonSize.width, buttonSize.height);

    connect(modeCameraButton, SIGNAL(clicked()), this, (SLOT(handleCameraSourceMode())));
    connect(modeVideoRealButton, SIGNAL(clicked()), this, SLOT(handleVideoRealMode()));
    connect(modeVideoStaticButton, SIGNAL(clicked()), this, SLOT(handleVideoStaticMode()));

    connect(fileDialogButton, SIGNAL(clicked()), this, SLOT(handleFileDialog()));
    connect(fileDialogButtonCopy, SIGNAL(clicked()), this, SLOT(handleFileDialog()));


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


void ModeSelectorWindow::handleFileDialog() {
    this->fileName = new QString(QFileDialog::getOpenFileName(this,
                                                              tr("Open Video"), QString::fromStdString(PROJECT_DIR), tr("Video Files (*.avi *.mpg *.mp4 *.mov)")));
}

#include "moc_ModeSelectorWindow.cpp"