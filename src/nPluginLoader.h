#include <QtGui>
#include <QMenu>

#include "nPanPlug.h"

#ifndef __nPluginLoader
#define __nPluginLoader

// holds plugin initialization (basic problem: nPlug is not a QObject

class nPluginLoader : public QPluginLoader {

	Q_OBJECT

public:
	nPluginLoader(QString, neutrino *);

    QString name() {return (my_panPlug ? my_panPlug->name() : QString("")); }

    bool ok() { return my_panPlug!=nullptr; }

    bool unload() {
        qDebug() << "killing me soflty" << my_panPlug;
        if (my_panPlug) {
            delete my_panPlug;
            my_panPlug=nullptr;
            nParent=nullptr;
        }
        return QPluginLoader::unload();
    }

    static QPointer<QMenu> getMenu(QString entryPoint, neutrino* neu);

public slots:
	
	void launch(void);

private:
    nPanPlug *my_panPlug;
    neutrino *nParent;
};

#endif
