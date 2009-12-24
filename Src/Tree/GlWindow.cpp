#include "TreePch.h"
#include "GlWindow.h"
#include "Node.h"
#include "VideoMan.h"
#include "Tree.h"

struct HitRecord
{
    GLuint numNames;	// Number of names in the name stack for this hit record
    GLuint minDepth;	// Minimum depth value of primitives (range 0 to 2^32-1)
    GLuint maxDepth;	// Maximum depth value of primitives (range 0 to 2^32-1)
    GLuint contents;	// Name stack contents
};

GlWindow::GlWindow(QWidget *parent)
    : QGLWidget(parent)
    , m_sg(0)
    , m_activeCam(0)
    , m_activeLight(0)
    , m_bRefresh(true)
    , m_bDrawJointIndicator(false)
    , m_bDrawEndeffectorIndicator(false)
    , m_bDrawJointAxisIndicator(false)
    , m_bDrawRootNodeIndicator(false)
    , m_bDrawGrid(false)
    , m_xRot(0)
    , m_yRot(0)
    , m_zRot(0)
    , m_activeIkSolver(0)
    , m_activeNode(0)
    , m_viewMode(VM_UNKNOWN)
{
    m_refreshTimer = startTimer(10);

    avd.X		= 0;
    avd.Y		= 0;
    avd.Width	= width();
    avd.Height	= height();
    avd.MinZ	= 0;
    avd.MaxZ	= 1.0f;
}

void GlWindow::initializeGL()
{
    if (ArnInitGlExtFunctions() < 0)
    {
        std::cerr << " *** OpenGL extensions needed to run this program are not available." << std::endl;
        std::cerr << "     Check whether you are in the remote control display or have a legacy graphics adapter." << std::endl;
    }
    if (ArnInitializeGl() < 0)
    {
        std::cerr << " *** OpenGL context for ARAN library cannot be initialized." << std::endl;
    }

    glClearColor(0,0,0,1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
}

void GlWindow::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    avd.Width	= width;
    avd.Height	= height;
}

static void RenderGrid(ViewMode vm, const float gridCellSize, const int gridCellCount, const float gridColor[3], const float thickness)
{
    glColor3fv(gridColor);
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);

    // subgrid
    glLineWidth(thickness);
    glBegin(GL_LINES);
    const float v1 = gridCellCount * gridCellSize;
    for (int i = -gridCellCount; i <= gridCellCount; ++i)
    {
        const float v2 = gridCellSize * i;

        switch (vm)
        {
        case VM_UNKNOWN:
        case VM_CAMERA:
        case VM_TOP:
            // X direction
            glVertex3f(-v1, v2, 0);
            glVertex3f( v1, v2, 0);
            // Y direction
            glVertex3f(v2, -v1, 0);
            glVertex3f(v2,  v1, 0);
            break;
        case VM_RIGHT:
            // Y direction
            glVertex3f(0, -v1, v2);
            glVertex3f(0,  v1, v2);
            // Z direction
            glVertex3f(0, v2, -v1);
            glVertex3f(0, v2,  v1);
            break;
        case VM_BACK:
            // X direction
            glVertex3f(-v1, 0, v2);
            glVertex3f( v1, 0, v2);
            // Z direction
            glVertex3f(v2, 0, -v1);
            glVertex3f(v2, 0,  v1);
            break;
        default:
            break;
        }
    }
    glEnd();

    glPopAttrib();
}

void GlWindow::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (m_activeCam)
    {
        ArnConfigureViewportProjectionMatrixGl(&avd, m_activeCam); // Projection matrix is not changed during runtime for now.

        const ArnQuat dQuat = ArnQuat::createFromEuler(ArnToRadian(m_xRot / 10.0),
                                                   ArnToRadian(m_yRot / 10.0),
                                                   ArnToRadian(m_zRot / 10.0));
        ArnConfigureViewMatrixGl2(m_activeCam, &ArnConsts::ARNVEC3_ZERO, &dQuat);
    }

    if (m_activeLight)
    {
        ArnConfigureLightGl(0, m_activeLight);
    }

    if (m_bDrawGrid)
    {
        const static float gridColor[3] = { 0.4f, 0.4f, 0.4f };
        RenderGrid(m_viewMode, 0.5f, 10, gridColor, 0.5f);
        RenderGrid(m_viewMode, 2.5f, 2, gridColor, 1.0f);
    }

    // Render skeletons under control of IK solver
    foreach (ArnIkSolver *ikSolver, m_ikSolvers)
    {
        glPushMatrix();
        TreeDraw(*ikSolver->getTree(),
                 m_bDrawJointIndicator,
                 m_bDrawEndeffectorIndicator,
                 m_bDrawJointAxisIndicator,
                 m_bDrawRootNodeIndicator);
        glPopMatrix();
    }
    if (m_sg)
    {
        ArnSceneGraphRenderGl(dynamic_cast<const ArnSceneGraph *>(m_sg), true);
    }
}

void GlWindow::setSceneGraph (ArnSceneGraph *sg)
{
    Q_ASSERT(sg->getType() == NDT_RT_SCENEGRAPH);
    m_sg = sg;
    m_activeCam = reinterpret_cast<ArnCamera *>(m_sg->findFirstNodeOfType(NDT_RT_CAMERA));
    m_activeLight = reinterpret_cast<ArnLight *>(m_sg->findFirstNodeOfType(NDT_RT_LIGHT));

    ArnSkeleton *skel = reinterpret_cast<ArnSkeleton *>(m_sg->findFirstNodeOfType(NDT_RT_SKELETON));
    if (skel && skel->getAnimCtrl() && skel->getAnimCtrl()->getTrackCount() > 0)
    {
        skel->getAnimCtrl()->SetTrackAnimationSet(0, 0);
        skel->getAnimCtrl()->SetTrackPosition(0, skel->getAnimCtrl()->GetTime());
        ARNTRACK_DESC desc;
        skel->getAnimCtrl()->GetTrackDesc(0, &desc);
        skel->getAnimCtrl()->SetTrackEnable(0, desc.Enable ? false : true);
        skel->getAnimCtrl()->SetTrackWeight(0, 1);
    }

    foreach (ArnIkSolver *ikSolver, m_ikSolvers)
    {
        delete ikSolver;
    }
    m_ikSolvers.clear();
    ArnCreateArnIkSolversOnSceneGraph(m_ikSolvers, m_sg);

    m_xRot = 0;
    m_yRot = 0;
    m_zRot = 0;

    m_activeIkSolver = 0;
    m_activeNode = 0;

    emit activeNodeChanged (0, 0);
    emit ikSolversChanged (m_ikSolvers);

    updateGL();
}

void GlWindow::setActiveCam()
{
    std::cout << "setActiveCam()" << std::endl;
    QPushButton *action = qobject_cast<QPushButton *>(sender());
    if (action)
    {
        ArnCamera *cam = reinterpret_cast<ArnCamera *>(action->userData(0));
        Q_ASSERT(cam->getType() == NDT_RT_CAMERA);
        setActiveCam(cam);
    }
}

void GlWindow::setActiveCam(ArnCamera *cam)
{
    m_activeCam = cam;
    m_xRot = 0;
    m_yRot = 0;
    m_zRot = 0;
    updateGL();
}


void GlWindow::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == m_refreshTimer)
    {
        if (m_bRefresh)
        {
            if (m_sg)
                m_sg->update(0, 0.01);
            updateGL();
        }
    }
}


void GlWindow::setRefresh(bool refresh)
{
    m_bRefresh = refresh;
}

bool BuffComp (const HitRecord &h1, const HitRecord &h2)
{
    return h1.minDepth < h2.minDepth;
}

void GlWindow::selectGraphicObject (const float mousePx, const float mousePy)
{
    if (!m_sg)
        return;

    std::vector <HitRecord> buff (16);
    GLint hits, view[4];

    /* This choose the buffer where store the values for the selection data */
    glSelectBuffer (4*16, reinterpret_cast<GLuint*>(&buff[0]));

    /* This retrieve info about the viewport */
    glGetIntegerv (GL_VIEWPORT, view);

    /* Switching in selecton mode */
    glRenderMode (GL_SELECT);

    /* Clearing the name's stack. This stack contains all the info about the objects */
    glInitNames ();

    /* Now fill the stack with one element (or glLoadName will generate an error) */
    glPushName (0);

    /* Now modify the vieving volume, restricting selection area around the cursor */
    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    float origProjMat[16];
    glGetFloatv (GL_PROJECTION_MATRIX, origProjMat);
    glLoadIdentity ();

    /* restrict the draw to an area around the cursor */
    const GLdouble pickingAroundFactor = 1.0;
    gluPickMatrix (mousePx, mousePy, pickingAroundFactor, pickingAroundFactor, view);

    /* your original projection matrix */
    glMultMatrixf (origProjMat);

    /* Draw the objects onto the screen */
    glMatrixMode (GL_MODELVIEW);
    /* draw only the names in the stack, and fill the array */
    glFlush ();

    // Rendering routine START
    ArnSceneGraphRenderGl (static_cast<ArnSceneGraph *> (m_sg), true);
    foreach (ArnIkSolver* ikSolver, m_ikSolvers)
    {
        TreeDraw (*ikSolver->getTree(), m_bDrawJointIndicator, m_bDrawEndeffectorIndicator, m_bDrawJointAxisIndicator, m_bDrawRootNodeIndicator);
    }
    // Rendering routine END

    /* Do you remeber? We do pushMatrix in PROJECTION mode */
    glMatrixMode (GL_PROJECTION);
    glPopMatrix ();

    /* get number of objects drawed in that area and return to render mode */
    hits = glRenderMode (GL_RENDER);

    glMatrixMode (GL_MODELVIEW);

    std::sort (buff.begin (), buff.begin () + hits, BuffComp);

    /* Print a list of the objects */
    std::cout << "---------------------" << std::endl;
    for (GLint h = 0; h < hits; ++h)
    {
        if (buff[h].contents) // Zero means that ray hit on bounding box area.
        {
            const ArnNode *node = m_sg->getConstNodeById (buff[h].contents);
            if (node)
            {
                const ArnNode *parentNode = node->getParent ();
                const char *name = node->getName ();
                if (strlen (name) == 0)
                    name = "<Unnamed>";
                if (parentNode)
                {
                    const char *parentName = parentNode->getName ();
                    if (strlen (parentName) == 0)
                        parentName = "<Unnamed>";

                    /*
                    printf("[Object 0x%p ID %d %s (Parent Object 0x%p ID %d %s)]\n",
                        node, node->getObjectId(), name, parentNode, parentNode->getObjectId(), parentName);
                    */
                    std::cout << "[Object ";
                    std::cout << std::setw(8) << std::setfill('0') << node;
                    std::cout << " ID " << std::dec << node->getObjectId() << " " << name;
                    std::cout << " (Parent Object ";
                    std::cout << std::hex << std::setw(8) << std::setfill('0') << parentNode;
                    std::cout << " ID " << std::dec << parentNode->getObjectId() << " " << parentName;
                    std::cout << ")] " << buff[h].minDepth << " " << buff[h].maxDepth << std::endl;
                }
                else
                {
                    /*
                    printf("[Object 0x%p ID %d %s]\n",
                        node, node->getObjectId(), name);
                    */
                    std::cout << "[Object 0x" << std::hex << std::setw(8) << std::setfill('0') << node;
                    std::cout << " ID " << node->getObjectId() << " " << name;
                    std::cout << "]" << std::endl;
                }

                const ArnMesh* mesh = dynamic_cast<const ArnMesh*>(parentNode);
                if (mesh)
                {
                    ArnVec3 dim;
                    mesh->getBoundingBoxDimension(&dim, true);
                    printf("Mesh Dimension: "); dim.printFormatString();
                }
            }


            foreach (ArnIkSolver* ikSolver, m_ikSolvers)
            {
                std::cout << "contents: " << std::dec << buff[h].contents << std::endl;
                Node* node = ikSolver->getNodeByObjectId(buff[h].contents);
                if (node)
                {
                    assert (node->getObjectId () == buff[h].contents);

                    //printf("[Object 0x%p ID %d %s] Endeffector=%d\n",
                    //    node, node->getObjectId(), node->getName(), node->isEndeffector());

                    std::cout << "[Object " << std::hex << node;
                    std::cout << " ID " << std::dec << node->getObjectId() << " " << node->getName();
                    std::cout << " Endeffector=" << node->isEndeffector() << std::endl;

                    m_activeIkSolver = ikSolver;
                    m_activeNode = node;

                    emit activeNodeChanged (ikSolver, node);

                    if (node->isEndeffector())
                    {
                        ikSolver->setSelectedEndeffector(node);
                    }
                }
            }
        }
    }

    if (hits)
    {
        if (buff[0].contents) // Zero means that ray hit on bounding box area.
        {
            const ArnNode *node = m_sg->getConstNodeById (buff[0].contents);
            if (node)
            {
                emit selectedChanged (node);
            }
        }
    }
    else
    {
        emit selectedChanged (0);
    }
}

void GlWindow::mousePressEvent (QMouseEvent * event)
{
    m_lastPos = event->pos ();

    selectGraphicObject(float(event->x()),
                        float(height() - event->y()) // Note that Y-coord flipped.
                        );

    if (m_sg)
    {
        ArnMatrix modelview, projection;
        glGetFloatv(GL_MODELVIEW_MATRIX, reinterpret_cast<GLfloat*>(modelview.m));
        modelview = modelview.transpose();
        glGetFloatv(GL_PROJECTION_MATRIX, reinterpret_cast<GLfloat*>(projection.m));
        projection = projection.transpose();
        ArnVec3 origin, direction;
        ArnMakePickRay(&origin, &direction, float(event->x()), float(height() - event->y()), &modelview, &projection, &avd);
        ArnMesh* mesh = reinterpret_cast<ArnMesh*>(m_sg->findFirstNodeOfType(NDT_RT_MESH));
        if (mesh)
        {
            bool bHit = false;
            unsigned int faceIdx = 0;
            ArnIntersectGl(mesh, &origin, &direction, &bHit, &faceIdx, 0, 0, 0, 0, 0);
            if (bHit)
                std::cout << "Hit on Face " << std::dec << faceIdx << " of mesh " << mesh->getName() << std::endl;
        }
    }
}

void GlWindow::mouseMoveEvent (QMouseEvent *event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(m_xRot + dy);
        setYRotation(m_yRot + dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(m_xRot + dy);
        setZRotation(m_zRot + dx);
    }
    m_lastPos = event->pos();
}

void GlWindow::viewOptionToggle (int state)
{
    QCheckBox *cb = qobject_cast <QCheckBox *> (sender ());
    if (cb->text().compare("Joint") == 0)
        m_bDrawJointIndicator = (state == Qt::Checked);
    else if (cb->text().compare("Endeffector") == 0)
        m_bDrawEndeffectorIndicator = (state == Qt::Checked);
    else if (cb->text().compare("Joint axis") == 0)
        m_bDrawJointAxisIndicator = (state == Qt::Checked);
    else if (cb->text().compare("Root node") == 0)
        m_bDrawRootNodeIndicator = (state == Qt::Checked);
    else if (cb->text().compare("Grid") == 0)
        m_bDrawGrid = (state == Qt::Checked);

    updateGL();
}

void GlWindow::setXRotation (int angle)
{
    if (angle != m_xRot)
    {
        m_xRot = angle;
        updateGL();
    }
}

void GlWindow::setYRotation (int angle)
{
    if (angle != m_yRot)
    {
        m_yRot = angle;
        updateGL();
    }
}

void GlWindow::setZRotation (int angle)
{
    if (angle != m_zRot)
    {
        m_zRot = angle;
        updateGL();
    }
}

Node *GlWindow::resetIkRoot ()
{
    m_activeIkSolver->reconfigureRoot (m_activeNode);
    // m_activeNode invalidated from now on. Should be reset.
    m_activeNode = m_activeIkSolver->getTree ()->GetRoot ();
    assert (m_activeNode);
    updateGL();
    emit activeNodeChanged (m_activeIkSolver, m_activeNode);
    return m_activeNode;
}
