#include "holderGUI.h"
#include <QImageReader>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QTreeWidgetItem>

#include "graphics/nView.h"

holderGUI::holderGUI() : QMainWindow() ,
    physUID(0),
    viewerUID(0)
{
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

		for (auto& img: allPhys()) {
			my_view->showPhys(img);
		}
	}
	physViewer->show();
}


void holderGUI::registerPhys(nPhysD* my_phys) {
	my_phys->prop["physUID"]=physUID++;

	QListWidgetItem *my_item=new QListWidgetItem(QIcon(":icons/icon.png"), QString::fromStdString(my_phys->getName()), physList);
	QVariant my_var=QVariant::fromValue(qobject_cast<nPhysD*>(my_phys));
	qDebug() << my_phys << my_var;
	my_item->setData(1,my_var);
	connect(my_phys, SIGNAL(destroyed(QObject*)), this, SLOT(delPhys(QObject*)));
	emit addPhys(my_phys);
}


void holderGUI::registerViewer(nView* my_view) {
	QTreeWidgetItem *my_item=new QTreeWidgetItem(viewerTree, QStringList(QString::number(viewerUID++)));
	my_item->setData(1,0,QVariant::fromValue(my_view));
	connect(my_view, SIGNAL(destroyed(QObject*)), this, SLOT(unregisterViewer(QObject*)));
	connect(my_view, SIGNAL(addPhys(nPhysD*)), this, SLOT(addViewPhys(nPhysD*)));
	connect(my_view, SIGNAL(delPhys(nPhysD*)), this, SLOT(delViewPhys(nPhysD*)));
}

void holderGUI::unregisterViewer(QObject *my_obj) {
	nView *my_view=qobject_cast<nView*>(my_obj);
	QTreeWidgetItemIterator it(viewerTree);
	while (*it) {
		if ((*it)->data(1,0)==QVariant::fromValue(my_view)) {
			delete (*it);
		}
		++it;
	}
}

void holderGUI::addViewPhys (nPhysD* my_phys) {
	nView* my_view=qobject_cast<nView*>(sender());
	if (my_view) {
		for (int i=0; i< viewerTree->topLevelItemCount() ; i++) {
			QTreeWidgetItem *item = viewerTree->topLevelItem(i);
			if (item->data(1,0)==QVariant::fromValue(my_view)) {
				QTreeWidgetItem *my_item=new QTreeWidgetItem(item,QStringList(QString::fromStdString(my_phys->getName())));
				QVariant my_var=QVariant::fromValue(qobject_cast<nPhysD*>(my_phys));
				qDebug() << my_phys << my_var;
				my_item->setData(1,0,my_var);
				my_item->setIcon(0,QIcon(":icons/icon.png"));
			}
		}
	}
}

void holderGUI::delViewPhys (nPhysD* my_phys) {
	DEBUG(my_phys->getName());
	nView* my_view=qobject_cast<nView*>(sender());
	if (my_view) {
		for (int i=0; i< viewerTree->topLevelItemCount() ; i++) {
			QTreeWidgetItem *item = viewerTree->topLevelItem(i);
			if (item->data(1,0)==QVariant::fromValue(my_view)) {
				for (int i=0; i <item->childCount(); i++) {
					QTreeWidgetItem *itemChild = item->child(i);
					if (itemChild->data(1,0)==QVariant::fromValue(my_phys)) {
						delete itemChild;
					}
				}
			}
		}
	}
}
void holderGUI::delPhys(QObject* my_obj) {
	nPhysD *my_phys=static_cast<nPhysD*>(my_obj);
	if (my_phys) {
		for(int i = 0; i < physList->count(); ++i) {
			QListWidgetItem* item = physList->item(i);
			if (item->data(1) == QVariant::fromValue(my_phys)) {
				delete item;
			}
		}
	}
}


std::vector<nPhysD*> holderGUI::allPhys() {
	std::vector<nPhysD*> retvec;
	for(int i = 0; i < physList->count(); ++i) {
		nPhysD *my_phys=physList->item(i)->data(1).value<nPhysD*>();
		if (my_phys) {
			retvec.push_back(my_phys);
		}
	}
	return retvec;
}

void holderGUI::keyPressEvent (QKeyEvent *event) {
	QList<nPhysD*> selectedPhys;
	for (auto& item: physList->selectedItems()) {
		nPhysD *my_phys=item->data(1).value<nPhysD*>();
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


void holderGUI::saveDefaults(){
	QSettings my_set("neutrino","");
	my_set.beginGroup("nPreferences");
	my_set.setValue("geometry", pos());

	my_set.beginGroup("Properties");
	foreach(QByteArray ba, dynamicPropertyNames()) {
		if(ba.startsWith("NeuSave")) {
			my_set.setValue(ba, property(ba));
		}
	}
	my_set.endGroup();
	my_set.endGroup();
}

void holderGUI::loadDefaults(){
	QSettings my_set("neutrino","");
	my_set.beginGroup("nPreferences");
	move(my_set.value("geometry",pos()).toPoint());

	if (my_set.childGroups().contains("Properties")) {
		my_set.beginGroup("Properties");
		foreach(QString my_key, my_set.allKeys()) {
			setProperty(my_key.toUtf8().constData(), my_set.value(my_key));
			qDebug() << my_key;
		}
		my_set.endGroup();
	}

	my_set.endGroup();
}

void holderGUI::closeEvent (QCloseEvent *e) {
	saveDefaults();
	e->accept();
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


void holderGUI::logging(QString msg) {
	qDebug() << sender() << ":" << msg;
}

void holderGUI::on_physList_itemDoubleClicked(QListWidgetItem* item) {
	on_actionViewer_triggered();
	nPhysD *my_phys=item->data(1).value<nPhysD*>();
	if (my_phys) {
		emit nPhysDSelected(my_phys);
	}

}

void holderGUI::on_physList_itemPressed(QListWidgetItem* item) {
	nPhysD *my_phys=item->data(1).value<nPhysD*>();
	if (my_phys) {
		emit nPhysDClick(my_phys);
	}
	qDebug() << item->data(1).typeName();
}

void holderGUI::on_physList_itemSelectionChanged() {
	qDebug() << "-->>> here";
	QList<QListWidgetItem *> items= physList->selectedItems();
	if (items.size()==1) {
		nPhysD *my_phys=items.first()->data(1).value<nPhysD*>();
		if(my_phys) {
			qDebug() << "-->>> there";

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

std::vector<nPhysD*> holderGUI::fileOpen(QString fname) {

	setProperty("NeuSave-fileOpen", fname);
	std::vector<nPhysD*> retlist;

	for (auto &my_phys : phys_open(fname.toStdString())) {
		nPhysD* my_nphys=new nPhysD(my_phys);
		retlist.push_back(my_nphys);
	}

	return retlist;
}

std::vector<nPhysD*> holderGUI::openFiles(QStringList fnames) {
	std::vector<nPhysD*> retlist;
	foreach (QString fname, fnames) {
		for (auto& img: fileOpen(fname)) {
			retlist.push_back(img);
		}
	}
	return retlist;
}
