#ifndef __HolderGUI
#define __HolderGUI

#include "ui_holderGUI.h"
#include "nPhysD.h"

class holderGUI: public QMainWindow, private std::vector<nPhysD*>, private Ui::holderGUI {
	Q_OBJECT

public:
	Q_INVOKABLE holderGUI();

public slots:

	std::vector<nPhysD*> fileOpen(std::string fname);
	void on_actionOpen_triggered();
	void on_actionViewer_triggered();
	void openFiles(QStringList fnames);
	void delPhys(QObject* my_phys);
	void addViewPhys(nPhysD* my_phys);

	void on_treeWidget_itemPressed(QTreeWidgetItem* item, int i);
	void on_treeWidget_itemSelectionChanged();
	void on_treeWidget_itemDoubleClicked(QTreeWidgetItem* item, int i);

	void keyPressEvent (QKeyEvent *);
	void logging(QString msg);

	void dragEnterEvent(QDragEnterEvent *);
	void dragMoveEvent(QDragMoveEvent *);
	void dropEvent(QDropEvent *);

signals:
	void nPhysDSelected(nPhysD*);
	void nPhysDDoubleClick(nPhysD*);
	void nPhysDClick(nPhysD*);
	void addPhys(nPhysD*);

private:
	QPointer<QMainWindow> physViewer;
};


#endif
