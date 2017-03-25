#ifndef __holderGUI
#define __holderGUI

#include "ui_holderGUI.h"
#include "nPhysD.h"

class nView;

class holderGUI: public QMainWindow, private Ui::holderGUI {
	Q_OBJECT

public:
	Q_INVOKABLE holderGUI();

public slots:

	std::vector<nPhysD*> fileOpen(QString fname);
	void on_actionOpen_triggered();

	void on_actionViewer_triggered();
	std::vector<nPhysD*> openFiles(QStringList fnames);
	void delPhys(QObject* my_phys);
	void addViewPhys(nPhysD* my_phys);
	void delViewPhys(nPhysD* my_phys);

	void on_physList_itemPressed(QListWidgetItem* item);
	void on_physList_itemSelectionChanged();
	void on_physList_itemDoubleClicked(QListWidgetItem* item);

	void keyPressEvent (QKeyEvent *);
	void logging(QString msg);

	void dragEnterEvent(QDragEnterEvent *);
	void dragMoveEvent(QDragMoveEvent *);
	void dropEvent(QDropEvent *);

	void registerPhys(nPhysD*);
	void registerViewer(nView*);
	void unregisterViewer(QObject*);

	std::vector<nPhysD*> allPhys();

	void loadDefaults();
	void saveDefaults();

	void closeEvent(QCloseEvent *e);

signals:
	void nPhysDSelected(nPhysD*);
	void nPhysDDoubleClick(nPhysD*);
	void nPhysDClick(nPhysD*);
	void addPhys(nPhysD*);

private:
	QPointer<QMainWindow> physViewer;
	int physUID;
	int viewerUID;
};


#endif
