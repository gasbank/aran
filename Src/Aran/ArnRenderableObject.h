#pragma once

#include "ArnNode.h"

class ARAN_API ArnRenderableObject : public ArnNode
{
public:
	virtual									~ArnRenderableObject(void);
	virtual const char*						getName() const { return m_name.c_str(); }
	//virtual int								initialize() = 0;
	virtual int								render() const = 0; // TODO: ArnMesh---render, ArnTexture---bind?
	virtual void							cleanup()  = 0;
	bool									isInitialized() const { return m_bInitialized; }
	RendererType							getRendererType() const { return m_rendererType; }

	/*! @name Internal use only methods
	These methods are exposed in order to make internal linkage between objects or initialization.
	Clients should aware that these are not for client-side APIs.
	*/
	//@{
	virtual void							interconnect(ArnNode* sceneRoot);
	//@}
protected:
											ArnRenderableObject(void);
	void									setInitialized(bool b) { m_bInitialized = b; }
	void									setRendererType(RendererType rt) { m_rendererType = rt; }
private:
	std::string								m_name;
	bool									m_bInitialized;
	RendererType							m_rendererType;
};
