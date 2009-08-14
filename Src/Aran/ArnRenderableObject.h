/*!
 * @file ArnRenderableObject.h
 * @author Geoyeob Kim
 * @date 2009
 */
#pragma once

#include "ArnNode.h"

/*!
 * @brief 렌더러 컨텍스트와 관련되어 있는 클래스의 인터페이스 제공
 */
class ARAN_API ArnRenderableObject : public ArnNode
{
public:
	virtual									~ArnRenderableObject(void);
	virtual const char*						getName() const { return m_name.c_str(); }
	/*!
	 * @brief 렌더러 컨텍스트에 적용
	 *
	 * ArnMesh라면 메시가 렌더링되는 함수가 호출되고, ArnTexture라면 텍스처가
	 * 렌더러 컨텍스트에 바인딩되는 등의 일을 하는 함수입니다.
	 */
	virtual int								render() const = 0; // TODO: ArnMesh---render, ArnTexture---bind?
	virtual void							cleanup()  = 0;
	/*!
	 * @brief 렌더러 컨텍스트 의존 자료의 초기화 여부 알아내기
	 * @sa ArnRenderableObject::render
	 */
	bool									isInitialized() const { return m_bInitialized; }
	/*!
	 * @brief 어떤 렌더러 컨텍스트에 맞춰진 인스턴스인지 알아내기
	 */
	RendererType							getRendererType() const { return m_rendererType; }
	/*!
	 * @internalonly
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
