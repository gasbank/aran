/*!
 * @file Jacobian.h
 * @author http://math.ucsd.edu/~sbuss/ResearchWeb/ikmethods/index.html
 * @author Geoyeob Kim
 */
#include "Node.h"
#include "Tree.h"
#include "MathMisc.h"
#include "LinearR3.h"
#include "VectorRn.h"
#include "MatrixRmn.h"

#ifndef _CLASS_JACOBIAN
#define _CLASS_JACOBIAN

#ifdef _DYNAMIC
const double BASEMAXDIST = 0.02;
#else
const double MAXDIST = 0.08;	// optimal value for double Y shape : 0.08
#endif
const double DELTA = 0.4;
const long double LAMBDA = 2.0;		// only for DLS. optimal : 0.24
const double NEARZERO = 0.0000000001;

enum UpdateMode
{
	JACOB_Undefined = 0,
	JACOB_JacobianTranspose = 1,
	JACOB_PseudoInverse = 2,
	JACOB_DLS = 3,
	JACOB_SDLS = 4
};

class Jacobian
{
public:
								Jacobian(TreePtr);
								~Jacobian();
	static void					CompareErrors( const Jacobian& j1, const Jacobian& j2, double* weightedDist1, double* weightedDist2 );
	static void					CountErrors( const Jacobian& j1, const Jacobian& j2, int* numBetter1, int* numBetter2, int* numTies );
	void						ComputeJacobian();
	const MatrixRmn&			ActiveJacobian() const { return *m_Jactive; }
	void						SetJendActive() { m_Jactive = &m_Jend; }						// The default setting is Jend.
	void						SetJtargetActive() { m_Jactive = &m_Jtarget; }
	void						CalcDeltaThetas();			// Use this only if the Current Mode has been set.
	void						ZeroDeltaThetas();
	void						CalcDeltaThetasTranspose();
	void						CalcDeltaThetasPseudoinverse();
	void						CalcDeltaThetasDLS();
	void						CalcDeltaThetasDLSwithSVD();
	void						CalcDeltaThetasSDLS();
	void						UpdateThetas();
	double						UpdateErrorArray();		// Returns sum of errors
	const VectorRn&				GetErrorArray() const { return m_errorArray; }
	void						UpdatedSClampValue();
	void						DrawEigenVectors() const;
	void						SetCurrentMode( UpdateMode mode ) { m_CurrentUpdateMode = mode; }
	UpdateMode					GetCurrentMode() const { return m_CurrentUpdateMode; }
	void						SetDampingDLS( double lambda ) { m_DampingLambda = lambda; m_DampingLambdaSq = Square(lambda); }
	void						Reset();
	void						setTarget(unsigned int i, const VectorR3& v);

private:
	void						CalcdTClampedFromdS();

	// Parameters for pseudoinverses
	static const double			PseudoInverseThresholdFactor;		// Threshold for treating eigenvalue as zero (fraction of largest eigenvalue)
	// Parameters for damped least squares
	static const double			DefaultDampingLambda;
	// Cap on max. value of changes in angles in single update step
	static const double			MaxAngleJtranspose;
	static const double			MaxAnglePseudoinverse;
	static const double			MaxAngleDLS;
	static const double			MaxAngleSDLS;	
	static const double			BaseMaxTargetDist;

	TreePtr						m_tree;			// tree associated with this Jacobian matrix
	int							m_nEffector;		// Number of end effectors
	int							m_nJoint;			// Number of joints
	int							m_nRow;			// Total number of rows the real J (= 3*number of end effectors for now)
	int							m_nCol;			// Total number of columns in the real J (= number of joints for now)
	MatrixRmn					m_Jend;		// Jacobian matrix based on end effector positions
	MatrixRmn					m_Jtarget;	// Jacobian matrix based on target positions
	MatrixRmn					m_Jnorms;	// Norms of 3-vectors in active Jacobian (SDLS only)
	MatrixRmn					m_U;		// J = U * Diag(w) * V^T	(Singular Value Decomposition)
	VectorRn					m_w;
	MatrixRmn					m_V;
	UpdateMode					m_CurrentUpdateMode;
	VectorRn					m_dS;			// delta s
	VectorRn					m_dT;			// delta t		--  these are delta S values clamped to smaller magnitude
	VectorRn					m_dSclamp;		// Value to clamp magnitude of dT at.
	VectorRn					m_dTheta;		// delta theta
	VectorRn					m_dPreTheta;		// delta theta for single eigenvalue  (SDLS only)
	VectorRn					m_errorArray;	// Distance of end effectors from target after updating
	double						m_DampingLambda;
	double						m_DampingLambdaSq;
	//double					DampingLambdaSDLS;
	MatrixRmn*					m_Jactive;
	VectorR3					m_target[10];
};

typedef std::tr1::shared_ptr<Jacobian> JacobianPtr;

#endif
