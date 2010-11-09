#ifndef __FIBEREFFECTIMP_H__

/*
typedef struct _InputStruct
{
	double px_org, py_org, pz_org;
    double qw_org, qx_org, qy_org, qz_org;
    double pdx_org, pdy_org, pdz_org;
    double qdw_org, qdx_org, qdy_org, qdz_org;
    double m_org;
    double Ixx_org, Iyy_org, Izz_org;

    double px_ins, py_ins, pz_ins;
    double qw_ins, qx_ins, qy_ins, qz_ins;
    double pdx_ins, pdy_ins, pdz_ins;
    double qdw_ins, qdx_ins, qdy_ins, qdz_ins;
    double m_ins;
    double Ixx_ins, Iyy_ins, Izz_ins;

    double KSE, KPE, b, xrest, T, A;
    double fibbx_org, fibby_org, fibbz_org;
    double fibbx_ins, fibby_ins, fibbz_ins;
} InputStruct;

typedef struct _OutputStruct
	int a, b, c;
} OutputStruct;
*/

int FiberEffectImp(const Double_48 input,
                   const int       bClearVariable,
                   Double_2x14     yd_Q_orgins,
                   double          *Td,
                   Double_2x14     dTddy_orgins,
                   Double_2x14     dyd_Q_orginsdT,
                   double          *dTddT,
                   /* Sparse matrix structure of 'dyd_Q_orginsdy_orgins' */
                   Uint_784x2      dyd_Q_orginsdy_orgins_keys,
                   Double_784      dyd_Q_orginsdy_orgins_values);

#endif
