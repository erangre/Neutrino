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

QByteArray nPhysD::to_ByteArray() {
	std::ostringstream oss;

	phys_dump_binary(this,oss);

	QByteArray retVal;
	retVal += QString::fromStdString(oss.str());

	return retVal;
}
