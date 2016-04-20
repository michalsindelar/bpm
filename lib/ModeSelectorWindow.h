//
// Created by Michal on 15/04/16.
//

#ifndef BPM_MYBUTTON_H
#define BPM_MYBUTTON_H

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
    QString *inputFilePath;
    QString *outputFolderPath;

public:
    ModeSelectorWindow(QWidget *parent = 0);

    int getMode() const {
        return mode;
    }

    QString *getFileName() const {
        return inputFilePath;
    }

    QString *getOutputFolder() const {
        return outputFolderPath;
    }

    bool getSaveOutput() const {
        return saveOutput;
    }

public slots:
    void handleCameraSourceMode();
    void handleVideoRealMode();
    void handleVideoStaticMode();
    void handleInputFileDialog();

    void handleOutputFileDialog();
    void handleSaveButton();
};


#endif //BPM_MYBUTTON_H
