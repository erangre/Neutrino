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
#include "physList.h"
#include "graphics/nView.h"
#include "nPhysD.h"
#include "nApp.h"

physList::physList(QWidget *parent):
    QListWidget(parent),
    physUID(0)
{
	qDebug() << "ctor";
	qDebug() << parent->parent();
	setAcceptDrops(true);

	QFont my_font=QListWidget::font();
	my_font.setPointSize(10);
	setFont(my_font);

	setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(this, SIGNAL(itemPressed(QListWidgetItem*)), this, SLOT(itemPressed(QListWidgetItem*)));

}

void physList::mousePressEvent(QMouseEvent *e) {
	qDebug() << "here" << selectedItems();

	dragitems =  QListWidget::selectedItems();
    dragposition=e->pos();
    dragtime.start();
	QListWidget::mousePressEvent(e);
}

void physList::mouseMoveEvent(QMouseEvent *e) {
	if (dragitems.size() && (dragposition - e->pos()).manhattanLength()>=QApplication::startDragDistance() && dragtime.elapsed() > QApplication::startDragTime() ) {
		qDebug() << "here" << e->pos();
		QByteArray dragPhysPointers;
		foreach (QListWidgetItem * item, dragitems) {
			nPhysD *my_phys=item->data(1).value<nPhysD*>();
			if (my_phys) {
				qDebug() << "here";
				dragPhysPointers+=my_phys->to_ByteArray();
			}
		}
		qDebug() << dragPhysPointers.size();
		if (dragPhysPointers.size()) {
//			qDebug() << "here" << e->pos() << dragPhysPointers;
			QMimeData *mymimeData=new QMimeData;
			mymimeData->setData(QString("data/neutrino"), dragPhysPointers);
			QDrag *drag = new QDrag(this);
			drag->setMimeData(mymimeData);
			drag->exec();
		}
		dragitems.clear();
	}
}

void physList::mouseReleaseEvent(QMouseEvent *e) {
    dragitems.clear();
	QListWidget::mouseReleaseEvent(e);
}

// Drag and Drop
void physList::dragEnterEvent(QDragEnterEvent *e)
{
	qDebug() << "here" << e;
    e->acceptProposedAction();
}

void physList::dragMoveEvent(QDragMoveEvent *e)
{
	qDebug() << "here" << e;
	e->acceptProposedAction();
}

void physList::dropEvent(QDropEvent *e) {
	nApp::holder()->dropEvent(e);
	dragitems.clear();
}

void physList::keyPressEvent (QKeyEvent *event) {
	QList<nPhysD*> selectedPhys;
	for (auto& item: selectedItems()) {
		nPhysD *my_phys=item->data(1).value<nPhysD*>();
		if(my_phys) {
			selectedPhys.append(my_phys);
		}
	}
	switch (event->key()) {
		case Qt::Key_Backspace:
			for (auto& phys: selectedPhys)
				phys->deleteLater();
			QApplication::processEvents();
			break;
		default:
			break;
	}
	QListWidget::keyPressEvent(event);
}

void physList::registerPhys(nPhysD* my_phys) {
	my_phys->prop["physUID"]=physUID++;
	QListWidgetItem *my_item=new QListWidgetItem(QString::fromStdString(my_phys->getName()), this);
	QVariant my_var=QVariant::fromValue(qobject_cast<nPhysD*>(my_phys));
	qDebug() << my_phys << my_var;
	my_item->setData(1,my_var);
	connect(my_phys, SIGNAL(destroyed(QObject*)), this, SLOT(unregisterPhys(QObject*)));
	emit addPhys(my_phys);
}

void physList::unregisterPhys(QObject* my_obj) {
	nPhysD *my_phys=static_cast<nPhysD*>(my_obj);
	if (my_phys) {
		for(int i = 0; i < count(); ++i) {
			QListWidgetItem* item = this->item(i);
			if (item->data(1) == QVariant::fromValue(my_phys)) {
				delete item;
			}
		}
	}
}

void physList::itemPressed(QListWidgetItem* item) {
	nPhysD *my_phys=item->data(1).value<nPhysD*>();
	if (my_phys) {
		emit nPhysDClick(my_phys);
	}
	qDebug() << item->data(1).typeName();
}

void physList::itemSelectionChanged() {
	QList<QListWidgetItem *> items= selectedItems();
	if (items.size()==1) {
		nPhysD *my_phys=items.first()->data(1).value<nPhysD*>();
		if(my_phys) {
			emit nPhysDSelected(my_phys);
		}
	}
}

void physList::itemDoubleClicked(QListWidgetItem* item) {
	nPhysD *my_phys=item->data(1).value<nPhysD*>();
	if (my_phys) {
		emit nPhysDDoubleClick(my_phys);
	}

}

