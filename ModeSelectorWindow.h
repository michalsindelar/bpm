//
// Created by Michal on 15/04/16.
//

#ifndef BPM_MYBUTTON_H
#define BPM_MYBUTTON_H

#include <QApplication>
#include <QWidget>
#include <QPushButton>

class ModeSelectorWindow : public QWidget {
    Q_OBJECT

    private:
        int mode;

    public:
        ModeSelectorWindow(QWidget *parent = 0);
        int getMode() const {
            return mode;
        }

    public slots:
        void handleCameraSourceMode();
        void handleVideoRealMode();
        void handleVideoStaticMode();
};


#endif //BPM_MYBUTTON_H
