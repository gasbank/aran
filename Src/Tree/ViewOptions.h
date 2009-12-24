#ifndef VIEWOPTIONS_H
#define VIEWOPTIONS_H

class GlWindow;

class ViewOptions : public QWidget
{
Q_OBJECT
public:
    explicit ViewOptions (QWidget *parent, GlWindow *glWindow);
    ~ViewOptions ();
signals:

public slots:

private:
    QMap<QString, QCheckBox *> m_checkBoxes;
};

#endif // VIEWOPTIONS_H
