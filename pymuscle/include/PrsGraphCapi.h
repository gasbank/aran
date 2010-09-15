#ifndef PRSGRAPHCAPI_H
#define PRSGRAPHCAPI_H

#ifdef __cplusplus
#define PRSGRAPH_API extern "C"
#else
#define PRSGRAPH_API
#endif

typedef void* PRSGRAPHDATA;
typedef void* PRSGRAPH;

PRSGRAPH_API PRSGRAPHDATA PrsGraphDataNew(int capacity);
PRSGRAPH_API void PrsGraphDataDelete(PRSGRAPH g);
PRSGRAPH_API void PrsGraphDataPushBack(PRSGRAPHDATA gd, double v);
PRSGRAPH_API void PrsGraphDataSetLineColor(PRSGRAPHDATA gd, double r, double g, double b);

PRSGRAPH_API PRSGRAPH PrsGraphNew(const char *title);
PRSGRAPH_API void PrsGraphDelete(PRSGRAPH g);
PRSGRAPH_API void PrsGraphRender(PRSGRAPH g);
PRSGRAPH_API void PrsGraphAttach(PRSGRAPH g, int gdid, PRSGRAPHDATA gd);
PRSGRAPH_API void PrsGraphPushBackTo(PRSGRAPH g, int gdid, double v);
PRSGRAPH_API void PrsGraphSetMaxY(PRSGRAPH g, double maxY);
PRSGRAPH_API const char *PrsGraphTitle(PRSGRAPH g);
PRSGRAPH_API void PrsGraphAddGuideY(PRSGRAPH g, double y, double red, double green,
                                    double blue);
#endif // PRSGRAPHCAPI_H
