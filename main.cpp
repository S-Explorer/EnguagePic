#include <QApplication>

#include "MainScene.h"

int main(int argc, char* argv[]){
    QApplication app(argc, argv);

    MainScene m;
    m.show();
    
    return app.exec();
}
