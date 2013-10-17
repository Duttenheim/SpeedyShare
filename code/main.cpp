#include "speedyshare.h"
#if __QT5__
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>
#else
#include <QtGui/QApplication>
#include <QtGui/QStyleFactory>
#endif



//------------------------------------------------------------------------------------
/**
*/
int main(int argc, char *argv[])
{
	qRegisterMetaType<FilePackage>("FilePackage");
	QApplication a(argc, argv);
	QStyle* style = QStyleFactory::create("fusion");
	a.setStyle(style);
	SpeedyShare w;	
	w.show();
	a.setQuitOnLastWindowClosed(true);
	return a.exec();
}
