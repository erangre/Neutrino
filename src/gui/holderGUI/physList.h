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
#include <iostream>

#include <QtGui>
#include <QWidget>
#include <QObject>

#include <QListWidget>

#ifndef __physList_h
#define __physList_h

class nPhysD;

class physList : public QListWidget {
    Q_OBJECT
public:	
	Q_INVOKABLE physList(QWidget *parent);
	~physList(){};
    QPoint dragposition;
	QList<QListWidgetItem*> dragitems;
    QTime dragtime;

public slots:
    void dropEvent(QDropEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

	void itemPressed(QListWidgetItem* item);
	void itemSelectionChanged();
	void itemDoubleClicked(QListWidgetItem* item);

	void keyPressEvent(QKeyEvent *);

	void registerPhys(nPhysD*);
	void unregisterPhys(QObject* my_phys);

private:
	int physUID;

signals:
	void nPhysDSelected(nPhysD*);
	void nPhysDDoubleClick(nPhysD*);
	void nPhysDClick(nPhysD*);
	void addPhys(nPhysD*);



};



#endif
