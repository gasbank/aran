/**
@file ArnPhyBox.h
@author Geoyeob Kim
@date 2009
*/
#pragma once
#include "GeneralBody.h"

TYPEDEF_SHARED_PTR(ArnPhyBox)

/**
@brief 직육면체의 bounding volume과 질량 분포를 가지는 물리 시뮬레이션 개체
*/
class ARANPHY_API ArnPhyBox : public GeneralBody
{
public:
	/**
	@brief			이름과 질량 중심, 크기, 질량을 입력받아 인스턴스를 생성
	@return			성공 시 새 인스턴스 포인터, 실패 시 0
	@param osc		ODE 컨텍스트 (\c NULL 일 수 있음)
	@param name		이름
	@param com		질량 중심(center of mass)
	@param size		가로, 세로, 높이 순의 크기
	@param mass		질량 (0 보다 큰 값이어야 함)
	@param fixed	강체의 고정 여부
	@remark			ODE 컨텍스트 파라미터를 \c NULL 로 설정했을 경우에는
					이 개체가 SimWorld에 등록될 때 ODE 컨텍스트가 설정됩니다.
	*/
	static ArnPhyBox*		createFrom(const OdeSpaceContext* osc, const char* name, const ArnVec3& com, const ArnVec3& size, float mass, bool fixed);
	virtual					~ArnPhyBox();
protected:
private:
							ArnPhyBox(const OdeSpaceContext* osc);
};
