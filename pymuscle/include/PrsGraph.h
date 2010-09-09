#ifndef PRSGRAPH_H
#define PRSGRAPH_H

class PrsGraphData;

class PrsGraph
{
public:
    PrsGraph();
    ~PrsGraph();
    void attach(int gdid, PrsGraphData *gd);
    void render() const;
    const double *axisColor() const { return m_axisColor; }
    double axisWidth() const { return m_axisWidth; }
    void push_back_to(int gdid, double v);
    int numGraphs() const { return m_gd.size(); }
    void setMaxY(double maxY) { m_maxY = maxY; }
protected:
private:
    std::map<int, PrsGraphData *> m_gd;
    typedef std::pair<int, PrsGraphData *> MapEntry;
    double m_axisColor[3];
    double m_axisWidth;
    double m_maxY;
};

#endif // PRSGRAPH_H
