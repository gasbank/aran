/*!
 * @file AranIk.h
 * @author Geoyeob Kim
 * @date 2009
 */
#pragma once

class ArnSkeleton;
class ArnIkSolver;
TYPEDEF_SHARED_PTR(ArnSceneGraph)

/*!
 * @brief ArnSkeleton으로부터 IK 솔버 생성
 * @param solver 새로 생성된 솔버가 반환됨
 * @param skel 입력되는 ArnSkeleton
 */
ARANIK_API void CreateArnIkSolver(ArnIkSolver** solver, const ArnSkeleton* skel);

/*!
 * @brief ArnSceneGraph의 루트에 속한 ArnSkeleton에 대해 ArnIkSolver를 생성
 * @param ikSolvers 생성된 ArnIkSolver의 포인터 배열
 * @param sg scene graph
 * @remark ArnSkeleton을 재귀적으로 모두 찾는 것이 아니라 최상위 노드(root)에서만 검색합니다.
 */
ARANIK_API void ArnCreateArnIkSolversOnSceneGraph( std::vector<ArnIkSolver*>& ikSolvers, ArnSceneGraphPtr sg );
