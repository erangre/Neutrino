#ifndef __nPhysD
#define __nPhysD

#include "nPhysImageF.h"
#include "nPhysFormats.h"
#include <QtCore>
#include <list>

class nPhysD : public QObject, public physD  {
    Q_OBJECT

public:
	nPhysD(physD *ref);

	void TscanBrightness();

    double gamma();

	QByteArray to_ByteArray();

signals:
    void physChanged(nPhysD*);

};

Q_DECLARE_METATYPE(physD*);

#endif
