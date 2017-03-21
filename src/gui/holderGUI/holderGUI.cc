#include "holderGUI.h"
#include <QImageReader>
#include <QFileDialog>
#include <QTreeWidgetItem>

holderGUI::holderGUI() : QMainWindow() {
	setAttribute(Qt::WA_DeleteOnClose);
	DEBUG("HERE");
	setupUi(this);
	show();
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
			graphicsView->showPhys(my_phys);
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

void holderGUI::openFiles(QStringList fnames) {
	setProperty("NeuSave-fileOpen", fnames);
	foreach (QString fname, fnames) {
		QTreeWidgetItem *item=new QTreeWidgetItem(treeWidget,QStringList(fname));
		item->setIcon(0,QIcon(":icons/filetype.png"));
		item->setExpanded(true);
		std::vector<nPhysD*> retlist = nHolder::getInstance().fileOpen(fname.toStdString());
		for (auto& img: retlist) {
			addPhys(img,item);
		}
	}
}

void holderGUI::addPhys(nPhysD* my_phys,QTreeWidgetItem *item) {
	graphicsView->showPhys(my_phys);
	QTreeWidgetItem *my_item=new QTreeWidgetItem(item,QStringList(QString::fromStdString(my_phys->getName())));
	QVariant my_var=QVariant::fromValue(qobject_cast<nPhysD*>(my_phys));
	qDebug() << my_phys << my_var;
	my_item->setData(1,0,my_var);
	my_item->setIcon(0,QIcon(":icons/icon.png"));
	connect(my_phys, SIGNAL(destroyed(QObject*)), this, SLOT(delPhys(QObject*)));
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
