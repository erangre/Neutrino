#ifndef __holderGUI
#define __holderGUI

#include "ui_holderGUI.h"
#include "nPhysD.h"
#include "physList.h"
#include "viewerTree.h"
#include "panTree.h"

class nView;

class holderGUI: public QMainWindow, private Ui::holderGUI {
	Q_OBJECT

public:
	Q_INVOKABLE holderGUI();

	QPointer<physList> listPhys;
	QPointer<viewerTree> treeViewers;
	QPointer<panTree> treePans;

public slots:

	std::vector<nPhysD*> fileOpen(QString fname);
	void on_actionOpen_triggered();

	void on_actionViewer_triggered();
	std::vector<nPhysD*> openFiles(QStringList fnames);

	void logging(QString msg);

	void dragEnterEvent(QDragEnterEvent *);
	void dragMoveEvent(QDragMoveEvent *);
	void dropEvent(QDropEvent *);

	std::vector<nPhysD*> allPhys();

	void loadDefaults();
	void saveDefaults();

	void closeEvent(QCloseEvent *e);

};


#endif
