#pragma once

#include "ArnObject.h"

class ARAN_API ArnRenderableObject : public ArnObject
{
public:
	virtual									~ArnRenderableObject(void);
	virtual const char*						getName() const { return m_name.c_str(); }
	//virtual int								initialize() = 0;
	virtual int								render() = 0; // TODO: ArnMesh---render, ArnTexture---bind?
	virtual void							cleanup()  = 0;
	bool									isInitialized() const { return m_bInitialized; }
	RendererType							getRendererType() const { return m_rendererType; }
protected:
											ArnRenderableObject(void);
	void									setInitialized(bool b) { m_bInitialized = b; }
	void									setRendererType(RendererType rt) { m_rendererType = rt; }
private:
	std::string								m_name;
	bool									m_bInitialized;
	RendererType							m_rendererType;
};
