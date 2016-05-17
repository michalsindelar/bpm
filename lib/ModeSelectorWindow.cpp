//
// Created by Michal on 15/04/16.
//

// Classes
#include "ModeSelectorWindow.h"

ModeSelectorWindow::ModeSelectorWindow(QWidget *parent)
        : QWidget(parent) {

    // Don't save output by default
    this->saveOutput = false;

    // Default camera
    this->mode = CAMERA_SOURCE_MODE;

    this->inputFilePath = QString();
    this->outputFolderPath = QString();

    QSize buttonSize = QSize(150, 50);

    modeCameraButton = new QPushButton("Mode Camera Real", this);
    modeCameraButton->setGeometry(QRect(QPoint(20, 40), buttonSize));

    modeVideoRealButton = new QPushButton("Mode Video Real", this);
    modeVideoRealButton->setGeometry(QRect(QPoint(180, 40), buttonSize));

    modeVideoStaticButton = new QPushButton("Mode Video Static", this);
    modeVideoStaticButton->setGeometry(QRect(QPoint(340, 40), buttonSize));

    fileDialogButton = new QPushButton("Choose video file", this);
    fileDialogButton->setGeometry(180, 95, buttonSize.width(), buttonSize.height() - 15);

    fileDialogButtonCopy = new QPushButton("Choose video file", this);
    fileDialogButtonCopy->setGeometry(340, 95, buttonSize.width(), buttonSize.height() - 15);

    saveButton = new QPushButton("Save output", this);
    saveButton->setGeometry(QRect(QPoint(20, 150), buttonSize));

    saveDialogButton = new QPushButton("Choose output folder", this);
    saveDialogButton->setGeometry(QRect(QPoint(180, 150), buttonSize));
    connect(saveDialogButton, SIGNAL(clicked()), this, SLOT(handleOutputFileDialog()));

    runButton = new QPushButton("Run app!", this);
    runButton->setGeometry(180, 250, buttonSize.width(), buttonSize.height() - 15);

    connect(modeCameraButton, SIGNAL(clicked()), this, (SLOT(handleCameraSourceMode())));
    connect(modeVideoRealButton, SIGNAL(clicked()), this, SLOT(handleVideoRealMode()));
    connect(modeVideoStaticButton, SIGNAL(clicked()), this, SLOT(handleVideoStaticMode()));

    connect(fileDialogButton, SIGNAL(clicked()), this, SLOT(handleInputFileDialog()));
    connect(fileDialogButtonCopy, SIGNAL(clicked()), this, SLOT(handleInputFileDialog()));

    connect(saveButton, SIGNAL(clicked()), this, SLOT(handleSaveButton()));

    // Run button will exit mode selector window and init execution of main app
    connect(runButton, SIGNAL(clicked()), qApp, SLOT(quit()));

    // Reset border stylesA
    resetStyle();
}
void ModeSelectorWindow::resetStyle() {
    // Reset at first
    modeCameraButton->setStyleSheet("border: 2px solid #aaaaaa;");
    modeVideoRealButton->setStyleSheet("border: 2px solid #aaaaaa;");
    modeVideoStaticButton->setStyleSheet("border: 2px solid #aaaaaa;");
    saveButton->setStyleSheet("border: 2px solid #aaaaaa;");


    // Active state - mode
    switch (this->mode) {
        case CAMERA_SOURCE_MODE:
            modeCameraButton->setStyleSheet("border: 2px solid #008800;");
            break;
        case VIDEO_REAL_SOURCE_MODE:
            modeVideoRealButton->setStyleSheet("border: 2px solid #008800;");
            break;
        case VIDEO_STATIC_SOURCE_MODE:
            modeVideoStaticButton->setStyleSheet("border: 2px solid #008800;");
            break;
        default:
            break;
    }


    if (this->mode == VIDEO_REAL_SOURCE_MODE || this->mode == VIDEO_STATIC_SOURCE_MODE) {
        if (this->inputFilePath.isEmpty()) {
            this->fileDialogButton->setStyleSheet("border: 2px solid #880000;");
            this->fileDialogButtonCopy->setStyleSheet("border: 2px solid #880000;");
        } else {
            this->fileDialogButton->setStyleSheet("border: 2px solid #aaaaaa;");
            this->fileDialogButtonCopy->setStyleSheet("border: 2px solid #aaaaaa;");
        }
    }

    if (this->saveOutput) {
        saveButton->setStyleSheet("border: 2px solid #008800;");
        if (this->outputFolderPath.isEmpty()) {
            this->saveDialogButton->setStyleSheet("border: 2px solid #880000;");
        } else {
            this->saveDialogButton->setStyleSheet("border: 2px solid #aaaaaa;");
        }
    }
}

void ModeSelectorWindow::handleCameraSourceMode() {
    this->mode = CAMERA_SOURCE_MODE;
    resetStyle();
}

void ModeSelectorWindow::handleVideoRealMode() {
    this->mode = VIDEO_REAL_SOURCE_MODE;
    resetStyle();
}

void ModeSelectorWindow::handleVideoStaticMode() {
    this->mode = VIDEO_STATIC_SOURCE_MODE;
    resetStyle();
}

void ModeSelectorWindow::handleInputFileDialog() {
    this->inputFilePath = (QFileDialog::getOpenFileName(
            this,
            tr("Open Video"),
            QString::fromStdString(PROJECT_DIR),
            tr("Video Files (*.avi *.mpg *.mp4 *.mov)")));
    resetStyle();
}

void ModeSelectorWindow::handleOutputFileDialog() {
    this->outputFolderPath = (QFileDialog::getExistingDirectory (
            this,
            tr("Open Directory"),
            QString::fromStdString(PROJECT_DIR),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
    resetStyle();
}

void ModeSelectorWindow::handleSaveButton() {
    this->saveOutput = !this->saveOutput;
    resetStyle();
}

#include "moc_ModeSelectorWindow.cpp"