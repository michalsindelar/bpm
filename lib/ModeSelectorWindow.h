//
// Created by Michal on 15/04/16.
//

#ifndef BPM_MODESELECTORWINDOW_H
#define BPM_MODESELECTORWINDOW_H

#include <QApplication>
#include <QFileDialog>
#include <QWidget>
#include <QPushButton>
#include <QString>

// Constants
#include "constants.h"

#include "./lib/imageOperation.h"
#include <iostream>

class ModeSelectorWindow : public QWidget {
    Q_OBJECT

private:
    int mode;
    bool saveOutput;
    QString inputFilePath;

    // Modes
    QPushButton *modeCameraButton, *modeVideoRealButton, *modeVideoStaticButton;

    // Run
    QPushButton *runButton;

    // Files dialog
    QPushButton *fileDialogButton, *fileDialogButtonCopy;

    // Save
    QPushButton *saveButton, *saveDialogButton;

    QString outputFolderPath;

public:
    explicit ModeSelectorWindow(QWidget *parent = 0);


    int getMode() const {
        return mode;
    }

    QString getFileName() const {
        return inputFilePath;
    }

    QString getOutputFolder() const {
        return outputFolderPath;
    }

    bool getSaveOutput() const {
        return saveOutput;
    }

    void resetStyle();

public slots:
    void handleCameraSourceMode();
    void handleVideoRealMode();
    void handleVideoStaticMode();
    void handleInputFileDialog();

    void handleOutputFileDialog();
    void handleSaveButton();
};


#endif //BPM_MODESELECTORWINDOW_H
