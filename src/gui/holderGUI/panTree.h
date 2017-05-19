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

#include <QTreeWidget>

#ifndef __panTree_h
#define __panTree_h

class genericPan;
class nPhysD;

class panTree : public QTreeWidget {
    Q_OBJECT
public:	
	Q_INVOKABLE panTree(QWidget *parent);
	~panTree(){};
    QPoint dragposition;
    QList<QTreeWidgetItem*> dragitems;
    QTime dragtime;

public slots:
    void dropEvent(QDropEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

	void addPanPhys(nPhysD* my_phys);
	void delPanPhys(nPhysD* my_phys);

	void itemPressed(QTreeWidgetItem* item, int col);
	void itemSelectionChanged();
	void itemDoubleClicked(QTreeWidgetItem* item, int col);

	void registerPan(genericPan*);
	void unregisterPan(QObject*);

	void keyPressEvent(QKeyEvent *);

private:
	int viewerUID;

};



#endif
