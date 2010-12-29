/*!
 * @file AranPhy.h
 * @author Geoyeob Kim
 * @date 2009
 *
 * AranPhy API ì£??¤ë” ?Œì¼
 */
#pragma once

class GeneralBody;
class ArnPlane;

/*!
 * @brief ë¬¼ë¦¬ ?¼ì´ë¸ŒëŸ¬ë¦?ì´ˆê¸°??
 * @return ?±ê³µ ??0, ?¤íŒ¨(??ì´ˆê¸°???¬í•¨) ???Œìˆ˜
 */
ARANPHY_API int
ArnInitializePhysics();

/*!
 * @brief ë¬¼ë¦¬ ?¼ì´ë¸ŒëŸ¬ë¦??´ì œ
 * @return ?±ê³µ ??0, ?¤íŒ¨(ë¯?ì´ˆê¸°?????¸ì¶œ ?¬í•¨) ???Œìˆ˜
 * @remarks ì´ˆê¸°?”ê? ?˜ì? ?Šì? ?íƒœ?ì„œ ?´ì œ?˜ëŠ” ê²½ìš° ë°???ë²??´ìƒ ?´ì œ?˜ëŠ” ê²ƒë„ ?¤íŒ¨ë¡?ê°„ì£¼
 */
ARANPHY_API int
ArnCleanupPhysics();

/*!
 * @brief GeneralBody ?€ ArnPlane ?¬ì´??êµì°¨??ê³„ì‚°
 *
 * ?¤ìˆ˜??êµì°¨?ì´ ?ˆì„ ?˜ë„ ?ˆìœ¼ë¯€ë¡?êµì°¨??ë¦¬ìŠ¤?¸ë? ë§Œë“¤???…ë‹ˆ??
 */
ARANPHY_API void
ArnGeneralBodyPlaneIntersection(
	std::vector<ArnVec3>& points,		///< [out] êµì°¨??
	const GeneralBody& gb,			///< [in] êµì°¨ ?ŒìŠ¤?¸ë? ??ë¬¼ë¦¬
	const ArnPlane& plane			///< [in] êµì°¨ ?ŒìŠ¤?¸ë? ???‰ë©´
);

/*!
 * @brief GeneralBody ?€ ?˜ì§??vertical line)ê³¼ì˜ êµì°¨??ê³„ì‚°
 *
 * ?¤ìˆ˜??êµì°¨?ì´ ?ˆì„ ?˜ë„ ?ˆìœ¼ë¯€ë¡?êµì°¨??ë¦¬ìŠ¤?¸ë? ë§Œë“¤???…ë‹ˆ??
 */
ARANPHY_API void
ArnGeneralBodyVerticalLineIntersection(std::vector<ArnVec3>& points,	///< [out] êµì°¨??
									const GeneralBody& gb,			///< [in] êµì°¨ ?ŒìŠ¤?¸ë? ??ë¬¼ë¦¬
									const float x,					///< [in] ?˜ì§? ì˜ X ì¢Œí‘œ
									const float y					///< [in] ?˜ì§? ì˜ Y ì¢Œí‘œ
									);
