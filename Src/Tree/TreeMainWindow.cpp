#include "TreePch.h"
#include "TreeMainWindow.h"
#include "TreeModel.h"
#include "SceneGraphModel.h"
#include "GlWindow.h"
#include "ViewOptions.h"
#include "NodeProperties.h"
#include "IkSolverProperties.h"
#include "AranIk.h"
#include "ArnIkSolver.h"
#include "Node.h"
#include "SimWorld.h"

TreeMainWindow::TreeMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_sg(0)
    , m_sgModel(0)
{
    m_glWindow = new GlWindow(this);
    m_glWindow->setMinimumSize(300, 300);


    createHorizontalGroupBox();
    createActions();

    m_sceneName = new QLabel("Scene name");
    m_sgTreeView = new QTreeView();
    m_sgModel = new SceneGraphModel(this);
    m_sgTreeView->setModel(m_sgModel);


    QDockWidget *viewOptionsDock = new QDockWidget("View Options", this);
    viewOptionsDock->setObjectName("view-options-dock");
    m_viewOptions = new ViewOptions(this, m_glWindow);
    viewOptionsDock->setWidget(m_viewOptions);
    addDockWidget(Qt::TopDockWidgetArea, viewOptionsDock);

    QDockWidget *nodePropDock = new QDockWidget("Node Properties", this);
    nodePropDock->setObjectName("node-properties-dock");
    m_nodeProp = new NodeProperties(this);
    nodePropDock->setWidget(m_nodeProp);
    addDockWidget(Qt::TopDockWidgetArea, nodePropDock);

    QDockWidget *ikSolverPropDock = new QDockWidget ("IK Solver Properties", this);
    ikSolverPropDock->setObjectName ("ik-solver-properties-dock");
    m_ikSolverProp = new IkSolverProperties (this);
    ikSolverPropDock->setWidget (m_ikSolverProp);
    addDockWidget(Qt::TopDockWidgetArea, ikSolverPropDock);

    m_refreshGl = new QPushButton("Refresh", this);
    m_refreshGl->setCheckable(true);
    m_refreshGl->setChecked(true);
    m_refreshGl->setStyleSheet("* { background: rgb(0,255,0); border: 3px solid white } *:!checked { background: rgb(255, 0, 0) }");
    connect(m_refreshGl, SIGNAL(clicked(bool)), m_glWindow, SLOT(setRefresh(bool)));



    QHBoxLayout *reconstructIkTreeLayout = new QHBoxLayout ();
    m_resetIkTreeRootButton = new QPushButton ("No IK node selected", this);
    m_resetIkTreeRootButton->setEnabled (false);
    m_reconstructIkLabel = new QLabel (this);
    reconstructIkTreeLayout->addWidget (new QLabel ("Change IK Root to", this));
    reconstructIkTreeLayout->addWidget (m_resetIkTreeRootButton);
    reconstructIkTreeLayout->addWidget (m_reconstructIkLabel);
    reconstructIkTreeLayout->addStretch ();

    QDockWidget *toolboxDock = new QDockWidget("Toolbox", this);
    toolboxDock->setObjectName("toolbox-dock");
    QVBoxLayout *toolboxLayout = new QVBoxLayout();
    toolboxLayout->setSpacing (2);
    toolboxLayout->setContentsMargins (2,2,2,2);
    toolboxLayout->addWidget(m_sceneName);
    toolboxLayout->addWidget(m_refreshGl);
    toolboxLayout->addWidget(m_horizontalGroupBox);
    toolboxLayout->addLayout(reconstructIkTreeLayout);
    toolboxLayout->addWidget(m_sgTreeView);
    QWidget* toolboxWidget = new QWidget();
    toolboxWidget->setLayout(toolboxLayout);
    toolboxDock->setWidget(toolboxWidget);
    addDockWidget(Qt::RightDockWidgetArea, toolboxDock);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(m_glWindow);

    QWidget* centerWidget = new QWidget();
    setCentralWidget(centerWidget);
    centerWidget->setLayout(topLayout);
    topLayout->setSpacing (2);
    topLayout->setContentsMargins (2,2,2,2);

    setWindowTitle(tr("ARAN Scene Graph Reader"));

    createMenus();
    (void)statusBar();

    setWindowFilePath(QString());

    connect (m_sgTreeView->selectionModel (), SIGNAL (currentChanged (QModelIndex,QModelIndex)), m_nodeProp, SLOT (nodeChanged (QModelIndex, QModelIndex)));
    //connect (m_sgTreeView->selectionModel (), SIGNAL (selectionChanged(QModelIndex,QModelIndex)), m_nodeProp, SLOT (nodeChanged (QModelIndex, QModelIndex)));
    connect (m_glWindow, SIGNAL(selectedChanged(const ArnNode *)), this, SLOT(selectedNodeChanged(const ArnNode *)));

    connect (m_glWindow, SIGNAL (activeNodeChanged(const ArnIkSolver*,const Node*)), this, SLOT (activeNodeChanged(const ArnIkSolver*,const Node*)));
    connect (m_glWindow, SIGNAL (ikSolversChanged(const std::vector<ArnIkSolver*>&)), m_ikSolverProp, SLOT(setIkSolvers(const std::vector<ArnIkSolver*>&)));
    connect (m_resetIkTreeRootButton, SIGNAL (clicked ()), this, SLOT (resetIkRoot ()));
}

void TreeMainWindow::activeNodeChanged (const ArnIkSolver *ikSolver, const Node *node)
{
    if (node)
    {
        m_resetIkTreeRootButton->setText (node->getName ());
        m_resetIkTreeRootButton->setEnabled (true);
    }
    else
    {
        m_resetIkTreeRootButton->setText ("No IK node selected");
        m_resetIkTreeRootButton->setEnabled (false);
        m_reconstructIkLabel->clear();
    }
}

void ExpandTreeRecursive (QTreeView *tv, const QModelIndex &idx)
{
    tv->expand (idx);
    if (idx.parent () != QModelIndex ())
        ExpandTreeRecursive (tv, idx.parent ());
}

void TreeMainWindow::selectedNodeChanged_Int (const ArnNode *selected, const QModelIndex &parent)
{
    const int rowCount = m_sgTreeView->model()->rowCount(parent);
    const int colCount = m_sgTreeView->model()->columnCount(parent);
    for (int i = 0; i < rowCount; ++i)
    {
        QModelIndex idx = m_sgTreeView->model()->index(i, 0, parent);
        if (reinterpret_cast <ArnNode *> (idx.internalPointer()) == selected)
        {
            m_sgTreeView->selectionModel()->clearSelection();
            for (int j = 0; j < colCount; ++j)
            {
                QModelIndex idx2 = m_sgTreeView->model()->index(i, j, parent);
                m_sgTreeView->selectionModel()->select(idx2, QItemSelectionModel::Select);
            }
            m_sgTreeView->scrollTo (idx, QAbstractItemView::PositionAtCenter);
            m_nodeProp->setNode (reinterpret_cast <ArnNode *> (idx.internalPointer()));
            ExpandTreeRecursive (m_sgTreeView, idx.parent());
            return;
        }
        selectedNodeChanged_Int (selected, idx);
    }
}

void TreeMainWindow::selectedNodeChanged(const ArnNode *selected)
{
    if (selected)
        selectedNodeChanged_Int(selected);
    else
    {
        m_sgTreeView->selectionModel()->clearSelection();
        m_nodeProp->setNode (0);
    }
}

void TreeMainWindow::createMenus()
{
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_openAct);
    m_separatorAct = m_fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i)
        m_fileMenu->addAction(m_recentFileActs[i]);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAct);
    updateRecentFileActions();

    menuBar()->addSeparator();

    m_windowMenu = menuBar()->addMenu(tr("&Window"));
    m_windowMenu->addAction(m_resetDockingStatesAct);

    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAct);
}

TreeMainWindow::~TreeMainWindow()
{
    delete m_sg;
    m_sg = 0;

    for (int i = 0; i < NumButtons; ++i)
        m_cameraButtons[i]->setUserData(0, 0);
}

void TreeMainWindow::createActions()
{
    m_openAct = new QAction(tr("&Open..."), this);
    m_openAct->setShortcuts(QKeySequence::Open);
    m_openAct->setStatusTip(tr("Open an existing file"));
    connect(m_openAct, SIGNAL(triggered()), this, SLOT(open()));

    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        m_recentFileActs[i] = new QAction(this);
        m_recentFileActs[i]->setVisible(false);
        connect(m_recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    m_exitAct = new QAction(tr("E&xit"), this);
    m_exitAct->setShortcuts(QKeySequence::Quit);
    m_exitAct->setStatusTip(tr("Exit the application"));
    connect(m_exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    m_resetDockingStatesAct = new QAction(tr("&Reset docking states"), this);
    m_resetDockingStatesAct->setStatusTip(tr("Reset all dock widget states (applied after restart)"));
    connect(m_resetDockingStatesAct, SIGNAL(triggered()), this, SLOT(resetDockingStates()));

    m_aboutAct = new QAction(tr("&About"), this);
    m_aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    for (int i = 0; i < NumButtons; ++i)
    {
        m_cameraActs[i] = new QAction(this);
        connect(m_cameraActs[i], SIGNAL(triggered()),
                m_glWindow, SLOT(setActiveCam()));
        connect(m_cameraButtons[i], SIGNAL(clicked()),
                m_glWindow, SLOT(setActiveCam()));
    }
}

void TreeMainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open ARAN XML"),
                                                    "/home/gbu", tr("XML Files (*.xml)"));
    if (!fileName.isEmpty())
        loadFile(fileName);
}

void TreeMainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        loadFile(action->data().toString());
}

void TreeMainWindow::about()
{
   QMessageBox::about(this, tr("About ARAN Scene Graph Reader"),
            tr("This program shows the content of an <b>ARAN</b> XML file."));
}

void TreeMainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Recent Files"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }
    file.close();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QByteArray ba = fileName.toAscii();
    const char* n = ba.constData();

    // Initialize a scene graph from given file name
    delete m_sg;
    m_sg = ArnSceneGraph::createFrom(n);
    if (!m_sg)
    {
        std::cerr << " *** Scene graph file " << n << " is not loaded correctly." << std::endl;
        std::cerr << "     Check your input XML scene file." << std::endl;
        return;
    }
    // Interconnect between nodes included in the scene graph. This is required for animation.
    m_sg->interconnect(m_sg);
    // Initialize renderable objects (textures, skinned meshes)
    ArnInitializeRenderableObjectsGl(m_sg);


    m_simWorld.reset (SimWorld::createFrom (m_sg));
    if (m_simWorld)
    {
        GeneralBodyPtr trunk = m_simWorld->getBodyByNameFromSet("Trunk");

        // Create ArnSkeleton from rigid body links!
        if (trunk)
        {
            ArnVec3 comPos;
            float bipedMass;
            trunk->calculateLumpedComAndMass(&comPos, &bipedMass);
            std::cout << " - Biped total mass: " << bipedMass << std::endl;

            ArnSkeleton* trunkSkel = trunk->createLumpedArnSkeleton(m_simWorld);
            trunkSkel->setName("Autogenerated Skeleton");
            m_sg->attachChildToFront(trunkSkel);
        }
    }

    std::cout << "   Scene file " << n << " loaded successfully." << std::endl;

    // Update tree view of the scene graph
    m_sgModel->setRootItem (m_sg);
    m_nodeProp->setNode (0);

    m_sceneName->setText(fileName);

    // Update camera buttons according to the cameras existing in the scene graph
    m_cameras.clear();
    extractCameras(m_sg);
    updateCameraButtons();

    // Notify GL window about the newly loaded scene graph
    m_glWindow->setSceneGraph(m_sg);

    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);

    setWindowTitle(fileName);
}

void TreeMainWindow::updateCameraButtons()
{
    for (int i = 0; i < NumButtons; ++i)
    {
        if (i < m_cameras.count())
        {
            m_cameraButtons[i]->setText(m_cameras.at(i)->getName());
            m_cameraButtons[i]->show();
            m_cameraButtons[i]->setUserData(0, reinterpret_cast<QObjectUserData *>(m_cameras.at(i)));
            m_cameraActs[i]->setData(qVariantFromValue(m_cameras.at(i)));
        }
        else
        {
            m_cameraButtons[i]->hide();
        }
    }
    if (m_cameras.isEmpty())
        m_cameraEmptyLabel->show();
    else
        m_cameraEmptyLabel->hide();
}

void TreeMainWindow::extractCameras(ArnNode *sg)
{
    if (sg)
    {
        foreach (ArnNode *n, sg->getChildren())
        {
            if (n->getType() == NDT_RT_CAMERA)
            {
                m_cameras.push_back(dynamic_cast<ArnCamera *>(n));
            }
            extractCameras(n);
        }
    }
}

void TreeMainWindow::setCurrentFile(const QString &fileName)
{
    m_curFile = fileName;
    setWindowFilePath(m_curFile);

    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        TreeMainWindow *mainWin = qobject_cast<TreeMainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
}

void TreeMainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
        m_recentFileActs[i]->setText(text);
        m_recentFileActs[i]->setData(files[i]);
        m_recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        m_recentFileActs[j]->setVisible(false);

    m_separatorAct->setVisible(numRecentFiles > 0);
}

QString TreeMainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void TreeMainWindow::createHorizontalGroupBox()
{
    m_horizontalGroupBox = new QGroupBox(tr("Available cameras"));
	QHBoxLayout *layout = new QHBoxLayout;

    for (int i = 0; i < NumButtons; ++i)
    {
        m_cameraButtons[i] = new QPushButton(tr("Button %1").arg(i + 1));
        m_cameraButtons[i]->hide();
        layout->addWidget(m_cameraButtons[i]);
	}
    m_cameraEmptyLabel = new QLabel("No availble camera exists.");
    layout->addWidget(m_cameraEmptyLabel);
    layout->addStretch ();
	m_horizontalGroupBox->setLayout(layout);
}

void TreeMainWindow::resetDockingStates()
{
    system("rm Tree.docklayout");
}

void TreeMainWindow::resetIkRoot ()
{
    Node *newNode = m_glWindow->resetIkRoot ();
    m_reconstructIkLabel->setText(tr("(current: %1)").arg (newNode->getName ()));
}
