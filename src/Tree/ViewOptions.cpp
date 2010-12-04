#include "TreePch.h"
#include "ViewOptions.h"
#include "GlWindow.h"


static const char *Options[] =
{
    "Grid",
    "HUD",
    "Joint",
    "Endeffector",
    "Joint axis",
    "Contact",
    "Contact force",
    "Root node",
};

static const int OptionsCount = int (sizeof (Options) / sizeof (Options[0]));

ViewOptions::ViewOptions (QWidget *parent, GlWindow *glWindow) :
    QWidget (parent)
{
    QSettings settings;
    for (int i = 0; i < OptionsCount; ++i)
    {
        m_checkBoxes[Options[i]] = new QCheckBox (Options[i], this);
        connect (m_checkBoxes[Options[i]], SIGNAL (stateChanged (int)),
                 glWindow, SLOT (viewOptionToggle (int)));

        int state = settings.value (tr ("viewOptions-%1").arg (Options[i])).toInt ();
        m_checkBoxes[Options[i]]->setCheckState (Qt::CheckState (state));
    }

    QVBoxLayout *layout = new QVBoxLayout ();
    layout->setSpacing (0);
    layout->setContentsMargins (2,2,2,2);
    foreach (QCheckBox *cb, m_checkBoxes)
    {
        layout->addWidget (cb);
    }

    setLayout (layout);
}

ViewOptions::~ViewOptions ()
{
    QSettings settings;
    for (int i = 0; i < OptionsCount; ++i)
    {
        settings.setValue (tr ("viewOptions-%1").arg (Options[i]),
                           m_checkBoxes[Options[i]]->checkState ());
    }
}
