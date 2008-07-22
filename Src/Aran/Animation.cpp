#include "AranPCH.h"
#include "Animation.h"

#define SMALL (-1.0e-10)

Animation::Animation( unsigned int totalCurveCount )
: m_totalCurveCount(totalCurveCount)
{
	m_curves = new AnimCurve[m_totalCurveCount];
	memset(m_curveFlags, 0, sizeof(m_curveFlags));
}
Animation::~Animation(void)
{
	delete m_curves;
}

unsigned int Animation::registerCurve( CurveName curveName, unsigned int pointCount, CurveType curveType )
{
	if (m_curveCount < m_totalCurveCount && m_curveFlags[curveName] == false)
	{
		AnimCurve* curve = &m_curves[m_curveCount];
		curve->curveName = curveName;
		curve->pointCount = pointCount;
		curve->curveType = curveType;
		curve->point2Array = 0;
		++m_curveCount;
		return m_curveCount;
	}
	else
	{
		return 0;
	}
}

bool Animation::setCurvePoint( CurveName curveName, POINT2FLOAT* point2Array )
{
	AnimCurve* curve = findCurve(curveName);
	if (curve)
	{
		curve->point2Array = point2Array;
		return true;
	}
	else
	{
		return false;
	}
}

Animation::AnimCurve* Animation::findCurve( CurveName curveName )
{
	unsigned int i;
	for (i = 0; i < m_totalCurveCount; ++i)
	{
		if (m_curves[i].curveName == curveName)
			return &m_curves[i];
	}
	return 0;
}



void correct_bezpart(float *v1, float *v2, float *v3, float *v4)
{
	/* the total length of the handles is not allowed to be more
	* than the horizontal distance between (v1-v4)
	* this to prevent curve loops
	*/
	float h1[2], h2[2], len1, len2, len, fac;

	h1[0]= v1[0]-v2[0];
	h1[1]= v1[1]-v2[1];
	h2[0]= v4[0]-v3[0];
	h2[1]= v4[1]-v3[1];

	len= v4[0]- v1[0];
	len1= (float)fabs(h1[0]);
	len2= (float)fabs(h2[0]);

	if(len1+len2==0.0) return;
	if(len1+len2 > len) {
		fac= len/(len1+len2);

		v2[0]= (v1[0]-fac*h1[0]);
		v2[1]= (v1[1]-fac*h1[1]);

		v3[0]= (v4[0]-fac*h2[0]);
		v3[1]= (v4[1]-fac*h2[1]);

	}
}

/* *********************** ARITH *********************** */

static double Sqrt3d(double d)
{
	if(d==0.0) return 0;
	if(d<0) return -exp(log(-d)/3);
	else return exp(log(d)/3);
}
static int findzero(float x, float q0, float q1, float q2, float q3, float *o)
{
	double c0, c1, c2, c3, a, b, c, p, q, d, t, phi;
	int nr= 0;

	c0= q0-x;
	c1= 3*(q1-q0);
	c2= 3*(q0-2*q1+q2);
	c3= q3-q0+3*(q1-q2);

	if(c3!=0.0) {
		a= c2/c3;
		b= c1/c3;
		c= c0/c3;
		a= a/3;

		p= b/3-a*a;
		q= (2*a*a*a-a*b+c)/2;
		d= q*q+p*p*p;

		if(d>0.0) {
			t= sqrt(d);
			o[0]= (float)(Sqrt3d(-q+t)+Sqrt3d(-q-t)-a);
			if(o[0]>= SMALL && o[0]<=1.000001) return 1;
			else return 0;
		}
		else if(d==0.0) {
			t= Sqrt3d(-q);
			o[0]= (float)(2*t-a);
			if(o[0]>=SMALL && o[0]<=1.000001) nr++;
			o[nr]= (float)(-t-a);
			if(o[nr]>=SMALL && o[nr]<=1.000001) return nr+1;
			else return nr;
		}
		else {
			phi= acos(-q/sqrt(-(p*p*p)));
			t= sqrt(-p);
			p= cos(phi/3);
			q= sqrt(3-3*p*p);
			o[0]= (float)(2*t*p-a);
			if(o[0]>=SMALL && o[0]<=1.000001) nr++;
			o[nr]= (float)(-t*(p+q)-a);
			if(o[nr]>=SMALL && o[nr]<=1.000001) nr++;
			o[nr]= (float)(-t*(p-q)-a);
			if(o[nr]>=SMALL && o[nr]<=1.000001) return nr+1;
			else return nr;
		}
	}
	else {
		a=c2;
		b=c1;
		c=c0;

		if(a!=0.0) {
			p=b*b-4*a*c;
			if(p>0) {
				p= sqrt(p);
				o[0]= (float)((-b-p)/(2*a));
				if(o[0]>=SMALL && o[0]<=1.000001) nr++;
				o[nr]= (float)((-b+p)/(2*a));
				if(o[nr]>=SMALL && o[nr]<=1.000001) return nr+1;
				else return nr;
			}
			else if(p==0) {
				o[0]= (float)(-b/(2*a));
				if(o[0]>=SMALL && o[0]<=1.000001) return 1;
				else return 0;
			}
		}
		else if(b!=0.0) {
			o[0]= (float)(-c/b);
			if(o[0]>=SMALL && o[0]<=1.000001) return 1;
			else return 0;
		}
		else if(c==0.0) {
			o[0]= 0.0;
			return 1;
		}
		return 0;	
	}
}

static void berekeny(float f1, float f2, float f3, float f4, float *o, int b)
{
	float t, c0, c1, c2, c3;
	int a;

	c0= f1;
	c1= 3.0f*(f2 - f1);
	c2= 3.0f*(f1 - 2.0f*f2 + f3);
	c3= f4 - f1 + 3.0f*(f2-f3);

	for(a=0; a<b; a++) {
		t= o[a];
		o[a]= c0+t*c1+t*t*c2+t*t*t*c3;
	}
}
float Animation::EvalCurveInterp(const CurveData* icu, float ipotime) 
{
	const BezTripleData* bezt;
	const BezTripleData* prevbezt;
	float v1[2], v2[2], v3[2], v4[2], opl[32], fac;
	float dx, cycyofs, cvalue = 0.0;
	int a, b;

	cycyofs= 0.0;
	if (icu->points[0                ].vec[1][0] >= ipotime) return icu->points[0                ].vec[1][1];
	if (icu->points[icu->pointCount-1].vec[1][0] <= ipotime) return icu->points[icu->pointCount-1].vec[1][1];

	if(icu->points.size()) {
		prevbezt= &icu->points[0];
		bezt= prevbezt+1;
		a= icu->points.size()-1;


		/* endpoints? */

		if(prevbezt->vec[1][0]>=ipotime) {
			cvalue= prevbezt->vec[1][1];

			cvalue+= cycyofs;
		}
		else if( (prevbezt+a)->vec[1][0]<=ipotime) {
			if( (/*icu->extrap & IPO_DIR*/ false) && icu->type!=IPO_CONST) {
				dx= prevbezt->vec[1][0]-ipotime;
				fac= prevbezt->vec[1][0]-prevbezt->vec[0][0];
				if(fac!=0.0) {
					fac= (prevbezt->vec[1][1]-prevbezt->vec[0][1])/fac;
					cvalue= prevbezt->vec[1][1]-fac*dx;
				}
				else cvalue= prevbezt->vec[1][1];
			}
			else cvalue= prevbezt->vec[1][1];

			cvalue+= cycyofs;
		}
		else {
			while(a--) {
				if(prevbezt->vec[1][0]<=ipotime && bezt->vec[1][0]>=ipotime) {
					if(icu->type==IPO_CONST) {
						cvalue= prevbezt->vec[1][1]+cycyofs;
					}
					else if(icu->type==IPO_LIN) {
						fac= bezt->vec[1][0]-prevbezt->vec[1][0];
						if(fac==0) cvalue= cycyofs+prevbezt->vec[1][1];
						else {
							fac= (ipotime-prevbezt->vec[1][0])/fac;
							cvalue= cycyofs+prevbezt->vec[1][1]+ fac*(bezt->vec[1][1]-prevbezt->vec[1][1]);
						}
					}
					else {
						v1[0]= prevbezt->vec[1][0];
						v1[1]= prevbezt->vec[1][1];
						v2[0]= prevbezt->vec[2][0];
						v2[1]= prevbezt->vec[2][1];

						v3[0]= bezt->vec[0][0];
						v3[1]= bezt->vec[0][1];
						v4[0]= bezt->vec[1][0];
						v4[1]= bezt->vec[1][1];

						correct_bezpart(v1, v2, v3, v4);

						b= findzero(ipotime, v1[0], v2[0], v3[0], v4[0], opl);
						if(b) {
							berekeny(v1[1], v2[1], v3[1], v4[1], opl, 1);
							cvalue= opl[0]+cycyofs;
							break;
						}
					}
				}
				prevbezt= bezt;
				bezt++;
			}
		}
	}

	/*if(icu->ymin < icu->ymax) {
		if(cvalue < icu->ymin) cvalue= icu->ymin;
		else if(cvalue > icu->ymax) cvalue= icu->ymax;
	}*/

	return cvalue;
}

