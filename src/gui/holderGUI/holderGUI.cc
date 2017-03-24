#include "holderGUI.h"
#include <QImageReader>
#include <QFileDialog>
#include <QTreeWidgetItem>
#include "graphics/nView.h"

holderGUI::holderGUI() : QMainWindow() {
	setupUi(this);
	setAcceptDrops(true);
	show();
}

void holderGUI::on_actionViewer_triggered() {
	if (physViewer.isNull()) {
		physViewer = new QMainWindow(this);
		physViewer->setAttribute(Qt::WA_DeleteOnClose);

		physViewer->setUnifiedTitleAndToolBarOnMac(true);
		nView *my_view = new nView(physViewer);
		physViewer->setAttribute(Qt::WA_DeleteOnClose);

		physViewer->setCentralWidget(my_view);
		physViewer->resize(400,400);

		QStatusBar *statusBar = new QStatusBar(physViewer);
		QFont my_font(statusBar->font());
		my_font.setPointSize(10);
		statusBar->setFont(my_font);
		physViewer->setStatusBar(statusBar);

		connect(this, SIGNAL(nPhysDSelected(nPhysD*)), my_view, SLOT(showPhys(nPhysD*)));
		connect(this, SIGNAL(addPhys(nPhysD*)), my_view, SLOT(showPhys(nPhysD*)));
		connect(my_view, SIGNAL(logging(QString)), statusBar, SLOT(showMessage(QString)));
		connect(my_view, SIGNAL(addPhys(nPhysD*)), this, SLOT(addViewPhys(nPhysD*)));

		QTreeWidgetItem *item=new QTreeWidgetItem(treeWidget,QStringList(QString("viewer")));
		item->setData(1,0,QVariant::fromValue(my_view));
		item->setExpanded(true);
	}
	physViewer->show();
}

void holderGUI::dragEnterEvent(QDragEnterEvent *e)
{
	Ui_holderGUI::statusBar->showMessage(tr("Drop content"), 2000);
	e->acceptProposedAction();
}

void holderGUI::dragMoveEvent(QDragMoveEvent *e)
{
	e->accept();
}

void holderGUI::dropEvent(QDropEvent *e) {
	qDebug() << "here";
	if (e->mimeData()->hasUrls()) {
		e->acceptProposedAction();
		foreach (QUrl qurl, e->mimeData()->urls()) {
			openFiles(QStringList(qurl.toLocalFile()));
		}
	}
}

void holderGUI::addViewPhys (nPhysD* my_phys) {
	qDebug() << "----------------------------------------------" << my_phys;
	nView* my_view=qobject_cast<nView*>(sender());
	if (my_view) {
		for (int i=0; i< treeWidget->topLevelItemCount() ; i++) {
			QTreeWidgetItem *item = treeWidget->topLevelItem(i);
			if (item->data(1,0)==QVariant::fromValue(my_view)) {

				QTreeWidgetItem *my_item=new QTreeWidgetItem(item,QStringList(QString::fromStdString(my_phys->getName())));
				QVariant my_var=QVariant::fromValue(qobject_cast<nPhysD*>(my_phys));
				qDebug() << my_phys << my_var;
				my_item->setData(1,0,my_var);
				my_item->setIcon(0,QIcon(":icons/icon.png"));
				connect(my_phys, SIGNAL(destroyed(QObject*)), this, SLOT(delPhys(QObject*)));
			}
		}
	}
}

void holderGUI::keyPressEvent (QKeyEvent *event) {
	QList<nPhysD*> selectedPhys;
	for (auto& item: treeWidget->selectedItems()) {
		for (int i=0; i< item->childCount(); i++) {
			 nPhysD *my_phys=item->child(i)->data(1,0).value<nPhysD*>();
			 if(my_phys) {
				 selectedPhys.append(my_phys);
			 }
		}
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

void holderGUI::logging(QString msg) {
	qDebug() << sender() << ":" << msg;
}

void holderGUI::on_treeWidget_itemDoubleClicked(QTreeWidgetItem* item, int i) {
	on_actionViewer_triggered();
	nPhysD *my_phys=item->data(1,0).value<nPhysD*>();
	if (my_phys) {
		emit nPhysDSelected(my_phys);
	}

}

void holderGUI::on_treeWidget_itemPressed(QTreeWidgetItem* item, int i) {
	nPhysD *my_phys=item->data(1,0).value<nPhysD*>();
	if (my_phys) {
		emit nPhysDClick(my_phys);
	}
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

std::vector<nPhysD*> holderGUI::fileOpen(std::string fname) {

	std::vector<nPhysD*> retlist;

	for (auto &my_phys : phys_open(fname)) {
		nPhysD* my_nphys=new nPhysD(my_phys);
		retlist.push_back(my_nphys);
	}

	return retlist;
}

void holderGUI::openFiles(QStringList fnames) {
	setProperty("NeuSave-fileOpen", fnames);
	foreach (QString fname, fnames) {
		QTreeWidgetItem *item=new QTreeWidgetItem(treeWidget,QStringList(fname));
		item->setIcon(0,QIcon(":icons/filetype.png"));
		item->setExpanded(true);
		std::vector<nPhysD*> retlist = fileOpen(fname.toStdString());
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
