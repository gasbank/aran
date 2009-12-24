#ifndef RUCB_PCH_H_INCLUDED
#define RUCB_PCH_H_INCLUDED

// INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#ifdef WIN32
    #include <memory>
#else
    #include <tr1/memory>
#endif

#ifndef WIN32
	#include <sys/time.h>
#endif

//
// ODE
//
#include <ode/ode.h>

//
// Qt Library
//
#include <QDesktopWidget>
#include <QApplication>
#include <QMainWindow>
#include <QGLWidget>
#include <QThread>
#include <QtDebug>
#include <QMap>
#include <QObject>
#include <QtOpenGL>
#include <QString>
#include <QStringList>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDockWidget>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QStatusBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>

//
// ARAN Library
//
#include "AranApi.h"
#include "VideoManGl.h"
#include "AranGl.h"
#include "ArnTextureGl.h"
#include "ArnGlExt.h"
#include "AranPhy.h"
#include "ArnPhyBox.h"
#include "SimWorld.h"
#include "GeneralJoint.h"
#include "AranIk.h"
#include "ArnIkSolver.h"
#include "ArnPlane.h"
#include "ArnBone.h"

#include "Tree.h"
#include "Jacobian.h"
#include "Node.h"


// The following macro is required to use qVariantFromValue(<type>).
Q_DECLARE_METATYPE( ArnCamera * )
Q_DECLARE_METATYPE( ArnIkSolver * )
Q_DECLARE_METATYPE( const ArnIkSolver * )

#endif // RUCB_PCH_H_INCLUDED
