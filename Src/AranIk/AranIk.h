/*!
 * @file AranIk.h
 * @author Geoyeob Kim
 * @date 2009
 */
#pragma once

class ArnSkeleton;
class ArnIkSolver;

/*!
 * @brief ArnSkeleton으로부터 IK 솔버 생성
 * @param solver 새로 생성된 솔버가 반환됨
 * @param skel 입력되는 ArnSkeleton
  */
ARANIK_API void CreateArnIkSolver(ArnIkSolver** solver, const ArnSkeleton* skel);
