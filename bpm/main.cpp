
#include "../Bpm.h"
#include "../MyButton.h"

#include <QApplication>
#include <QWidget>
#include <QPushButton>

int main (int argc, char * argv[]) {

    QApplication app(argc, argv);

    MyButton modes;

    modes.resize(500, 150);
    modes.setWindowTitle("MODES MENU");
    modes.show();

    app.exec();


    Bpm bpm = Bpm(VIDEO_REAL_SOURCE_MODE, FOURIER_MASK_MODE, 1.5f);

    // HERE WE GO!!
    bpm.run();
}
