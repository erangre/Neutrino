#include "nPhysD.h"
#include "nApp.h"
#include <cstring>
#include <algorithm>

nPhysD::nPhysD(physD *ref) : physD(*ref) {
	delete ref;
	DEBUG("------------------>>>>>>>>>>>>>" << getName());
	nApp::holder()->listPhys->registerPhys(this);
}

void nPhysD::TscanBrightness() {
	nPhysImageF<double>::TscanBrightness();
	emit physChanged(this);
}

double nPhysD::gamma() {
	if (!prop.have("gamma")) {
		prop["gamma"]=(int)1;
	}
	int gamma_int= prop["gamma"].get_i();
	return gamma_int < 1 ? -1.0/(gamma_int-2) : gamma_int;
}


const unsigned char* nPhysD::to_uchar_palette(std::vector<unsigned char>  &my_palette, std::string palette_name) {
	DEBUG("here " << getName() << " " << getW() << " " << getH());
	DEBUG("here " << palette_name);
	bidimvec<double> minmax=prop.have("display_range") ? prop["display_range"] : get_min_max();
	double mini=minmax.first();
	double maxi=minmax.second();

	if (getSurf()>0 && my_palette.size()==768) {

		if (uchar_buf.size() == getSurf()*3 &&
		        display_property.have("display_range") &&
		        display_property.have("palette_name") &&
		        display_property["palette_name"].get_str()==palette_name &&
		        display_property.have("gamma") &&
		        display_property["gamma"].get_i()==prop["gamma"].get_i()) {

			vec2f old_display_range=display_property["display_range"];
			vec2f new_display_range=prop["display_range"];

			if (old_display_range==new_display_range) {
				DEBUG("reusing old uchar_buf");
				return &uchar_buf[0];
			}
		}

		DEBUG(6,"8bit ["<<get_min()<<":"<<get_max() << "] from [" << mini << ":" << maxi<<"]");
		uchar_buf.resize(getSurf()*3);
		uchar_buf.assign(getSurf(),255);
#pragma omp parallel for
		for (size_t i=0; i<getSurf(); i++) {
			//int val = mult*(Timg_buffer[i]-lower_cut);
			double p=point(i);
			if (std::isfinite(p)) {
				unsigned char val = std::max(0,std::min(255,(int) (255.0*pow((p-mini)/(maxi-mini),gamma()))));
				std::copy ( my_palette.begin()+val*3, my_palette.begin()+val*3+3, uchar_buf.begin()+3*i);
			}
		}
		display_property["palette_name"]=palette_name;
		display_property["gamma"]=prop["gamma"].get_i();
		display_property["display_range"]=prop["display_range"];

		return &uchar_buf[0];
	}
	WARNING("asking for uchar buffer of empty image");

	return nullptr;
}

QByteArray nPhysD::to_ByteArray() {
	std::ostringstream oss;

	phys_dump_binary(this,oss);

	QByteArray retVal;
	retVal += QString::fromStdString(oss.str());

	return retVal;
}
