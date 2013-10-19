#include "speedyshare.h"
#include "config.h"

#if __QT5__
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>
#else
#include <QtGui/QApplication>
#include <QtGui/QStyleFactory>
#endif

#if WIN32
#include <windows.h>
#define WIN32_LEAN_AND_MEAN

// forward-declare main
int __cdecl main(int argc, char** argv);

//------------------------------------------------------------------------------------
/**
	Just handle this Windows bullshit
*/
int WINAPI WinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd )
{
	return main(nShowCmd, (char**)&lpCmdLine);
}
#endif

//------------------------------------------------------------------------------------
/**
*/
int
main(int argc, char** argv)
{
	qRegisterMetaType<FilePackage>("FilePackage");
	QApplication a(argc, argv);
#if __QT5__
	QStyle* style = QStyleFactory::create("fusion");
#else
	QStyle* style = QStyleFactory::create("plastique");
#endif
	a.setStyle(style);
	SpeedyShare w;	
	w.show();
	a.setQuitOnLastWindowClosed(true);
	return a.exec();
}
