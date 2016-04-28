/*
 *
 *    Copyright (C) 2014 Alessandro Flacco, Tommaso Vinci All Rights Reserved
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

#include <iterator>

#include "nFocalSpot.h"
#include "neutrino.h"

nFocalSpot::nFocalSpot(neutrino *nparent, QString winname)
	: nGenericPan(nparent, winname)
{
	my_w.setupUi(this);
	nContour = new nLine(nparent);
	nContour->setParentPan(panName,3);
	
	decorate();
	connect(nparent, SIGNAL(bufferChanged(nPhysD*)), this, SLOT(calculate_stats()));
	connect(my_w.zero_dsb, SIGNAL(editingFinished()), this, SLOT(calculate_stats()));
	connect(my_w.check_dsb, SIGNAL(editingFinished()), this, SLOT(calculate_stats()));

	calculate_stats();
}


void
nFocalSpot::calculate_stats()
{
	nPhysD *cur = nparent->getBuffer(-1);
	if (!cur) 
		return;

	if (cur->property.count("nFocalSpotDynamic") > 0) {
		std::cerr<<"nFocal dynamic image --- skip"<<std::endl;
		return;
	}

	// 0. build decimated
	decimated = nPhysD(*cur);
	phys_fast_gaussian_blur(decimated, my_w.blur_radius_sb->value());
	decimated.TscanBrightness();

	// 1. find centroid
	vec2 centr;
	if (cur->get_origin() == vec2(0,0)) {
		centr = vec2(decimated.max_Tv_x, decimated.max_Tv_y);
		cur->set_origin(centr);
	} else
		centr = cur->get_origin();
	decimated.set_origin(centr);

	my_w.centroid_lbl->setText(QString("%1:%2").arg(centr.x()).arg(centr.y()));


	// 2. calculate integrals
	double c_value = cur->point(centr.x(),centr.y());
	double total_energy = cur->sum()-cur->getSurf()*my_w.zero_dsb->value();
	double above_th_energy = 0;
	int point_count = 0;
	double th = my_w.check_dsb->value()*(c_value-my_w.zero_dsb->value()) +my_w.zero_dsb->value() ;
    for (size_t ii=0; ii<cur->getSurf(); ii++)
		if (cur->point(ii) > th) {
			above_th_energy += cur->point(ii);
			point_count++;
		}

	above_th_energy -= point_count*my_w.zero_dsb->value();

	double energy_ratio;
	if (total_energy != 0)
		energy_ratio = 100*(above_th_energy/total_energy);
	else
		energy_ratio = 0;

	my_w.integral_lbl->setText(QString("Threshold integral %:\n%1").arg(energy_ratio));

	//std::cerr<<"min/max: "<<cur->get_min()<<"/"<<cur->get_max()<<", surf: "<<cur->getSurf()<<", point_count: "<<point_count<<std::endl;
	
	double c_integral = find_contour();
	
	//double contour_ratio = contour_integral();
	my_w.integral_lbl->setText(my_w.integral_lbl->text()+QString("\nContour integral %:\n%1\n(total: %2)").arg(100*c_integral/total_energy).arg(c_integral));

}

double
nFocalSpot::find_contour(void)
{
	
	nPhysD *cur = nparent->getBuffer(-1);
	if (!cur) 
		return -1;

	std::list<vec2> contour;
	contour_trace(decimated, contour, my_w.zero_dsb->value(), my_w.check_dsb->value());
	std::list<vec2>::iterator itr = contour.begin(), itr_last = contour.end();

	DEBUG(5, "got contour of "<<contour.size()<<" points (distance: "<<std::distance(itr, itr_last)<<")");


	nContour->setPoints(QPolygonF());
	if (contour.size() > 0) {

		// set polygon
		nContour->setPoints(QPolygonF());
		QPolygonF myp;
		for (itr = contour.begin(); itr != itr_last; ++itr) {
			myp<<QPointF((*itr).x(), (*itr).y());
			//std::cerr<<*itr<<std::endl;
		}

		// get stats
		vec2f c_center = cur->get_origin();
		vec2f c_scale = cur->get_scale();
		double min_r = vmath::td<double>(contour.front()-c_center, c_scale).mod();
		double max_r = min_r;
		for (itr = contour.begin(); itr != itr_last; ++itr) {
			double dd = vmath::td<double>((*itr)-c_center, c_scale).mod();
			if (dd > max_r) max_r = dd;
			if (dd < min_r) min_r = dd;
		}

		my_w.contour_lbl->setText(QString("min. radius: %1\nmax. radius: %2").arg(min_r).arg(max_r));

		nContour->setPoints(myp);
		my_w.statusBar->showMessage("Contour ok");

		double intg_contour = contour_integrate(*cur, contour);
		return intg_contour;
	}
    return -1;
}


void
nFocalSpot::bufferChanged(nPhysD *buf)
{
	nGenericPan::bufferChanged(buf);
	calculate_stats();
}
