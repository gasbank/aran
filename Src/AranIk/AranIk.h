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
 * @brief ArnSceneGraph의 루트에 속한 ArnSkeleton에 대해 ArnIkSolver를 생성
 * @param ikSolvers 생성된 ArnIkSolver의 포인터 배열
 * @param sg scene graph
 * @remarks ArnSkeleton을 재귀적으로 모두 찾는 것이 아니라 최상위 노드(root)에서만 검색합니다.
 */
ARANIK_API void ArnCreateArnIkSolversOnSceneGraph( std::vector<ArnIkSolver*>& ikSolvers, ArnSceneGraphPtr sg );
ARANIK_API void ArnCreateArnIkSolversOnSceneGraph( std::vector<ArnIkSolver*>& ikSolvers, ArnSceneGraph* sg );
