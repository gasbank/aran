#ifndef GLWINDOW_H
#define GLWINDOW_H

enum ViewMode
{
    VM_UNKNOWN,
    VM_TOP,		// KeyPad 7
    VM_RIGHT,	// KeyPad 3
    VM_BACK,	// KeyPad 1
    VM_BOTTOM,	// Shift + KeyPad 7
    VM_LEFT,	// Shift + KeyPad 3
    VM_FRONT,	// Shift + KeyPad 1
    VM_CAMERA
};

class GlWindow : public QGLWidget
{
    Q_OBJECT
public:
    GlWindow (QWidget *parent);
    void setSceneGraph (ArnSceneGraph *sg);
    void setActiveCam (ArnCamera *cam);

signals:
    void selectedChanged (const ArnNode *node);
    void activeNodeChanged (const ArnIkSolver *ikSolver, const Node *node);
    void ikSolversChanged (const std::vector <ArnIkSolver *> &newIkSolvers);
    void refreshChanged (bool refresh);

public slots:
    void setRefresh (bool refresh);
    void setActiveCam ();
    void viewOptionToggle (int state);
    void setXRotation (int angle);
    void setYRotation (int angle);
    void setZRotation (int angle);
    Node *resetIkRoot ();
protected:
    void initializeGL ();
    void resizeGL (int w, int h);
    void paintGL ();
    void timerEvent (QTimerEvent * event);
    void mousePressEvent (QMouseEvent * event);
    void mouseMoveEvent (QMouseEvent *event);
private:
    void selectGraphicObject (const float mousePx, const float mousePy);

    ArnSceneGraph *m_sg;
    ArnCamera *m_activeCam;
    ArnLight *m_activeLight;
    bool m_bRefresh;
    int m_refreshTimer;

    std::vector <ArnIkSolver *> m_ikSolvers;
    bool m_bDrawJointIndicator;
    bool m_bDrawEndeffectorIndicator;
    bool m_bDrawJointAxisIndicator;
    bool m_bDrawRootNodeIndicator;
    bool m_bDrawGrid;
    ArnViewportData avd;
    QPoint m_lastPos;
    int m_xRot;
    int m_yRot;
    int m_zRot;
    ViewMode m_viewMode;

    ArnIkSolver *m_activeIkSolver;
    Node *m_activeNode;
};

#endif // GLWINDOW_H
