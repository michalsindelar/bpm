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
#include "./config.h"

#include "./lib/imageOperation.h"
#include <iostream>

class ModeSelectorWindow : public QWidget {
    Q_OBJECT

private:
    int mode;
    QString * fileName;

public:
    ModeSelectorWindow(QWidget *parent = 0);

    int getMode() const {
        return mode;
    }

    QString *getFileName() const {
        return fileName;
    }

public slots:
    void handleCameraSourceMode();
    void handleVideoRealMode();
    void handleVideoStaticMode();
    void handleFileDialog();
};


#endif //BPM_MYBUTTON_H
