/*
 *
 *    Copyright (C) 2013 Alessandro Flacco, Tommaso Vinci All Rights Reserved
 *
 *    This file is part of neutrino.
 *
 *    Neutrino is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Neutrino is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with neutrino.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    Contact Information:
 *	Alessandro Flacco <alessandro.flacco@polytechnique.edu>
 *	Tommaso Vinci <tommaso.vinci@polytechnique.edu>
 *
 */
#include "viewerTree.h"
#include "graphics/nView.h"
#include "nPhysD.h"

viewerTree::viewerTree(QWidget *parent):
    QTreeWidget(parent),
    viewerUID(0)
{
	qDebug() << "ctor";
	qDebug() << parent->parent();
	setAcceptDrops(true);

	connect(this, SIGNAL(itemPressed(QTreeWidgetItem*,int)), this, SLOT(itemPressed(QTreeWidgetItem*,int)));
	connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));
}

void viewerTree::mousePressEvent(QMouseEvent *e) {
	dragitems.clear();
	foreach (QTreeWidgetItem * item, selectedItems()) {
		dragitems << item;
	}
	dragposition=e->pos();
	dragtime.start();
	QTreeWidget::mousePressEvent(e);
}

void viewerTree::mouseMoveEvent(QMouseEvent *e) {
	if (dragitems.size() && (dragposition - e->pos()).manhattanLength()>=QApplication::startDragDistance() && dragtime.elapsed() > qApp->startDragTime() ) {
		nView* my_view=qobject_cast<nView *> (parent());
		if (my_view) {
			QByteArray dragPhysPointers;
			QList<QUrl> lista;
			foreach (QTreeWidgetItem * item, dragitems) {
				nPhysD *my_phys=(nPhysD*) (item->data((columnCount()-1),0).value<void*>());
				if (my_phys) {
					dragPhysPointers+=QByteArray::number((qlonglong) my_phys)+ " ";
					lista << QUrl(QString::fromUtf8(my_phys->getName().c_str()));
				}
			}
			if (lista.size()) {
				QMimeData *mymimeData=new QMimeData;
				mymimeData->setUrls(lista);
				mymimeData->setData(QString("data/neutrino"), dragPhysPointers);
				QDrag *drag = new QDrag(this);
				drag->setMimeData(mymimeData);
				drag->exec();
			}
		}
	}
}

void viewerTree::mouseReleaseEvent(QMouseEvent *e) {
	dragitems.clear();
	nView* my_view=qobject_cast<nView *> (parent());
	if (my_view) {
		if (e->modifiers() == Qt::NoModifier) {
			QTreeWidgetItem *item=itemAt(e->pos());
			if (item) {
				nPhysD *phys=(nPhysD*) (item->data(columnCount()-1,Qt::DisplayRole).value<void*>());
				my_view->showPhys(phys);
			}
		}
	}
	QTreeWidget::mouseReleaseEvent(e);
}

// Drag and Drop
void viewerTree::dragEnterEvent(QDragEnterEvent *e)
{
	e->acceptProposedAction();
}

void viewerTree::dragMoveEvent(QDragMoveEvent *e)
{
	e->acceptProposedAction();
}

void viewerTree::dropEvent(QDropEvent *e) {
	nView* my_view=qobject_cast<nView *> (parent());
	if (my_view) {
		my_view->dropEvent(e);
	}
	e->acceptProposedAction();
	dragitems.clear();
}


void viewerTree::itemPressed(QTreeWidgetItem* itemPhys, int col) {
	qDebug() << "here";
	if (itemPhys->parent()) {
		nView *my_view=itemPhys->parent()->data(1,0).value<nView*>();
		if (my_view) {
			my_view->showPhys(itemPhys->data(1,0).value<nPhysD*>());
		}
	}
}

void viewerTree::itemSelectionChanged() {
	QList<QTreeWidgetItem *> items=selectedItems();
	if (items.size()==1) {
		nPhysD *my_phys=items.first()->data(1,0).value<nPhysD*>();
		if(my_phys) {
			if (items.first()->parent()) {
				nView *my_view=items.first()->parent()->data(1,0).value<nView*>();
				if (my_view) {
					my_view->showPhys(items.first()->data(1,0).value<nPhysD*>());
				}
			}
		}
	}
}

void viewerTree::itemDoubleClicked(QTreeWidgetItem* item, int col) {

}

void viewerTree::registerViewer(nView* my_view) {
	QTreeWidgetItem *my_item=new QTreeWidgetItem(this, QStringList("Viewer "+ QString::number(viewerUID++)));
	my_item->setData(1,0,QVariant::fromValue(my_view));
	my_item->setExpanded(true);
	connect(my_view, SIGNAL(destroyed(QObject*)), this, SLOT(unregisterViewer(QObject*)));
	connect(my_view, SIGNAL(addViewPhys(nPhysD*)), this, SLOT(addViewPhys(nPhysD*)));
	connect(my_view, SIGNAL(delViewPhys(nPhysD*)), this, SLOT(delViewPhys(nPhysD*)));
}

void viewerTree::unregisterViewer(QObject *my_obj) {
	qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << my_obj;
	for (int i=0; i< topLevelItemCount() ; i++) {
		QTreeWidgetItem *item = topLevelItem(i);
		if (item->data(1,0)==QVariant::fromValue(my_obj)) {
			delete item;
		}
	}
}

void viewerTree::addViewPhys (nPhysD* my_phys) {
	nView* my_view=qobject_cast<nView*>(sender());
	if (my_view) {
		for (int i=0; i< topLevelItemCount() ; i++) {
			QTreeWidgetItem *item = topLevelItem(i);
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

void viewerTree::delViewPhys (nPhysD* my_phys) {
	nView* my_view=qobject_cast<nView*>(sender());
	if (my_view) {
		for (int i=0; i< topLevelItemCount() ; i++) {
			QTreeWidgetItem *item = topLevelItem(i);
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

void viewerTree::keyPressEvent (QKeyEvent *event) {
	QTreeWidget::keyPressEvent(event);
	for (auto& item: selectedItems()) {
		nPhysD *my_phys=item->data(1,0).value<nPhysD*>();
		if(my_phys) {
			if (item->parent()) {
				nView *my_view=item->parent()->data(1,0).value<nView*>();
				if (my_view) {
					switch (event->key()) {
						case Qt::Key_Backspace:
							my_view->delPhys(my_phys);
							break;
						default:
							break;
					}
				}
			}

		}
	}
	event->accept();
}

