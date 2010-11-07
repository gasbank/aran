#include <boost/circular_buffer.hpp>
#include "../include/PrsGraphData.h"

PrsGraphData::PrsGraphData(int capacity)
: m_data(capacity)
, m_pointSize(3.0)
, m_lineWidth(1.0)
, m_xoffset(0)
, m_attached(false)
{
    /* Default line color is green */
    m_lineColor[0] = 0.0;
    m_lineColor[1] = 1.0;
    m_lineColor[2] = 0.0;
}

PrsGraphData::~PrsGraphData()
{
    //dtor
}

void PrsGraphData::setLineColor(double r, double g, double b) {
    m_lineColor[0] = r;
    m_lineColor[1] = g;
    m_lineColor[2] = b;
}
