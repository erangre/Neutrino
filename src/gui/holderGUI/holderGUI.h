#ifndef __HolderGUI
#define __HolderGUI

#include "nHolder.h"
#include "ui_holderGUI.h"

class holderGUI: public QMainWindow, private Ui::holderGUI {
	Q_OBJECT

public:
	Q_INVOKABLE holderGUI();
	~holderGUI(){
		qDebug() << "destructor";
	};

public slots:

	void on_actionOpen_triggered();
	void on_actionViewer_triggered();
	void openFiles(QStringList fnames);
	void delPhys(QObject* my_phys);

	void on_treeWidget_itemPressed(QTreeWidgetItem* item, int i);
	void on_treeWidget_itemSelectionChanged();
	void on_treeWidget_itemDoubleClicked(QTreeWidgetItem* item, int i);

	void keyPressEvent (QKeyEvent *);

signals:
	void nPhysDSelected(nPhysD*);
	void nPhysDDoubleClick(nPhysD*);
	void addPhys(nPhysD*);

private:
	QPointer<QMainWindow> physViewer;
};


#endif
