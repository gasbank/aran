#pragma once

/*!
 * @brief Data structure for Direct3D compatibility. (D3DVIEWPORT9)
 *
 * 원점은 모니터 상단 왼쪽이며 구석입니다.
 * 오른쪽으로 가는 것이 가로축 양의 방향이고 아랫쪽으로 가는 것이 세로축 양의 방향입니다.
 */
struct ArnViewportData
{
	DWORD					X;			///< 가로축 시작점
	DWORD					Y;			///< 세로축 시작점
	DWORD					Width;		///< 가로 길이
	DWORD					Height;		///< 세로 길이
	float					MinZ;		///< 클립 볼륨의 최소 Z값
	float					MaxZ;		///< 클립 볼륨의 최대 Z값
};
