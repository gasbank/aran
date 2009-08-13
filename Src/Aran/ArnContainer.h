/*!
 * @file ArnContainer.h
 * @author Geoyeob Kim
 * @date 2009
 */
#pragma once

#include "ArnNode.h"

class ArnContainer; typedef std::tr1::shared_ptr<ArnContainer> ArnContainerPtr;

/*!
 * @brief ArnNode를 계층적으로 저장하고 싶을 때 사용하는 클래스
 *
 * ArnNode는 순수 가상 함수가 포함되어있기 때문에 instantiate를 할 수 없습니다.
 * 이 클래스는 순수 가상 함수를 모두 구현하여 개체를 생성할 수 있습니다.
 */
class ArnContainer : public ArnNode
{
public:
							~ArnContainer(void);
	/*!
	 * @brief 빈 container를 생성하여 반환
	 */
	static ArnContainerPtr	createFromEmpty();
	/*!
	 * @name Internal use only methods
	 * These methods are exposed in order to make internal linkage between objects.
	 * You should aware that these are not for client-side APIs.
	 */
	//@{
	virtual void			interconnect(ArnNode* sceneRoot) { ArnNode::interconnect(sceneRoot); }
	//@}
private:
							ArnContainer(void);
};


