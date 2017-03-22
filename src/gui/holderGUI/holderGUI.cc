#include "holderGUI.h"
#include <QImageReader>
#include <QFileDialog>
#include <QTreeWidgetItem>
#include "graphics/nView.h"

holderGUI::holderGUI() : QMainWindow() {
	setAttribute(Qt::WA_DeleteOnClose);
	setupUi(this);
	show();
}

void holderGUI::keyPressEvent (QKeyEvent *event) {
	QList<nPhysD*> selectedPhys;
	for (auto& item: treeWidget->selectedItems()) {
		nPhysD *my_phys=item->data(1,0).value<nPhysD*>();
		if(my_phys) {
			selectedPhys.append(my_phys);
		}
	}

	switch (event->key()) {
		case Qt::Key_Backspace:
			for (auto& phys: selectedPhys)
				phys->deleteLater();
			break;
		default:
			break;
	}
}

void holderGUI::on_treeWidget_itemDoubleClicked(QTreeWidgetItem* item, int i) {
	on_actionViewer_triggered();
	nPhysD *my_phys=item->data(1,0).value<nPhysD*>();
	if (my_phys) {
		emit nPhysDSelected(my_phys);
	}

}

void holderGUI::on_treeWidget_itemPressed(QTreeWidgetItem* item, int i) {
	qDebug() << i << ":" << item->data(1,0).typeName();
}

void holderGUI::on_treeWidget_itemSelectionChanged() {
	qDebug() << "-->>> here";
	QList<QTreeWidgetItem *> items= treeWidget->selectedItems();
	if (items.size()==1) {
		nPhysD *my_phys=items.first()->data(1,0).value<nPhysD*>();
		if(my_phys) {
			emit nPhysDSelected(my_phys);
		}
	}
};

void holderGUI::on_actionOpen_triggered() {
	QString formats("");
	formats+="Neutrino Images (*.txt *.neu *.neus *.tif *.tiff *.hdf *.sif *.b16 *.spe *.pcoraw *.img *.raw *.fits *.inf *.gz);;";
	formats+="Images (";
	foreach (QByteArray format, QImageReader::supportedImageFormats() ) {
		formats+="*."+format+" ";
	}
	formats.chop(1);
	formats+=");;";
	formats+=("Any files (*)");

	QStringList fnames = QFileDialog::getOpenFileNames(this,tr("Open Image(s)"),property("NeuSave-fileOpen").toString(),formats);
	openFiles(fnames);
}

void holderGUI::on_actionViewer_triggered() {
	if (physViewer.isNull()) {
		physViewer = new QMainWindow(this);
		nView *my_view = new nView(physViewer);
		physViewer->setAttribute(Qt::WA_DeleteOnClose);

		physViewer->setCentralWidget(my_view);
		connect(this, SIGNAL(nPhysDSelected(nPhysD*)), my_view, SLOT(showPhys(nPhysD*)));
		connect(this, SIGNAL(addPhys(nPhysD*)), my_view, SLOT(showPhys(nPhysD*)));
	}
	physViewer->show();
}

void holderGUI::openFiles(QStringList fnames) {
	setProperty("NeuSave-fileOpen", fnames);
	foreach (QString fname, fnames) {
		QTreeWidgetItem *item=new QTreeWidgetItem(treeWidget,QStringList(fname));
		item->setIcon(0,QIcon(":icons/filetype.png"));
		item->setExpanded(true);
		std::vector<nPhysD*> retlist = nHolder::getInstance().fileOpen(fname.toStdString());
		for (auto& img: retlist) {
			QTreeWidgetItem *my_item=new QTreeWidgetItem(item,QStringList(QString::fromStdString(img->getName())));
			QVariant my_var=QVariant::fromValue(qobject_cast<nPhysD*>(img));
			qDebug() << img << my_var;
			my_item->setData(1,0,my_var);
			my_item->setIcon(0,QIcon(":icons/icon.png"));
			connect(img, SIGNAL(destroyed(QObject*)), this, SLOT(delPhys(QObject*)));
			emit addPhys(img);
		}
	}
}

void holderGUI::delPhys(QObject* my_obj) {
	if (my_obj) {
		QTreeWidgetItemIterator it(treeWidget);
		while (*it) {
			if ((*it)->data(1,0) == QVariant::fromValue(static_cast<nPhysD*>(my_obj))) {
				QTreeWidgetItem *my_parent=(*it)->parent();
				delete (*it);
				if (my_parent->childCount()==0) {
					delete my_parent;
				}
			}
			++it;
		}
	}
}
