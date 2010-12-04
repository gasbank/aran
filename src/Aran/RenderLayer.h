#pragma once

class VideoMan;

class RenderLayer
{
public:
								RenderLayer(void);
	virtual						~RenderLayer(void);
	virtual HRESULT				render() = 0;
	virtual HRESULT				update(double fTime, float fElapsedTime) = 0;
	void						setVideoMan( VideoMan* pVideoMan ) { m_pVideoMan = pVideoMan; }
	void						setVisible(bool b) { m_bVisible = b; }
	bool						getVisible() { return m_bVisible; }
protected:
	VideoMan*					m_pVideoMan;
	bool						m_bVisible;
};
