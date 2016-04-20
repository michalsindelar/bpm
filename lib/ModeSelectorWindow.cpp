//
// Created by Michal on 15/04/16.
//

// Classes
#include "ModeSelectorWindow.h"

ModeSelectorWindow::ModeSelectorWindow(QWidget *parent)
        : QWidget(parent) {

    // Default camera
    this->mode = CAMERA_SOURCE_MODE;

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

    QPushButton *saveButton = new QPushButton("Save output", this);
    saveButton->setGeometry(20, 150, buttonSize.width, buttonSize.height);

    QPushButton *saveDialogButton = new QPushButton("Choose output folder", this);
    saveDialogButton->setGeometry(180, 150, buttonSize.width, buttonSize.height);
    connect(saveDialogButton, SIGNAL(clicked()), this, SLOT(handleOutputFileDialog()));

    QPushButton *runButton = new QPushButton("Run app!", this);
    runButton->setGeometry(180, 250, buttonSize.width, buttonSize.height);

    connect(modeCameraButton, SIGNAL(clicked()), this, (SLOT(handleCameraSourceMode())));
    connect(modeVideoRealButton, SIGNAL(clicked()), this, SLOT(handleVideoRealMode()));
    connect(modeVideoStaticButton, SIGNAL(clicked()), this, SLOT(handleVideoStaticMode()));

    connect(fileDialogButton, SIGNAL(clicked()), this, SLOT(handleInputFileDialog()));
    connect(fileDialogButtonCopy, SIGNAL(clicked()), this, SLOT(handleInputFileDialog()));

    connect(saveButton, SIGNAL(clicked()), this, SLOT(handleSaveButton()));

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


void ModeSelectorWindow::handleInputFileDialog() {
    this->inputFilePath = new QString(QFileDialog::getOpenFileName(
            this,
            tr("Open Video"),
            QString::fromStdString(PROJECT_DIR),
            tr("Video Files (*.avi *.mpg *.mp4 *.mov)")));
}


void ModeSelectorWindow::handleOutputFileDialog() {
    this->outputFolderPath = new QString(QFileDialog::getExistingDirectory (
            this,
            tr("Open Directory"),
            QString::fromStdString(PROJECT_DIR),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
}

void ModeSelectorWindow::handleSaveButton() {
    this->saveOutput = !this->saveOutput;
}

#include "moc_ModeSelectorWindow.cpp"