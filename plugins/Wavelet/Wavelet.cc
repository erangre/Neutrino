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
#include "Wavelet.h"
#include "neutrino.h"

// physWavelets

Wavelet::Wavelet(neutrino *nparent) : nGenericPan(nparent)
{
	my_w.setupUi(this);

    region =  new nRect(this,1);
	region->setRect(QRectF(100,100,100,100));
	
    linebarrier =  new nLine(this,1);
	QPolygonF poly;
	poly << QPointF(0,0) << QPointF(100,100);
	linebarrier->setPoints(poly);
    show();

	connect(my_w.actionCarrier, SIGNAL(triggered()), this, SLOT(guessCarrier()));
	connect(my_w.actionDoAll, SIGNAL(triggered()), this, SLOT(doAll()));

	connect(my_w.actionRect, SIGNAL(triggered()), region, SLOT(togglePadella()));
	
	connect(my_w.doWavelet, SIGNAL(pressed()), this, SLOT(doWavelet()));
	connect(my_w.doUnwrap, SIGNAL(pressed()), this, SLOT(doUnwrap()));

	connect(my_w.weightCarrier, SIGNAL(valueChanged(double)), this, SLOT(guessCarrier()));

	connect(my_w.doRemove, SIGNAL(pressed()), this, SLOT(doRemove()));
	
	connect(my_w.useBarrier, SIGNAL(toggled(bool)), this, SLOT(useBarrierToggled(bool)));
	connect(my_w.lineBarrier, SIGNAL(released()), linebarrier, SLOT(togglePadella()));
	useBarrierToggled(my_w.useBarrier->isChecked());

	my_w.widthCarrierLabel->setText(QString::number(my_w.widthCarrier->value())+my_w.widthCarrier->suffix());
	my_w.angleCarrierLabel->setText(QString::number(my_w.angleCarrier->value())+my_w.angleCarrier->suffix());
	connect(my_w.widthCarrier, SIGNAL(valueChanged(double)), this, SLOT(doRemoveCarrier()));
	connect(my_w.angleCarrier, SIGNAL(valueChanged(double)), this, SLOT(doRemoveCarrier()));
	connect(my_w.weightCarrier, SIGNAL(valueChanged(double)), this, SLOT(doRemoveCarrier()));

    connect(this, SIGNAL(changeCombo(QComboBox *)), this, SLOT(checkChangeCombo(QComboBox *)));
	
	origSubmatrix=unwrapPhys=referencePhys=carrierPhys=syntheticPhys=NULL;
}

void Wavelet::on_relative_toggled(bool tog) {
    my_w.minStretch->setSuffix(tog?" X":" [px]");
    my_w.maxStretch->setSuffix(tog?" X":" [px]");
}

void Wavelet::useBarrierToggled(bool val) {
	if (val) {
		linebarrier->show();
	} else {
		linebarrier->hide();
	}
}

void Wavelet::checkChangeCombo(QComboBox *combo) {
	if (combo==my_w.image) {
		region->show();
	}
}

void Wavelet::bufferChanged(nPhysD* buf) {
    nGenericPan::bufferChanged(buf);
    if (buf) {
		if (buf==getPhysFromCombo(my_w.image)) {
			region->show();
		} else {
			region->hide();
		}
	}
}

void Wavelet::physDel(nPhysD* buf) {
    std::vector<nPhysD *> localPhys;
    localPhys.push_back(origSubmatrix);
    localPhys.push_back(unwrapPhys);
    localPhys.push_back(referencePhys);
    localPhys.push_back(carrierPhys);
    localPhys.push_back(syntheticPhys);
    for (std::vector<nPhysD *>::iterator itr=localPhys.begin(); itr!=localPhys.end(); itr++) {
        if (buf==*itr) {
            *itr=NULL;
        }
    }
}


void Wavelet::guessCarrier() {
	nPhysD *image=getPhysFromCombo(my_w.image);
	if (image) {
        QRect geom2=region->getRect(image);
		nPhysD datamatrix;
		datamatrix = image->sub(geom2.x(),geom2.y(),geom2.width(),geom2.height());

		vec2f vecCarr=phys_guess_carrier(datamatrix, my_w.weightCarrier->value());
		
		if (vecCarr.first()==0) {
			my_w.statusbar->showMessage(tr("ERROR: Problem finding the carrier"), 5000);
		} else {
			my_w.statusbar->showMessage(tr("Carrier: ")+QString::number(vecCarr.first())+"px "+QString::number(vecCarr.second())+"deg", 5000);
			disconnect(my_w.widthCarrier, SIGNAL(valueChanged(double)), this, SLOT(doRemoveCarrier()));
			disconnect(my_w.angleCarrier, SIGNAL(valueChanged(double)), this, SLOT(doRemoveCarrier()));
			disconnect(my_w.weightCarrier, SIGNAL(valueChanged(double)), this, SLOT(doRemoveCarrier()));
			my_w.widthCarrier->setValue(vecCarr.first());
			my_w.angleCarrier->setValue(vecCarr.second());
			my_w.widthCarrierLabel->setText(QString::number(my_w.widthCarrier->value())+my_w.widthCarrier->suffix());
			my_w.angleCarrierLabel->setText(QString::number(my_w.angleCarrier->value())+my_w.angleCarrier->suffix());
			connect(my_w.widthCarrier, SIGNAL(valueChanged(double)), this, SLOT(doRemoveCarrier()));
			connect(my_w.angleCarrier, SIGNAL(valueChanged(double)), this, SLOT(doRemoveCarrier()));
			connect(my_w.weightCarrier, SIGNAL(valueChanged(double)), this, SLOT(doRemoveCarrier()));
		}
	}
}

void Wavelet::doWavelet () {
    setEnabled(false);
	nPhysD *image=getPhysFromCombo(my_w.image);
	if (image) {
		QTime timer;
		timer.start();

		saveDefaults();
        QRect geom2=region->getRect(image);

		nPhysD datamatrix = image->sub(geom2.x(),geom2.y(),geom2.width(),geom2.height());		
		
		double conversionAngle=0.0;
		double conversionStretch=1.0;
		if (my_w.relative->isChecked()) {
			conversionAngle=my_w.angleCarrier->value();
			conversionStretch=my_w.widthCarrier->value();
		}

		//qCalculation_th my_qt;

		QSettings settings("neutrino","");
        settings.beginGroup("nPreferences");

		if (my_w.numAngle->value()==0) {
			my_params.init_angle=my_w.angleCarrier->value();
			my_params.end_angle=my_w.angleCarrier->value();
			my_params.n_angles=1;
		} else {
			my_params.init_angle=my_w.minAngle->value()+conversionAngle;
			my_params.end_angle=my_w.maxAngle->value()+conversionAngle;
			my_params.n_angles=my_w.numAngle->value();
		}
		if (my_w.numStretch->value()==0) {
			my_params.init_lambda=my_w.widthCarrier->value();
			my_params.end_lambda=my_w.widthCarrier->value();
			my_params.n_lambdas=1;
		} else {
			my_params.init_lambda=my_w.minStretch->value()*conversionStretch;
			my_params.end_lambda=my_w.maxStretch->value()*conversionStretch;
			my_params.n_lambdas=my_w.numStretch->value();
		}
		my_params.thickness=my_w.thickness->value();
		my_params.damp=my_w.damp->value();
        my_params.data=&datamatrix;

        QString out;

        qDebug() << openclEnabled() << settings.value("openclUnit").toInt();

        if (openclEnabled()>0 && settings.value("openclUnit").toInt()>0) {
            out="OpenCL: ";
            my_params.opencl_unit=settings.value("openclUnit").toInt();
            runThread(&my_params, phys_wavelet_trasl_opencl, "OpenCL wavelet", my_params.n_angles*my_params.n_lambdas);
        } else {
            out="CPU: ";
            runThread(&my_params, phys_wavelet_trasl_cpu, "CPU wavelet", my_params.n_angles*my_params.n_lambdas);
        }


		my_w.erasePrevious->setEnabled(true);
        std::map<std::string, nPhysD *>::const_iterator itr;
		for(itr = my_params.olist.begin(); itr != my_params.olist.end(); ++itr) {
            if ((itr->first=="angle"  && my_params.n_angles==1) ||
                (itr->first=="lambda" && my_params.n_lambdas==1)) {
                delete itr->second;
            } else {
                if (my_w.erasePrevious->isChecked()) {
                    waveletPhys[itr->first]=nparent->replacePhys(itr->second,waveletPhys[itr->first],false);
                } else {
                    nparent->addPhys(itr->second);
                    waveletPhys[itr->first]=itr->second;
                }
            }
		}
        QApplication::processEvents();
        
		for(itr = waveletPhys.begin(); itr != waveletPhys.end(); ++itr) {
            if (itr->first=="phase_2pi") {
                my_w.imageUnwrap->setCurrentIndex(my_w.imageUnwrap->findData(qVariantFromValue((void*)(itr->second))));
            } else if (itr->first=="contrast") {
                my_w.qualityUnwrap->setCurrentIndex(my_w.qualityUnwrap->findData(qVariantFromValue((void*)(itr->second))));
            }
        }
        
		if (my_w.showSource->isChecked()) {
			datamatrix.setShortName("wavelet source");
			nPhysD *deepcopy=new nPhysD();
			*deepcopy=datamatrix;
			if (my_w.erasePrevious->isChecked()) {
				origSubmatrix=nparent->replacePhys(deepcopy,origSubmatrix,false);
			} else {
				nparent->addPhys(deepcopy);
				origSubmatrix=deepcopy;
			}
		}
        if (my_w.synthetic->isChecked()) {
            nPhysD *tmpSynthetic = new nPhysD();
            phys_synthetic_interferogram(*tmpSynthetic,waveletPhys["phase_2pi"],waveletPhys["contrast"]);
            if (my_w.erasePrevious->isChecked()) {
                syntheticPhys=nparent->replacePhys(tmpSynthetic,syntheticPhys,false);
            } else {
                nparent->addPhys(tmpSynthetic);
                syntheticPhys=tmpSynthetic;
            }
        }
        
        QString status_bar_measure;
        status_bar_measure.sprintf("%.2f sec, %.2f Mpx/s",1.0e-3*timer.elapsed(), 1.0e-3*my_params.n_angles*my_params.n_lambdas*geom2.width()*geom2.height()/timer.elapsed());
        my_w.statusbar->showMessage(out+status_bar_measure, 50000);
        DEBUG(status_bar_measure.toStdString());
    } else {
        my_w.statusbar->showMessage("Canceled", 5000);
    }
    QApplication::processEvents();
    setEnabled(true);
}


// --------------------------------------------------------------------------

void Wavelet::doUnwrap () {
	nPhysD *phase=getPhysFromCombo(my_w.imageUnwrap);
	nPhysD *qual=getPhysFromCombo(my_w.qualityUnwrap);
	nPhysD barrierPhys;
	
	QTime timer;
	timer.start();

	if (qual && phase) {
		nPhysD *uphase=NULL;

		QString methodName=my_w.method->currentText();

		if (my_w.useBarrier->isChecked()) {
			barrierPhys = nPhysD(phase->getW(),phase->getH(),1.0,"barrier");
            QPolygonF my_poly=linebarrier->poly(phase->getW()+phase->getH());
            my_poly.translate(qual->get_origin().x(),qual->get_origin().y());
			foreach(QPointF p, my_poly) {
				barrierPhys.set(p.x()-1,p.y()-1,0.0);
				barrierPhys.set(p.x()-1,p.y()  ,0.0);
				barrierPhys.set(p.x()-1,p.y()+1,0.0);
				barrierPhys.set(p.x()  ,p.y()-1,0.0);
				barrierPhys.set(p.x()  ,p.y()  ,0.0);
				barrierPhys.set(p.x()  ,p.y()+1,0.0);
				barrierPhys.set(p.x()+1,p.y()-1,0.0);
				barrierPhys.set(p.x()+1,p.y()  ,0.0);
				barrierPhys.set(p.x()+1,p.y()+1,0.0);
			}
			if (methodName=="Simple H+V") {
				uphase = phys_phase_unwrap(*phase, barrierPhys, SIMPLE_HV);
			} else if (methodName=="Simple V+H") {
				uphase = phys_phase_unwrap(*phase, barrierPhys, SIMPLE_VH);
			} else if (methodName=="Goldstein") {
				uphase = phys_phase_unwrap(*phase, barrierPhys, GOLDSTEIN);
			} else if (methodName=="Miguel") {
				uphase = phys_phase_unwrap(*phase, barrierPhys, MIGUEL_QUALITY);
			} else if (methodName=="Miguel+Quality") {
				phys_point_multiply(barrierPhys,*qual);
				uphase = phys_phase_unwrap(*phase, barrierPhys, MIGUEL_QUALITY);
			} else if (methodName=="Quality") {
				phys_point_multiply(barrierPhys,*qual);
				uphase = phys_phase_unwrap(*phase, barrierPhys, QUALITY);
			}
		} else {
			if (methodName=="Simple H+V") {
				uphase = phys_phase_unwrap(*phase, *qual, SIMPLE_HV);
			} else if (methodName=="Simple V+H") {
				uphase = phys_phase_unwrap(*phase, *qual, SIMPLE_VH);
			} else if (methodName=="Goldstein") {
				uphase = phys_phase_unwrap(*phase, *qual, GOLDSTEIN);
			} else if (methodName=="Miguel") {
				uphase = phys_phase_unwrap(*phase, *qual, MIGUEL);
			} else if (methodName=="Miguel+Quality") {
				uphase = phys_phase_unwrap(*phase, *qual, MIGUEL_QUALITY);
			} else if (methodName=="Quality") {
				uphase = phys_phase_unwrap(*phase, *qual, QUALITY);
			}
		}
		
		// esistono sicuramente dei metodi piu' intelligenti
		
		if (uphase) {
			uphase->setShortName("unwrap");
			uphase->setName(uphase->getShortName()+"-"+methodName.toStdString()+" "+QFileInfo(QString::fromUtf8(phase->getFromName().c_str())).fileName().toStdString());
			uphase->setFromName(phase->getFromName());
			my_w.erasePreviousUnwrap->setEnabled(true);

			if (my_w.removeCarrierAfterUnwrap->isChecked()) {
                double alpha=my_w.angleCarrier->value();
                double lambda=my_w.widthCarrier->value();
                double kx = cos(alpha*_phys_deg)/lambda;
                double ky = -sin(alpha*_phys_deg)/lambda;
                phys_subtract_carrier(*uphase, kx, ky);
			}
			
			if (my_w.erasePreviousUnwrap->isChecked()) {
				unwrapPhys=nparent->replacePhys(uphase,unwrapPhys);
			} else {
				unwrapPhys=uphase;
				nparent->addShowPhys(unwrapPhys);
			}
			my_w.unwrapped->setCurrentIndex(my_w.unwrapped->findData(qVariantFromValue((void*)unwrapPhys)));
			
		}
	}
	QString out;
	out.sprintf("%d msec",timer.elapsed());
	my_w.statusbar->showMessage(out);
}

void Wavelet::doAll () {
	doWavelet();
	doUnwrap();
}

void Wavelet::doRemove () {
	if (my_w.carrier->isChecked()) {
		doRemoveCarrier();
	}
	if (my_w.reference->isChecked()) {
		doRemoveReference();
	}
}

void Wavelet::doRemoveCarrier () {
	my_w.widthCarrierLabel->setText(QString::number(my_w.widthCarrier->value())+my_w.widthCarrier->suffix());
	my_w.angleCarrierLabel->setText(QString::number(my_w.angleCarrier->value())+my_w.angleCarrier->suffix());
	if (sender() && (sender()==my_w.widthCarrier || sender()==my_w.angleCarrier || sender()==my_w.weightCarrier) && (!my_w.liveView->isChecked())) {
		return;
	}
	my_w.carrier->setChecked(true);
	// check normalize
	nPhysD *unwrapped=getPhysFromCombo(my_w.unwrapped);
	if (unwrapped) {
		nPhysD *unwrappedSubtracted;
		unwrappedSubtracted = new nPhysD(*unwrapped);

		double alpha=my_w.angleCarrier->value();
		double lambda=my_w.widthCarrier->value();

		unwrappedSubtracted->setName("No carrier ("+QString::number(lambda).toStdString()+","+
																 QString::number(alpha).toStdString()+") "+
																 QFileInfo(QString::fromUtf8(unwrapped->getFromName().c_str())).fileName().toStdString());
		unwrappedSubtracted->setShortName("No carrier");
		unwrappedSubtracted->setFromName(unwrapped->getFromName());

        double kx = cos(alpha*_phys_deg)/lambda;
        double ky = -sin(alpha*_phys_deg)/lambda;
        phys_subtract_carrier(*unwrappedSubtracted, kx, ky);
		phys_subtract(*unwrappedSubtracted, my_w.phaseOffset->value());
		my_w.erasePreviuos->setEnabled(true);
		if (my_w.erasePreviuos->isChecked()) {
			carrierPhys=nparent->replacePhys(unwrappedSubtracted,carrierPhys);
		} else {
			nparent->addShowPhys(unwrappedSubtracted);
		}
	}
}

void Wavelet::doRemoveReference () {
	// check normalize
	nPhysD *ref=getPhysFromCombo(my_w.refUnwrap);
	nPhysD *unwrapped=getPhysFromCombo(my_w.unwrapped);
	if (unwrapped && ref) {
		if (unwrapped->getW() == ref->getW() && unwrapped->getH() == ref->getH()) {
			nPhysD *unwrappedSubtracted;
			unwrappedSubtracted = new nPhysD();
			*unwrappedSubtracted=(*unwrapped)-(*ref);
			phys_subtract(*unwrappedSubtracted, my_w.phaseOffset->value());
			unwrappedSubtracted->setName(unwrapped->getName()+" - "+ref->getName());
			unwrappedSubtracted->setName("Reference removed : "+ref->getName());
			unwrappedSubtracted->setShortName("Ref removed");
			if (unwrapped->getFromName()==ref->getFromName()) {
				unwrappedSubtracted->setFromName(unwrapped->getFromName());
			} else {
				unwrappedSubtracted->setFromName(unwrapped->getFromName()+" "+ref->getFromName());
			}
			my_w.erasePreviuos->setEnabled(true);
			if (my_w.erasePreviuos->isChecked()) {
				referencePhys=nparent->replacePhys(unwrappedSubtracted,referencePhys);
			} else {
				nparent->addShowPhys(unwrappedSubtracted);
			}
		}
	}
}
