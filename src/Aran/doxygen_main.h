/**
@mainpage Action RPG And Network Library

@section intro 소개
본 프로젝트는 2007년부터 시작된 게임 라이브러리 프로젝트입니다.
오픈소스 모델링 도구인 Blender의 디자인을 많이 참고하여 작성되었고
아직까지 개발 단계에 있습니다. 다만 2008년 여름 진행된 풀쥐 프로젝트에
본 라이브러리가 간단한 모델 렌더링 도구로서 사용되었습니다.

@section objective 개발 목표
본 프로젝트는 RPG 게임을 작성하는데 도움이 되는 기능을 포함하는 라이브러리 작성을 목표로 합니다.

@section significantchanges 풀쥐 프로젝트 이후 바뀐 점
- 렌더러(Direct3D 9과 OpenGL)과 모델 데이터의 분리: 같은 데이터로 D3D 9 혹은 OpenGL로 렌더링 가능하도록 구조를 분리했습니다.
- 수학 관련 함수 라이브러리화
- ARN 파일을 기존 바이너리에서 XML로 변경
- D3D 9 기능 확장 중지 및 OpenGL 지원

@section futurework 향후 계획
- 뼈대 기반 애니메이션과 물리 엔진의 결합

@section working 현재 진행 중인 작업
- 메모리 누수 확인 후 수정 중

@section FIRSTCREATEINFO 최초 문서 작성 정보
- 작성자 : gb
- 작성일 : 2009/08/04

@section MODIFYINFO 최종 문서 수정 정보
- 수정내역 : gb(2009/08/04) --- doxygen 문서 작성 시작
*/



/**
@page components ARAN 라이브러리 구성 요소
<h3>작성: gb 2009-08-04</h3>

ARAN 라이브러리는 아래의 하부 라이브러리로 구성되어 있습니다.
참고로 ARAN은 라이브러리 전체를 칭하는 말이고 Aran은 하부 라이브러리
중 하나의 이름입니다. 아직 네트워크 관련 라이브러리는 구현되지 않았습니다.

@section aranmath AranMath
모든 다른 하부 라이브러리의 기본이 되는 수학 관련 라이브러리입니다.
기본적인 벡터, 행렬 계산을 포함해 ray-triangle 교차 테스트 함수도
제공합니다. 기본적인 계산은 직접 작성되었지만 많은 응용된 수학 계산
루틴은 <a href="http://www.cmldev.net/">CML</a>
(configurable math library)의 것을 사용했습니다.

@section aran Aran
scene graph를 포함해 메시, 카메라, 빛, 물질 같은 렌더링에 필요한
자료 구조를 처리하는 클래스와 함수가 포함된 라이브러리입니다.
이 라이브러리는 Direct3D나 OpenGL같은 3D 렌더링 라이브러리와
독립적으로 작성되었으므로 렌더링과 직접적으로 관련된 자료나 함수를
포함하고 있지 않습니다.
본 라이브러리의 주된 기능은 Blender exporter로 만들어진 XML 및 바이너리
모델 파일을 읽어들여 적절한 scene graph(ArnSceneGraph)를 만들어
내는 것입니다. 이러한 scene graph는 AranGl이나 AranDx9에 제공된 렌더링
함수를 이용해 화면에 출력할 수 있게 됩니다. 이 때 사용된 XML 파서는
<a href="http://www.grinninglizard.com/tinyxml/">TinyXML</a>입니다.
AranMath 라이브러리를 필요로 합니다.

@section aranphymath AranPhy
강체 물리 시뮬레이션을 지원하기 위한 라이브러리입니다. 특정 메시의
bounding volume과 질량 등을 설정하여 물리 법칙에 따라 움직이도록 도와주는
자료 구조 및 함수를 제공합니다. 시뮬레이터는
<a href="http://opende.sourceforge.net/">OpenDE</a>(Open Dynamics Engine)를
사용했습니다. 본 라이브러리는 현재 기능이 구현되지 않았습니다.
AranMath 라이브러리를 필요로 합니다.

@section aranrenderer AranGl / AranDx9
Aran 라이브러리는 렌더러에 독립적인 자료 구조 및 함수만을 지원합니다.
이는 사용되는 렌더러의 기능과 렌더링에 필요한 자료를 구분하여 특정 렌더러에
종속적인 자료 구조가 되지 않도록 하기 위함입니다. AranGl과 AranDx9는 Aran
라이브러리에서 정의된 자료를 이용해 각각 OpenGL이나 Direct3D 9에서 렌더링하는
기능을 모아둔 라이브러리입니다. 현재 AranDx9는 기능이 구현되지 않았습니다.
AranMath, Aran 라이브러리를 필요로 합니다.
*/