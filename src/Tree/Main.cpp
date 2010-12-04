#include "TreePch.h"
#include "TreeMainWindow.h"
#include "Main.h"

void SaveAppLayout(const QMainWindow& mainWin)
{
	QFile dockLayoutOut("Tree.docklayout");
	if (dockLayoutOut.open(QIODevice::WriteOnly))
	{
		dockLayoutOut.write(mainWin.saveState());
		dockLayoutOut.close();
	}
	QFile winLayoutOut("Tree.winlayout");
	if (winLayoutOut.open(QIODevice::WriteOnly))
	{
		winLayoutOut.write(mainWin.saveGeometry());
		winLayoutOut.close();
	}
}

void RestoreAppLayout(QMainWindow& mainWin)
{
	QFile dockLayoutIn("Tree.docklayout");
	if (dockLayoutIn.open(QIODevice::ReadOnly))
	{
		mainWin.restoreState(dockLayoutIn.readAll());
		dockLayoutIn.close();
	}
	QFile winLayoutIn("Tree.winlayout");
	if (winLayoutIn.open(QIODevice::ReadOnly))
	{
		mainWin.restoreGeometry(winLayoutIn.readAll());
		winLayoutIn.close();
	}
}

int main(int argc, char **argv)
{
    int err;
    err = ArnInitializeXmlParser();
    Q_ASSERT(err == 0);
    err = ArnInitializeImageLibrary();
    Q_ASSERT(err == 0);

    QApplication app(argc, argv);
    app.setOrganizationName("Geoyeob Kim");
    app.setApplicationName("ARAN Scene Graph Reader");
    TreeMainWindow mainWin;
    RestoreAppLayout(mainWin);
#ifndef WIN32
    QPoint windowFrameOffset(3, 24);
#endif

    // Calling QMainWindow::show() is crucial for Qt app.
    mainWin.show();

#ifndef WIN32
    mainWin.move(mainWin.pos() - windowFrameOffset);
#endif
    int retCode = app.exec();
    SaveAppLayout(mainWin);

    ArnCleanupXmlParser();
    ArnCleanupImageLibrary();
    ArnCleanupPhysics();
    ArnCleanupGl();

    return retCode;
}
