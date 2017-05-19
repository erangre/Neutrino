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
#include "panTree.h"
#include "genericPan/genericPan.h"

panTree::panTree(QWidget *parent):
    QTreeWidget(parent),
    viewerUID(0)
{
	qDebug() << "ctor";
	qDebug() << parent->parent();
	setAcceptDrops(true);
	setHeaderHidden(true);
	QFont my_font=QTreeWidget::font();
	my_font.setPointSize(10);
	setFont(my_font);

	connect(this, SIGNAL(itemPressed(QTreeWidgetItem*,int)), this, SLOT(itemPressed(QTreeWidgetItem*,int)));
	connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));
}

void panTree::mousePressEvent(QMouseEvent *e) {
	dragitems.clear();
	foreach (QTreeWidgetItem * item, selectedItems()) {
		dragitems << item;
	}
	dragposition=e->pos();
	dragtime.start();
	QTreeWidget::mousePressEvent(e);
}

void panTree::mouseMoveEvent(QMouseEvent *e) {
//	if (dragitems.size() && (dragposition - e->pos()).manhattanLength()>=QApplication::startDragDistance() && dragtime.elapsed() > qApp->startDragTime() ) {
//		genericPan* my_view=qobject_cast<genericPan *> (parent());
//		if (my_view) {
//			QByteArray dragPhysPointers;
//			QList<QUrl> lista;
//			foreach (QTreeWidgetItem * item, dragitems) {
//				genericPan *my_phys=(genericPan*) (item->data((columnCount()-1),0).value<void*>());
//				if (my_phys) {
//					dragPhysPointers+=QByteArray::number((qlonglong) my_phys)+ " ";
//					lista << QUrl(QString::fromUtf8(my_phys->getName().c_str()));
//				}
//			}
//			if (lista.size()) {
//				QMimeData *mymimeData=new QMimeData;
//				mymimeData->setUrls(lista);
//				mymimeData->setData(QString("data/neutrino"), dragPhysPointers);
//				QDrag *drag = new QDrag(this);
//				drag->setMimeData(mymimeData);
//				drag->exec();
//			}
//		}
//	}
}

void panTree::mouseReleaseEvent(QMouseEvent *e) {
	dragitems.clear();
	genericPan* my_pan=qobject_cast<genericPan *> (parent());
	if (my_pan) {
		if (e->modifiers() == Qt::NoModifier) {
			QTreeWidgetItem *item=itemAt(e->pos());
			if (item) {
//				genericPan *my_pan=(genericPan*) (item->data(columnCount()-1,Qt::DisplayRole).value<void*>());
//				my_pan->showPhys(phys);
			}
		}
	}
	QTreeWidget::mouseReleaseEvent(e);
}

// Drag and Drop
void panTree::dragEnterEvent(QDragEnterEvent *e)
{
	e->acceptProposedAction();
}

void panTree::dragMoveEvent(QDragMoveEvent *e)
{
	e->acceptProposedAction();
}

void panTree::dropEvent(QDropEvent *e) {
	genericPan* my_pan=qobject_cast<genericPan *> (parent());
	if (my_pan) {
//		my_pan->dropEvent(e);
	}
	e->acceptProposedAction();
	dragitems.clear();
}


void panTree::itemPressed(QTreeWidgetItem* itemPhys, int col) {
	qDebug() << "here";
	if (itemPhys->parent()) {
		genericPan *my_pan=itemPhys->parent()->data(1,0).value<genericPan*>();
		if (my_pan) {
//			my_pan->showPhys(itemPhys->data(1,0).value<genericPan*>());
		}
	}
}

void panTree::itemSelectionChanged() {
	QList<QTreeWidgetItem *> items=selectedItems();
	if (items.size()==1) {
		genericPan *my_pan=items.first()->data(1,0).value<genericPan*>();
		if(my_pan) {
			if (items.first()->parent()) {
				genericPan *my_view=items.first()->parent()->data(1,0).value<genericPan*>();
				if (my_view) {
//					my_view->showPhys(items.first()->data(1,0).value<genericPan*>());
				}
			}
		}
	}
}

void panTree::itemDoubleClicked(QTreeWidgetItem* item, int col) {

}

void panTree::registerPan(genericPan* my_pan) {
	QTreeWidgetItem *my_item=new QTreeWidgetItem(this, QStringList("genericPan "+ QString::number(viewerUID++)));
	my_item->setData(1,0,QVariant::fromValue(my_pan));
	my_item->setExpanded(true);
	connect(my_pan, SIGNAL(destroyed(QObject*)), this, SLOT(unregisterPan(QObject*)));
	connect(my_pan, SIGNAL(addPanPhys(genericPan*)), this, SLOT(addPanPhys(genericPan*)));
	connect(my_pan, SIGNAL(delPanPhys(genericPan*)), this, SLOT(delPanPhys(genericPan*)));
}

void panTree::unregisterPan(QObject *my_obj) {
	qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << my_obj;
	for (int i=0; i< topLevelItemCount() ; i++) {
		QTreeWidgetItem *item = topLevelItem(i);
		if (item->data(1,0)==QVariant::fromValue(my_obj)) {
			delete item;
		}
	}
}

void panTree::addPanPhys (nPhysD* my_phys) {
	genericPan* my_view=qobject_cast<genericPan*>(sender());
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

void panTree::delPanPhys (nPhysD* my_phys) {
	genericPan* my_view=qobject_cast<genericPan*>(sender());
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

void panTree::keyPressEvent (QKeyEvent *event) {
	QTreeWidget::keyPressEvent(event);
	for (auto& item: selectedItems()) {
		genericPan *my_phys=item->data(1,0).value<genericPan*>();
		if(my_phys) {
			if (item->parent()) {
				genericPan *my_view=item->parent()->data(1,0).value<genericPan*>();
				if (my_view) {
					switch (event->key()) {
						case Qt::Key_Backspace:
//							my_view->delPhys(my_phys);
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

