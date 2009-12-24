#ifndef TREEMAINWINDOW_H
#define TREEMAINWINDOW_H

class SceneGraphModel;
class GlWindow;
class ViewOptions;
class NodeProperties;
class IkSolverProperties;
class SimWorld;

class TreeMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    TreeMainWindow(QWidget *parent = 0);
    virtual ~TreeMainWindow();

public slots:
    void open();
    void openRecentFile();
    void about();
    void resetDockingStates();
    void selectedNodeChanged(const ArnNode *);
    void activeNodeChanged (const ArnIkSolver *, const Node *);
    void resetIkRoot ();
protected:


private:
    void createActions();
    void createMenus();
	void createHorizontalGroupBox();
    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    void loadFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
    void extractCameras(ArnNode *sg);
    void updateCameraButtons();
    void selectedNodeChanged_Int (const ArnNode *selected, const QModelIndex &parent = QModelIndex ());
	enum { NumGridRows = 3, NumButtons = 4 };
    QPushButton *m_cameraButtons[NumButtons];
    QAction *m_cameraActs[NumButtons];
    QTreeView *m_sgTreeView;
	QGroupBox *m_horizontalGroupBox;
    QLabel *m_cameraEmptyLabel;
    QPushButton *m_refreshGl;
    QPushButton *m_resetIkTreeRootButton;

    ViewOptions *m_viewOptions;
    NodeProperties *m_nodeProp;
    IkSolverProperties *m_ikSolverProp;
    ArnSceneGraph *m_sg;
    SceneGraphModel *m_sgModel;
    QLabel *m_sceneName;
    GlWindow* m_glWindow;
    QList<ArnCamera *> m_cameras;
    QLabel *m_reconstructIkLabel;

    QMenu *m_fileMenu;
    QMenu *m_windowMenu;
    QMenu *m_helpMenu;
    QAction *m_openAct;
    QAction *m_resetDockingStatesAct;
    QAction *m_exitAct;
    QAction *m_aboutAct;
    QAction *m_separatorAct;

    enum { MaxRecentFiles = 10 };
    QAction *m_recentFileActs[MaxRecentFiles];

    QString m_curFile;
    SimWorldPtr m_simWorld;
};

#endif // TREEMAINWINDOW_H
