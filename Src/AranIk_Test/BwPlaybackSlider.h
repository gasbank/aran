#pragma once

class BwAppContext;

class PlaybackSlider : public Fl_Hor_Slider
{
public:
							PlaybackSlider(int x, int y, int w, int h, const char* c, BwAppContext& ac);
	virtual					~PlaybackSlider(void);
	int						handle(int eventType);
	void					setAvailableFrames(int frames);
	BwAppContext&			getAppContext();
protected:
	 virtual void			draw();
	 unsigned int			m_availableFrames;
	 BwAppContext&			m_ac;
};
