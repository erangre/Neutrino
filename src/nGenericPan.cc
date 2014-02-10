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
#include "nGenericPan.h"
#include "neutrino.h"

nGenericPan::~nGenericPan()
{
	foreach (QWidget *widget, QApplication::allWidgets()) {
		neutrino *neu=qobject_cast<neutrino *>(widget);
		if (neu==nparent) {
			disconnect(nparent, SIGNAL(mouseAtMatrix(QPointF)), this, SLOT(mouseAtMatrix(QPointF)));
			disconnect(nparent, SIGNAL(mouseAtWorld(QPointF)), this, SLOT(mouseAtWorld(QPointF)));
			
			disconnect(nparent, SIGNAL(nZoom(double)), this, SLOT(nZoom(double)));
			
			disconnect(nparent->my_w.my_view, SIGNAL(mousePressEvent_sig(QPointF)), this, SLOT(imageMousePress(QPointF)));
			disconnect(nparent->my_w.my_view, SIGNAL(mouseReleaseEvent_sig(QPointF)), this, SLOT(imageMouseRelease(QPointF)));
			disconnect(nparent, SIGNAL(bufferChanged(nPhysD *)), this, SLOT(bufferChanged(nPhysD *)));
			nparent->emitPanDel(this);
		}
	}
	emit unregister_paintPath(this);
}

nGenericPan::nGenericPan(neutrino *parent, QString name)
: QMainWindow(), nparent(parent), panName(name), currentBuffer(NULL)
{	
#ifdef Q_OS_MAC
	DEBUG("NEW OSX FEATURE have the main menu always visible!!! might break up things on every update!");
	setParent(nparent);
	setWindowFlags(Qt::Tool|Qt::WindowStaysOnBottomHint);
	grabKeyboard();
#endif

	setProperty("panName",panName);
	int panNum=0;
	foreach (QWidget *widget, QApplication::allWidgets()) {
		nGenericPan *pan=qobject_cast<nGenericPan *>(widget);
		if (pan && pan != this && pan->nparent == nparent) {
			if (pan->panName.contains(panName)) {
				panNum=max(pan->property("panNum").toInt(),panNum);				
			}
		}
	}
	panNum++;
	if (panNum>1) panName.append(QString::number(panNum));
	setProperty("panNum",panNum);
	
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(Qt::Window);

	connect(parent, SIGNAL(destroyed()), this, SLOT(close()));

	my_s=parent->getScene();

	connect(parent, SIGNAL(mouseAtMatrix(QPointF)), this, SLOT(mouseAtMatrix(QPointF)));
	connect(parent, SIGNAL(mouseAtWorld(QPointF)), this, SLOT(mouseAtWorld(QPointF)));

	connect(parent, SIGNAL(nZoom(double)), this, SLOT(nZoom(double)));

	connect(parent->my_w.my_view, SIGNAL(mousePressEvent_sig(QPointF)), this, SLOT(imageMousePress(QPointF)));
	connect(parent->my_w.my_view, SIGNAL(mouseReleaseEvent_sig(QPointF)), this, SLOT(imageMouseRelease(QPointF)));

	connect(parent, SIGNAL(bufferChanged(nPhysD *)), this, SLOT(bufferChanged(nPhysD *)));

	connect(parent, SIGNAL(physAdd(nPhysD*)), this, SLOT(physAdd(nPhysD*)));
	connect(parent, SIGNAL(physDel(nPhysD*)), this, SLOT(physDel(nPhysD*)));

	bufferChanged(nparent->currentBuffer);
	nparent->emitPanAdd(this);
}

QString nGenericPan::getNameForCombo(QComboBox* combo, nPhysD *buffer) {
//	if (!combo->property("physNameLength").isValid()) combo->setProperty("physNameLength",35);
	QString name="";
	if (nparent) {
		int position = nparent->physList.indexOf(buffer);
		name=QString::fromStdString(buffer->getName());
		int len=combo->property("physNameLength").toInt();
		if (name.length()>len) name=name.left((len-5)/2)+"[...]"+name.right((len-5)/2);
		name.prepend(QString::number(position)+" : ");
	} 
	return name;
}
	
void nGenericPan::addPhysToCombos(nPhysD *buffer) {
	foreach (QComboBox *combo, findChildren<QComboBox *>()) {
		if (combo->property("neutrinoImage").isValid()) {
			int alreadyThere = combo->findData(qVariantFromValue((void*) buffer));
			if (alreadyThere == -1) {
				combo->addItem(getNameForCombo(combo,buffer),qVariantFromValue((void*) buffer));
			}
		}
	}
}

void nGenericPan::decorate() {
//	qDebug() << __PRETTY_FUNCTION__ << panName << objectName() << metaObject()->className();
	setProperty("fileTxt", QString(panName+".txt"));
	setProperty("fileExport", QString(panName+".svg"));
	setProperty("fileIni", QString(panName+".ini"));
	neutrinoProperties << "fileTxt" << "fileExport" << "fileIni";

	setWindowTitle(nparent->property("winId").toString()+": "+panName);
	foreach (nPhysD *buffer, nparent->physList) addPhysToCombos(buffer);
	foreach (QComboBox *combo, findChildren<QComboBox *>()) {
		if (combo->property("neutrinoImage").isValid()) {	
			if (combo->property("neutrinoImage").toBool()) {
				//connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(comboChanged(int)));
				connect(combo,SIGNAL(highlighted(int)),this, SLOT(comboChanged(int)));
				connect(combo,SIGNAL(activated(int)),this, SLOT(comboChanged(int)));
				
				if (!combo->property("physNameLength").isValid()) combo->setProperty("physNameLength",35);

			}
		}
	}
	foreach (QWidget *wdgt, findChildren<QWidget *>()) {
		if (wdgt->property("neutrinoSave").isValid() || wdgt->property("neutrinoImage").isValid()) {
			wdgt->setToolTip(wdgt->toolTip()+" ["+wdgt->objectName()+"]");
		}
	}

	QSize iconSize;
	foreach (QToolBar *widget, nparent->findChildren<QToolBar *>()) {
		iconSize=widget->iconSize();
		widget->show();
		break;
	}
	foreach (QToolBar *widget, findChildren<QToolBar *>()) {
		widget->setIconSize(iconSize);
	}

	loadDefaults();
	show();
}

void
nGenericPan::physAdd(nPhysD * buffer) {
	DEBUG(5, buffer->getName());
	addPhysToCombos(buffer);
	QApplication::processEvents();
}

void
nGenericPan::physDel(nPhysD * buffer) {
	DEBUG(5, buffer->getName());
	foreach (QComboBox *combo, findChildren<QComboBox *>()) {
		if (combo->property("neutrinoImage").isValid()) {
			if (combo->property("neutrinoImage").toBool()) {
//				disconnect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(comboChanged(int)));
				disconnect(combo,SIGNAL(highlighted(int)),this, SLOT(comboChanged(int)));
				disconnect(combo,SIGNAL(activated(int)),this, SLOT(comboChanged(int)));
			}
			int position=combo->findData(qVariantFromValue((void*) buffer));
			DEBUG(5, "removed " << buffer->getName() << " " << combo->objectName().toStdString() << " " <<position);
			combo->removeItem(position);
			if (combo->property("neutrinoImage").toBool()) {
//				connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(comboChanged(int)));
				connect(combo,SIGNAL(highlighted(int)),this, SLOT(comboChanged(int)));
				connect(combo,SIGNAL(activated(int)),this, SLOT(comboChanged(int)));
			}
		}
	}
	QApplication::processEvents();
}

void
nGenericPan::bufferChanged(nPhysD * buffer)
{
	foreach (QComboBox *combo, findChildren<QComboBox *>()) {
		if (combo->property("neutrinoImage").isValid()) {
			int position=combo->findData(qVariantFromValue((void*) buffer));
			if (position >= 0) combo->setItemText(position,getNameForCombo(combo,buffer));
		}
	}
	currentBuffer = buffer;
}

void
nGenericPan::showMessage(QString message) {
	nparent->statusBar()->showMessage(message);
}

void
nGenericPan::showMessage(QString message,int msec) {
	nparent->statusBar()->showMessage(message,msec);
}


void
nGenericPan::comboChanged(int k) {
	QComboBox *combo = qobject_cast<QComboBox *>(sender());
	if (combo) {
		nPhysD *image=(nPhysD*) (combo->itemData(k).value<void*>());
		if (image) {
			nparent->showPhys(image);
		}
		DEBUG(panName.toStdString() << " " << combo->objectName().toStdString());
		emit changeCombo(combo);
	} else {
		DEBUG("not a combo");
	}
}

nPhysD* nGenericPan::getPhysFromCombo(QComboBox* combo) {
	return (nPhysD*) (combo->itemData(combo->currentIndex()).value<void*>());
}

void
nGenericPan::loadUi(QSettings *settings) {
	foreach (QLineEdit *widget, findChildren<QLineEdit *>()) {
		if (widget->property("neutrinoSave").isValid() && widget->property("neutrinoSave").toBool()) {
			widget->setText(settings->value(widget->objectName(),widget->text()).toString());
		}
	}
	foreach (QPlainTextEdit *widget, findChildren<QPlainTextEdit *>()) {
		if (widget->property("neutrinoSave").isValid() && widget->property("neutrinoSave").toBool()) {
			widget->setPlainText(settings->value(widget->objectName(),widget->toPlainText()).toString());
		}
	}
	foreach (QDoubleSpinBox *widget, findChildren<QDoubleSpinBox *>()) {
		if (widget->property("neutrinoSave").isValid() && widget->property("neutrinoSave").toBool()) widget->setValue(settings->value(widget->objectName(),widget->value()).toDouble());
	}
	foreach (QSpinBox *widget, findChildren<QSpinBox *>()) {
		if (widget->property("neutrinoSave").isValid() && widget->property("neutrinoSave").toBool()) widget->setValue(settings->value(widget->objectName(),widget->value()).toInt());
	}
	foreach (QTabWidget *widget, findChildren<QTabWidget *>()) {
		if (widget->property("neutrinoSave").isValid() && widget->property("neutrinoSave").toBool()) widget->setCurrentIndex(settings->value(widget->objectName(),widget->currentIndex()).toInt());
	}
	foreach (QCheckBox *widget, findChildren<QCheckBox *>()) {
		if (widget->property("neutrinoSave").isValid() && widget->property("neutrinoSave").toBool()) widget->setChecked(settings->value(widget->objectName(),widget->isChecked()).toBool());
	}
	foreach (QRadioButton *widget, findChildren<QRadioButton *>()) {
		if (widget->property("neutrinoSave").isValid() && widget->property("neutrinoSave").toBool()) widget->setChecked(settings->value(widget->objectName(),widget->isChecked()).toBool());
	}

	foreach (QComboBox *widget, findChildren<QComboBox *>()) {
		if (widget->property("neutrinoSave").isValid()) {
			QString currText=widget->currentText();
			if (widget->property("neutrinoSave").toBool()) {
				QStringList lista;
				for (int i=0; i< widget->count(); i++) {
					lista << widget->itemText(i);
				}
				lista << settings->value(widget->objectName(),lista).toStringList();
				lista.removeDuplicates();
				widget->clear();
				widget->addItems(lista);
			}
			widget->setCurrentIndex(settings->value(widget->objectName()+"Default",0).toInt());
		}
		if (widget->property("neutrinoImage").isValid() && widget->property("neutrinoImage").toBool()) {
			string imageName=settings->value(widget->objectName()).toString().toStdString();
			foreach (nPhysD *physAperto,nparent->physList) {
				if (physAperto->getName()==imageName) {
					for (int i=0; i<widget->count();i++) {
						if (physAperto==(nPhysD*) (widget->itemData(i).value<void*>())) {
							widget->setCurrentIndex(i);
							break;
						}
					}
				}
			}
		}
	}

	foreach (QObject* widget, nparent->children()) {
		nLine *linea=qobject_cast<nLine *>(widget);
		if (linea && linea->property("parentPan").toString()==panName) {
			linea->loadSettings(settings);
		}
		nRect *rect=qobject_cast<nRect *>(widget);
		if (rect && rect->property("parentPan").toString()==panName) {
			rect->loadSettings(settings);
		}
		nEllipse *elli=qobject_cast<nEllipse *>(widget);
		if (elli && elli->property("parentPan").toString()==panName) {
			elli->loadSettings(settings);
		}
	}
	
}

void
nGenericPan::saveUi(QSettings *settings) {
	foreach (QLineEdit *widget, findChildren<QLineEdit *>()) {
		if (widget->property("neutrinoSave").isValid() && widget->property("neutrinoSave").toBool()) {
			settings->setValue(widget->objectName(),widget->text());
		}
	}
	foreach (QPlainTextEdit *widget, findChildren<QPlainTextEdit *>()) {
		if (widget->property("neutrinoSave").isValid() && widget->property("neutrinoSave").toBool()) {
			settings->setValue(widget->objectName(),widget->toPlainText());
		}
	}
	foreach (QDoubleSpinBox *widget, findChildren<QDoubleSpinBox *>()) {
		if (widget->property("neutrinoSave").isValid() && widget->property("neutrinoSave").toBool()) settings->setValue(widget->objectName(),widget->value());
	}
	foreach (QSpinBox *widget, findChildren<QSpinBox *>()) {
		if (widget->property("neutrinoSave").isValid() && widget->property("neutrinoSave").toBool()) settings->setValue(widget->objectName(),widget->value());
	}
	foreach (QTabWidget *widget, findChildren<QTabWidget *>()) {
		if (widget->property("neutrinoSave").isValid() && widget->property("neutrinoSave").toBool()) settings->setValue(widget->objectName(),widget->currentIndex());
	}
	foreach (QCheckBox *widget, findChildren<QCheckBox *>()) {
		if (widget->property("neutrinoSave").isValid() && widget->property("neutrinoSave").toBool()) settings->setValue(widget->objectName(),widget->isChecked());
	}
	foreach (QRadioButton *widget, findChildren<QRadioButton *>()) {
		if (widget->property("neutrinoSave").isValid() && widget->property("neutrinoSave").toBool()) settings->setValue(widget->objectName(),widget->isChecked());
	}
	foreach (QComboBox *widget, findChildren<QComboBox *>()) {
		if (widget->property("neutrinoSave").isValid()) {
			if (widget->property("neutrinoSave").toBool()) {
				QStringList lista;
				for (int i=0; i< widget->count(); i++) {
					lista << widget->itemText(i);
				}
				settings->setValue(widget->objectName(),lista);
			}
			settings->setValue(widget->objectName()+"Default",widget->currentIndex());
		}
		
		if (widget->property("neutrinoImage").isValid() && widget->property("neutrinoImage").toBool()) {
			for (int i=0; i< widget->count(); i++) {
				nPhysD *phys=(nPhysD*) (widget->itemData(widget->currentIndex()).value<void*>());
				if (phys) {
					settings->setValue(widget->objectName(),QString::fromStdString(phys->getName()));
					settings->setValue(widget->objectName()+"-From",QString::fromStdString(phys->getFromName()));
				}
			}		
		}
	}

	foreach (QObject* widget, nparent->children()) {
		nLine *line=qobject_cast<nLine *>(widget);
		if (line && line->property("parentPan").toString()==panName) {
			line->saveSettings(settings);
		}
		nRect *rect=qobject_cast<nRect *>(widget);
		if (rect && rect->property("parentPan").toString()==panName) {
			rect->saveSettings(settings);
		}
		nEllipse *elli=qobject_cast<nEllipse *>(widget);
		if (elli && elli->property("parentPan").toString()==panName) {
			elli->saveSettings(settings);
		}
	}
}

void nGenericPan::closeEvent(QCloseEvent*){
	foreach (QComboBox *combo, findChildren<QComboBox *>()) {
		if (combo->property("neutrinoImage").isValid()) {			
			if (combo->property("neutrinoImage").toBool()) {
				disconnect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(comboChanged(int)));
			}
		}
	}
	saveDefaults();
// this might be useful sometimes
//	bool askAll=true;
//	foreach (QComboBox *obj, findChildren<QComboBox *>()) {
//		if (obj->property("neutrinoImage").isValid() && obj->property("neutrinoImage").toBool()) {
//			nPhysD *phys=(nPhysD*) (obj->itemData(obj->currentIndex()).value<void*>());
//			if (askAll && phys && phys->getType()!=PHYS_FILE) {
//				int res=QMessageBox::warning(this,tr("Attention"),
//											 tr("The image of combo ")+obj->objectName()+QString("\n")+QString::fromStdString(phys->getName())+QString("\n")+tr("has not been saved. Do you vant to save it now?"),
//											 QMessageBox::Yes | QMessageBox::No  | QMessageBox::NoToAll | QMessageBox::Cancel);
//				switch (res) {
//					case QMessageBox::Yes:
//						nparent->file_save_slot(phys); // TODO: add here a check for a cancel to avoid exiting
//						break;
//					case QMessageBox::NoToAll:
//						askAll=false;
//						break;
//					case QMessageBox::Cancel:
//						e->ignore();
//						return;
//						break;
//				}		
//			}
//			e->accept();
//		}
//	}
	foreach (QObject* widget, nparent->children()) {
		nLine *line=qobject_cast<nLine *>(widget);
		nRect *rect=qobject_cast<nRect *>(widget);
		nEllipse *elli=qobject_cast<nEllipse *>(widget);
		if (line && line->property("parentPan").toString()==panName) {
			line->my_pad.hide();
			line->deleteLater();
		}
		if (rect && rect->property("parentPan").toString()==panName) {
			rect->my_pad.hide();
			rect->deleteLater();
		}
		if (elli && elli->property("parentPan").toString()==panName) {
			elli->my_pad.hide();
			elli->deleteLater();
		}
	}
}

//////////////////// SETTINGS
void nGenericPan::loadSettings() {
	QString fnametmp = QFileDialog::getOpenFileName(this, tr("Open INI File"),property("fileIni").toString(), tr("INI Files (*.ini *.conf);; Any files (*.*)"));
	if (!fnametmp.isEmpty()) {
		setProperty("fileIni",fnametmp);
		loadSettings(fnametmp);
	}
}

void nGenericPan::loadSettings(QString settingsFile) {
	QSettings settings(settingsFile,QSettings::IniFormat);
	loadSettings(&settings);
}

void nGenericPan::saveSettings() {
	QString fnametmp = QFileDialog::getSaveFileName(this, tr("Save INI File"),property("fileIni").toString(), tr("INI Files (*.ini *.conf)"));
	if (!fnametmp.isEmpty()) {
		setProperty("fileIni",fnametmp);
		QSettings settings(fnametmp,QSettings::IniFormat);
		settings.clear();
		saveSettings(&settings);
	}
}

void nGenericPan::loadDefaults() {
	QSettings settings("neutrino","");
	settings.beginGroup(panName);
	loadSettings(&settings);
	settings.endGroup();
}

void nGenericPan::saveDefaults() {
	QSettings settings("neutrino","");
	settings.beginGroup(panName);
	saveSettings(&settings);
	settings.endGroup();
}

/// THESE are specialized
void nGenericPan::loadSettings(QSettings *settings) {
	loadUi(settings);
	int size = settings->beginReadArray("neutrinoProperties");
	for (int i = 0; i < size; ++i) {
		settings->setArrayIndex(i);
		QString prop=settings->value("property").toString();
		QString valu=settings->value("value").toString();
		setProperty(prop.toUtf8().constData(),valu);
	}
	settings->endArray();
}

void nGenericPan::saveSettings(QSettings *settings) {
	saveUi(settings);
	settings->beginWriteArray("neutrinoProperties");
	for (int i = 0; i < neutrinoProperties.size(); ++i) {
		settings->setArrayIndex(i);
		settings->setValue("property", neutrinoProperties.at(i));
		settings->setValue("value", property(neutrinoProperties.at(i).toUtf8().constData()).toString());
	}
	settings->endArray();
}

// thread run
//
void
nGenericPan::progressRun(int max_calc) {
	QProgressDialog progress(nThread.winTitle, "Stop", 0, max_calc, this);
	progress.setWindowModality(Qt::WindowModal);
	progress.show();
	nThread.start();
	while (nThread.isRunning()) {
		progress.setValue(nThread.n_iter);
		QApplication::processEvents();
		sleeper_thread::msleep(100);
		if (progress.wasCanceled()) nThread.stop();
	}

}

void nGenericPan::set(QString name, QVariant my_val, int occurrence) {
//	qDebug() << "HERE HERE HERE HERE HERE HERE HERE HERE HERE " << name << my_val;
	bool ok;
	int my_occurrence=1;
	foreach (QComboBox *obj, findChildren<QComboBox *>()) {
		if (obj->property("neutrinoImage").isValid()&&obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				bool found=false;
				for (int i=0;i<obj->count();i++) {
					nPhysD *objPhys=(nPhysD*) (obj->itemData(i).value<void *>());
					if (*objPhys == *(nPhysD*) (my_val.value<void *>())){
						obj->setCurrentIndex(i);
						found=true;
					}
				}
				if (!found) {
					nparent->addPhys((nPhysD*) (my_val.value<void *>()));
					QApplication::processEvents();
					if (obj->findData(my_val)>-1) {
						obj->setCurrentIndex(obj->findData(my_val));
						return;
					} else {
						if (obj->findText(my_val.toString())>-1) {
							obj->setCurrentIndex(obj->findText(my_val.toString()));
							QApplication::processEvents();
							return;
						}
					}
				}
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QComboBox *obj, findChildren<QComboBox *>()) {
		if (obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				//qDebug() << name << my_val;
				int val=my_val.toInt(&ok);
				if (ok && val>0 && val <= obj->maxVisibleItems()) {
					obj->setCurrentIndex(val-1);
				} else {
					int pos=obj->findData(my_val);
					if (pos>-1) {
						obj->setCurrentIndex(pos);
						return;
					}
				}
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QDoubleSpinBox *obj, findChildren<QDoubleSpinBox *>()) {
		if (obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				double val=my_val.toDouble(&ok);
				if (ok) {
					obj->setValue(val);
					return;
				}
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QSpinBox *obj, findChildren<QSpinBox *>()) {
		if (obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				int val=my_val.toInt(&ok);
				if (ok) {
					obj->setValue(val);
					return;
				}
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QLineEdit *obj, findChildren<QLineEdit *>()) {
		if (obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				obj->setText(my_val.toString());
				return;
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QCheckBox *obj, findChildren<QCheckBox *>()) {
		if (obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				obj->setChecked(my_val.toBool());
				return;
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QGroupBox *group, findChildren<QGroupBox *>()) {
		if (group->objectName()==name) {
			if (my_occurrence==occurrence) {
				foreach (QRadioButton *obj, group->findChildren<QRadioButton *>()) {
					obj->setChecked(obj->objectName()==my_val.toString());
				}
				return;
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QTabWidget *obj, findChildren<QTabWidget *>()) {
		if (obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				int val=my_val.toInt(&ok);
				if (ok) {
					if (val>0 && val <= obj->count()) {
						obj->setCurrentIndex(val-1);
						return;
					}
				} else {
					for (int i=0; i< obj->count();i++) {
						if (obj->tabText(i)==my_val.toString()) {
							obj->setCurrentIndex(i);
							return;
						}
					}
				}

			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QObject *obj, nparent->findChildren<QObject *>()) {
		nLine *linea=qobject_cast<nLine *>(obj);
		if (linea) {
			if (linea->property("parentPan").toString()==panName) {
				if (my_occurrence==occurrence) {
					QPolygonF poly;
					foreach (QVariant p, my_val.toList()) {
						poly << p.toPoint();
					}
					if (poly.size()>1) linea->setPoints(poly);
					return;
				}
				my_occurrence++;
			}
		}
	}
	my_occurrence=1;
	foreach (QObject *obj, nparent->findChildren<QObject *>()) {
		nRect *rect=qobject_cast<nRect *>(obj);
		if (rect) {
			if (rect->property("parentPan").toString()==panName) {
				if (my_occurrence==occurrence) {
					if (my_val.canConvert(QVariant::RectF)) {
						rect->setRect(my_val.toRectF());
						return;
					}
				}
				my_occurrence++;
			}
		}
	}
	my_occurrence=1;
	foreach (QObject *obj, nparent->findChildren<QObject *>()) {
		nEllipse *rect=qobject_cast<nEllipse *>(obj);
		if (rect) {
			if (rect->property("parentPan").toString()==panName) {
				if (my_occurrence==occurrence) {
					if (my_val.canConvert(QVariant::RectF)) {
						rect->setRect(my_val.toRectF());
						return;
					}
				}
				my_occurrence++;
			}
		}
	}
}

QVariant nGenericPan::get(QString name, int occurrence) {
	int my_occurrence=1;
	foreach (QComboBox *obj, findChildren<QComboBox *>()) {
		if (obj->property("neutrinoImage").isValid()&&obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				nPhysD *copyPhys=getPhysFromCombo(obj);
				return qVariantFromValue(*copyPhys);
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QComboBox *obj, findChildren<QComboBox *>()) {
		if (obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				return QVariant(obj->currentIndex()+1);
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QDoubleSpinBox *obj, findChildren<QDoubleSpinBox *>()) {
		if (obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				return QVariant(obj->value());
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QSpinBox *obj, findChildren<QSpinBox *>()) {
		if (obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				return QVariant(obj->value());
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QLineEdit *obj, findChildren<QLineEdit *>()) {
		if (obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				return QVariant(obj->text());
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QCheckBox *obj, findChildren<QCheckBox *>()) {
		if (obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				return QVariant(obj->isChecked());
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QGroupBox *group, findChildren<QGroupBox *>()) {
		if (group->objectName()==name) {
			if (my_occurrence==occurrence) {
				foreach (QRadioButton *obj, group->findChildren<QRadioButton *>()) {
					if (obj->isChecked()) return QVariant(obj->objectName());
				}
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QTabWidget *obj, findChildren<QTabWidget *>()) {
		if (obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				return QVariant(obj->currentIndex()+1);
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QObject *obj, nparent->findChildren<QObject *>()) {
		nLine *linea=qobject_cast<nLine *>(obj);
		if (linea) {
			if (linea->property("parentPan").toString()==panName) {
				if (my_occurrence==occurrence) {
					QVariantList variantList;
					foreach (QPointF p, linea->getPoints()) {
						variantList << p;
					}
					return QVariant(variantList);
				}
				my_occurrence++;
			}
		}
	}
	my_occurrence=1;
	foreach (QObject *obj, nparent->findChildren<QObject *>()) {
		nRect *rect=qobject_cast<nRect *>(obj);
		if (rect) {
			if (rect->property("parentPan").toString()==panName) {
				if (my_occurrence==occurrence) {
					return QVariant(rect->getRectF());
				}
				my_occurrence++;
			}
		}
	}
	my_occurrence=1;
	foreach (QObject *obj, nparent->findChildren<QObject *>()) {
		nEllipse *rect=qobject_cast<nEllipse *>(obj);
		if (rect) {
			if (rect->property("parentPan").toString()==panName) {
				if (my_occurrence==occurrence) {
					return QVariant(rect->getRectF());
				}
				my_occurrence++;
			}
		}
	}
	return QVariant();
}

nPhysD* nGenericPan::getPhys(QString name, int occurrence) {
	int my_occurrence=1;
	nPhysD *my_phys=NULL;
	foreach (QComboBox *obj, findChildren<QComboBox *>()) {
		if (obj->property("neutrinoImage").isValid()&&obj->objectName()==name) {
			my_phys=getPhysFromCombo(obj);
			if (my_occurrence==occurrence) return my_phys;
			my_occurrence++;
		}
	}
	return my_phys;
}

void nGenericPan::button(QString name , int occurrence) {
	QApplication::processEvents();
	int my_occurrence;
	my_occurrence=1;
	foreach (QPushButton *obj, findChildren<QPushButton *>()) {
		if (obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				obj->click();
			}
			my_occurrence++;
		}
	}
	my_occurrence=1;
	foreach (QAction *obj, findChildren<QAction *>()) {
		if (obj->objectName()==name) {
			if (my_occurrence==occurrence) {
				obj->trigger();
			}
			my_occurrence++;
		}
	}
}


