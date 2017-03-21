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
		graphicsView->close();
	};

public slots:

	void on_actionOpen_triggered();
	void openFiles(QStringList fnames);
	void addPhys(nPhysD* my_phys,QTreeWidgetItem *item);
	void delPhys(QObject* my_phys);

	void on_treeWidget_itemPressed(QTreeWidgetItem* item, int i);
	void on_treeWidget_itemSelectionChanged();

};


#endif
