//
// Created by Michal on 15/04/16.
//

#ifndef BPM_MYBUTTON_H
#define BPM_MYBUTTON_H

#include <QApplication>
#include <QWidget>
#include <QPushButton>

class MyButton : public QWidget {
    Q_OBJECT

    private:
        int mode;

    public:
        MyButton(QWidget *parent = 0);
    public slots:
        void handleClickMan();

        int getMode() const {
            return mode;
        }
};


#endif //BPM_MYBUTTON_H
