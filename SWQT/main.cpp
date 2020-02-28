#include "SWQT.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	SWQT w;
	w.show();
	return a.exec();
}
