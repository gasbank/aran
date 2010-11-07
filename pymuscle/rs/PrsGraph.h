#ifndef PRSGRAPH_H
#define PRSGRAPH_H

class PrsGraphData;

class PrsGraph
{
public:
    explicit PrsGraph(const char *title);
    ~PrsGraph();
    void attach(int gdid, PrsGraphData *gd);
    void render() const;
    const double *axisColor() const { return m_axisColor; }
    double axisWidth() const { return m_axisWidth; }
    void push_back_to(int gdid, double v);
    int numGraphs() const { return m_gd.size(); }
    void setMaxY(double maxY) { m_maxY = maxY; }
    const char *title() const { return m_title.c_str(); }
    void addGuideY(double y, double r, double g, double b);
protected:
private:
    std::map<int, PrsGraphData *> m_gd;
    typedef std::pair<int, PrsGraphData *> MapEntry;
    double m_axisColor[3];
    double m_axisWidth;
    double m_maxY;
    std::string m_title;

    struct Guide {
        Guide(double _xy, double _r, double _g, double _b) : xy(_xy) {
            color[0] = _r;
            color[1] = _g;
            color[2] = _b;
        }
        double xy;
        double color[3];
    };
    std::vector<Guide> m_guideY;
};

#endif // PRSGRAPH_H
