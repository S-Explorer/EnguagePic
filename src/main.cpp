#include <QApplication>
#include <QIcon>
#include <QPixmap>

#include "MainScene.h"

int main(int argc, char* argv[]){
    QApplication app(argc, argv);
    
    // 设置应用程序图标（跨平台）
    // Windows: 图标会自动从 .rc 文件加载
    // Linux: 从 Qt 资源文件加载
    #ifdef Q_OS_LINUX
        app.setWindowIcon(QIcon(":/enguage.ico"));
    #endif
    
    // 设置应用程序信息
    app.setApplicationName("EnguagePic");
    app.setOrganizationName("Enguage");

    MainScene m;
    m.show();
    
    return app.exec();
}
