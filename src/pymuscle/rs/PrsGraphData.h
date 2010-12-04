#ifndef PRSGRAPHDATA_H
#define PRSGRAPHDATA_H

class PrsGraph;

class PrsGraphData
{
public:
    PrsGraphData(int capacity);
    ~PrsGraphData();
    void push_back(double v) { m_data.push_back(v); }
    double at(int i) { return m_data[i]; }
    const double *lineColor() const { return m_lineColor; }
    void setLineColor(double r, double g, double b);
    double lineWidth() const { return m_lineWidth; }
    double pointSize() const { return m_pointSize; }
    int size() const { return m_data.size(); }
    int capacity() const { return m_data.capacity(); }
    int xoffset() const { return m_xoffset; }
    void setXoffset(int xoffset) { m_xoffset = xoffset; }
    bool attached() const { return m_attached; }
    void setAttached(bool attached) { m_attached = attached; }
protected:
private:
    boost::circular_buffer<double> m_data;
    double m_lineColor[3];
    double m_pointSize;
    double m_lineWidth;
    int m_xoffset;
    bool m_attached;
};

#endif // PRSGRAPHDATA_H
