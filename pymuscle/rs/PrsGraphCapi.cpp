#include "PymRsPch.h"
#include "PrsGraphCapi.h"
#include "PrsGraphData.h"
#include "PrsGraph.h"

// PrsGraphData

PRSGRAPHDATA PrsGraphDataNew(int capacity) {
    return new PrsGraphData(capacity);
}

void PrsGraphDataDelete(PRSGRAPHDATA gd) {
    PrsGraphData *GD = reinterpret_cast<PrsGraphData *>(gd);
    delete GD;
}

void PrsGraphDataPushBack(PRSGRAPHDATA gd, double v) {
    PrsGraphData *GD = reinterpret_cast<PrsGraphData *>(gd);
    GD->push_back(v);
}

void PrsGraphDataSetLineColor(PRSGRAPHDATA gd, double r, double g, double b) {
    PrsGraphData *GD = reinterpret_cast<PrsGraphData *>(gd);
    GD->setLineColor(r, g, b);
}


// PrsGraph

PRSGRAPH PrsGraphNew(const char *title) {
    PrsGraph *G = new PrsGraph(title);
    return G;
}

void PrsGraphDelete(PRSGRAPH g) {
    PrsGraph *G = reinterpret_cast<PrsGraph *>(g);
    delete G;
}

void PrsGraphRender(PRSGRAPH g) {
    PrsGraph *G = reinterpret_cast<PrsGraph *>(g);
    G->render();
}

void PrsGraphAttach(PRSGRAPH g, int gdid, PRSGRAPHDATA gd) {
    assert(gd);
    PrsGraph *G = reinterpret_cast<PrsGraph *>(g);
    PrsGraphData *GD = reinterpret_cast<PrsGraphData *>(gd);
    G->attach(gdid, GD);
}

void PrsGraphPushBackTo(PRSGRAPH g, int gdid, double v) {
    PrsGraph *G = reinterpret_cast<PrsGraph *>(g);
    assert(0 <= gdid && gdid < G->numGraphs());
    G->push_back_to(gdid, v);
}

void PrsGraphSetMaxY(PRSGRAPH g, double maxY) {
    PrsGraph *G = reinterpret_cast<PrsGraph *>(g);
    G->setMaxY(maxY);
}

const char *PrsGraphTitle(PRSGRAPH g) {
    PrsGraph *G = reinterpret_cast<PrsGraph *>(g);
    return G->title();
}

void PrsGraphAddGuideY(PRSGRAPH g, double y, double red, double green,
                       double blue) {
    PrsGraph *G = reinterpret_cast<PrsGraph *>(g);
    G->addGuideY(y, red, green, blue);
}
