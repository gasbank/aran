#ifndef WIN32
#include <GL/glew.h>
#include <GL/glxew.h>
#include <sys/time.h>
#include <pthread.h>
#else
#include <windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/gl.h>
#endif
#include <map>
#include <vector>
#include <string>
#include <boost/foreach.hpp>
#include <boost/circular_buffer.hpp>
#define foreach         BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH
#include "../include/PrsGraphData.h"
#include "../include/PrsGraph.h"

PrsGraph::PrsGraph(const char *title)
: m_axisWidth(1.0)
, m_maxY(1.0)
, m_title(title)
{
    /* Default axis color is white */
    m_axisColor[0] = 1.0;
    m_axisColor[1] = 1.0;
    m_axisColor[2] = 1.0;
}

PrsGraph::~PrsGraph()
{
    foreach (MapEntry gd, m_gd) {
        delete gd.second;
    }
    m_gd.clear();
}

void PrsGraph::render() const {
    glPushAttrib(GL_LINE_BIT | GL_CURRENT_BIT | GL_POINT_BIT); /* PAIR A */
    /* X and Y axis */
    glColor3dv(axisColor());
    glLineWidth(axisWidth());
    glBegin(GL_LINES);
    glVertex2f(0,0); glVertex2f(1,0);
    glVertex2f(0,0); glVertex2f(0,1);
    glEnd();
    foreach (MapEntry gd, m_gd) {
        PrsGraphData *G = gd.second;
        glLineWidth(G->lineWidth());
        glPointSize(G->pointSize());
        /* Data in piecewise solid line segments */
        glColor3dv(G->lineColor());
        glBegin(GL_LINES);
        for (int i = 0; i < G->size()-1; ++i) {
            glVertex2f( (double)(i  )/G->capacity(), G->at(G->xoffset() + i  )/m_maxY);
            glVertex2f( (double)(i+1)/G->capacity(), G->at(G->xoffset() + i+1)/m_maxY);
        }
        /* Guides */
        foreach (const Guide &guide, m_guideY) {
            glColor3dv(guide.color);
            glVertex2f( 0, guide.xy/m_maxY);
            glVertex2f( 1, guide.xy/m_maxY);
        }
        glEnd();
        /* Data in points */
        glBegin(GL_POINTS);
        glColor3dv(G->lineColor());
        for (int i = 0; i < G->size()-1; ++i) {
            glVertex2f( (double)(i+1)/G->capacity(), G->at(G->xoffset() + i+1)/m_maxY);
        }
        glEnd();
    }
    glPopAttrib(); /* PAIR A */
}

void PrsGraph::push_back_to(int gdid, double v) {
    m_gd[gdid]->push_back(v);
}

void PrsGraph::attach(int gdid, PrsGraphData *gd) {
    assert(!gd->attached());
    gd->setAttached(true);
    assert(m_gd.find(gdid) == m_gd.end());
    m_gd[gdid] = gd;
}

void PrsGraph::addGuideY(double y, double r, double g, double b) {
    m_guideY.push_back( Guide(y, r, g, b) );
}
