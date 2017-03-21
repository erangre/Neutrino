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
#include <string>
#include <iostream>
#include <cmath>
#include <signal.h>

#include <QtGui>

#include "nApp.h"
#include "holderGUI.h"

#include <QTranslator>

void my_handler(int s){
    printf("Caught signal %d\n",s);
    QCoreApplication::quit();
}

int main(int argc, char **argv)
{
#ifndef __WIN32
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
#endif

    
    qSetMessagePattern("%{function}:%{line} : %{message}");

    nApp my_app(argc,argv);

    QStringList args=my_app.arguments();
    args.removeFirst();
    
    holderGUI *my_gui= new holderGUI();
	my_gui->openFiles(args);

    return my_app.exec();

}