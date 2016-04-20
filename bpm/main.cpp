#include "../Bpm.h"#include "ModeSelectorWindow.h"#include <QApplication>#include <QWidget>#include <QPushButton>int main (int argc, char * argv[]) {    // Selector gui    QApplication app(argc, argv);    ModeSelectorWindow modes;    modes.resize(500, 300);    modes.setWindowTitle("MODES MENU");    modes.show();    // TODO: Continue only if app exec returns continues    app.exec();    // Main application gui    Bpm bpm;    // Set video file name if video mode    if (modes.getMode() != CAMERA_SOURCE_MODE) {        bpm.setVideoFileName(modes.getFileName()->toStdString());    }    // Save output    if (modes.getSaveOutput()) {        // Generate name of output        time_t now = time(0);        tm *ltm = localtime(&now);        string outputName =            to_string(1900 + ltm->tm_year)+"-"+            to_string(1 + ltm->tm_mon)+"-"+            to_string(ltm->tm_mday)+"-"+            to_string(ltm->tm_hour)+"-"+            to_string(ltm->tm_min);        bpm.setOutputFilePath(modes.getOutputFolder()->toStdString() + "/"+outputName+"-out.avi");    } else {    }    bpm.init(modes.getMode(), FOURIER_MASK_MODE);    // HERE WE GO!!    bpm.run();    destroyAllWindows();    QApplication::quit();    return 0;}