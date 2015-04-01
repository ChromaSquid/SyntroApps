#//
#//  Copyright (c) 2014 richards-tech.
#//
#//  This file is part of SyntroNet
#//
#//  SyntroNet is free software: you can redistribute it and/or modify
#//  it under the terms of the GNU General Public License as published by
#//  the Free Software Foundation, either version 3 of the License, or
#//  (at your option) any later version.
#//
#//  SyntroNet is distributed in the hope that it will be useful,
#//  but WITHOUT ANY WARRANTY; without even the implied warranty of
#//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#//  GNU General Public License for more details.
#//
#//  You should have received a copy of the GNU General Public License
#//  along with SyntroNet.  If not, see <http://www.gnu.org/licenses/>.
#//

HEADERS += Camera.h \
    base64.h \
    CameraClient.h \
    StreamsDlg.h \
    CameraDlg.h \
    MotionDlg.h \
    PanaCamIF.h \
    SyntroPanaCam.h \
    PanaCamConsole.h 
SOURCES += CameraClient.cpp \
    base64.cpp \
    StreamsDlg.cpp \
    CameraDlg.cpp \
    MotionDlg.cpp \
    main.cpp \
    PanaCamConsole.cpp \
    PanaCamIF.cpp \
    SyntroPanaCam.cpp
FORMS += SyntroPanaCam.ui
