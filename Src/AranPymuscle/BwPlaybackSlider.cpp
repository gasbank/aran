#include "BwPch.h"
#include "BwPlaybackSlider.h"
#include "BwAppContext.h"
#include "BwOpenGlWindow.h"

static void cb(Fl_Widget *o, void *p)
{
	PlaybackSlider* ps = (PlaybackSlider*)p;
	BwAppContext& ac = ps->getAppContext();
	const int frame = (int)ps->value();
	ac.frames = min(frame, ps->getAvailableFrames());
	sprintf(ac.frameStr, "%d", ac.frames);
  ac.frameLabel->redraw_label();
  
  ac.simulateButton->value(0);
  ac.bSimulate = false;

  pym_config_t *pymCfg = &ac.pymRs->pymCfg;
  memcpy(pymCfg->body, &ac.rb_history[ac.frames][0], sizeof(pym_rb_t)*pymCfg->nBody);
  ac.glWindow->redraw();

	if (!ac.bSimulate)
	{
		if (ac.simWorldHistory[frame].generalBodyState.size())
		{
			ac.swPtr->setSimWorldState( ac.simWorldHistory[frame] );
			ac.glWindow->redraw();
		}
	}
}

PlaybackSlider::PlaybackSlider( int x, int y, int w, int h, const char* c, BwAppContext& ac )
: Fl_Hor_Slider(x, y, w, h, c)
, m_availableFrames(0)
, m_ac(ac)
{
  callback(cb, this);
}

PlaybackSlider::~PlaybackSlider(void)
{
}

void PlaybackSlider::draw()
{
	Fl_Hor_Slider::draw();

	Fl_Color c = fl_rgb_color(255, 0, 0);
	fl_rect(x(), y() + h()/2 - 5, w()/10000.0 * m_availableFrames, 10, c);
}

int PlaybackSlider::handle( int eventType )
{
	if (eventType == FL_MOVE || eventType == FL_KEYDOWN || eventType == FL_KEYUP)
	{
		if (value() > m_availableFrames)
		{
			value(m_availableFrames);
			return 1;
		}
	}
	return Fl_Hor_Slider::handle(eventType);
}

void PlaybackSlider::setAvailableFrames( int frames )
{
	m_availableFrames = frames;
}

BwAppContext& PlaybackSlider::getAppContext()
{
	return m_ac;
}