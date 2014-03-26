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

#include <unistd.h>

#include "nPreferences.h"
#include "neutrino.h"


#ifdef	__WIN32
#include <windows.h>
#endif

nPreferences::nPreferences(neutrino *nparent, QString winname)
: nGenericPan(nparent, winname) {
	my_w.setupUi(this);

	int coreNum =1;
#ifdef	__WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );

	coreNum = sysinfo.dwNumberOfProcessors;
#else
	coreNum=sysconf( _SC_NPROCESSORS_ONLN );
#endif

	my_w.threads->setMaximum(coreNum);

	if (coreNum==1) {
		my_w.threads->hide();
		my_w.labelThreads->hide();
	}
	if (!cudaEnabled()) {
		DEBUG("cuda not enabled");
		my_w.useCuda->setChecked(false);
		my_w.useCuda->setEnabled(false);
	} else {
		my_w.useCuda->setEnabled(true);
	}
	

	
	loadDefaults();
	decorate();
	
	connect(my_w.comboIconSize, SIGNAL(currentIndexChanged(int)), this, SLOT(changeIconSize(int)));
	connect(my_w.chooseFont, SIGNAL(pressed()), this, SLOT(changeFont()));
	connect(my_w.showDimPixel, SIGNAL(released()), this, SLOT(changeShowDimPixel()));

	my_w.labelFont->setFont(nparent->my_w.my_view->font());
	my_w.labelFont->setText(nparent->my_w.my_view->font().family()+" "+QString::number(nparent->my_w.my_view->font().pointSize()));
	
	

}

void nPreferences::changeShowDimPixel() {
	nparent->my_w.my_view->showDimPixel=my_w.showDimPixel->isChecked();
	
	nparent->my_w.my_view->update();
}

void nPreferences::changeFont() {
	bool ok;
	QFont font = QFontDialog::getFont(&ok, nparent->my_w.my_view->font(), this);
	if (ok) {
		nparent->my_w.my_view->setFont(font);
		my_w.labelFont->setFont(font);
		my_w.labelFont->setText(font.family()+" "+QString::number(font.pointSize()));
		QSettings settings("neutrino","");
		settings.beginGroup("Preferences");
		settings.setValue("defaultFont",font.toString());
		settings.endGroup();
		nparent->my_w.my_view->setSize();
	}
}

void nPreferences::changeIconSize(int val) {
	QSize mysize=QSize(10*(val+1),10*(val+1));
	
	foreach (QToolBar *obj, nparent->findChildren<QToolBar *>()) {
		if (obj->iconSize()!=mysize) {
			obj->hide();
			obj->setIconSize(mysize);
			obj->show();
		}
	}
	foreach (nGenericPan* pan, nparent->getPans()) {
		foreach (QToolBar *obj, pan->findChildren<QToolBar *>()) {
			if (obj->iconSize()!=mysize) {
				obj->hide();
				obj->setIconSize(mysize);
				obj->show();
			}
		}
	}
}

void nPreferences::hideEvent(QHideEvent*){
	disconnect(my_w.comboIconSize, SIGNAL(currentIndexChanged(int)), this, SLOT(changeIconSize(int)));
	saveDefaults();
}

void nPreferences::showEvent(QShowEvent*){
	loadDefaults();
	connect(my_w.comboIconSize, SIGNAL(currentIndexChanged(int)), this, SLOT(changeIconSize(int)));
};
