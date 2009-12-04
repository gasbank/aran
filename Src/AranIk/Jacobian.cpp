#include "AranIkPCH.h"
#include "Jacobian.h"

void Arrow(const VectorR3& tail, const VectorR3& head);

///////////////////extern int RestPositionOn;
int RestPositionOn;
///////////////////extern VectorR3 target[];

//VectorR3 target[10];

// Optimal damping values have to be determined in an ad hoc manner  (Yuck!)
// const double Jacobian::DefaultDampingLambda = 0.6;		// Optimal for the "Y" shape (any lower gives jitter)
const double Jacobian::DefaultDampingLambda = 1.1;			// Optimal for the DLS "double Y" shape (any lower gives jitter)
// const double Jacobian::DefaultDampingLambda = 0.7;			// Optimal for the DLS "double Y" shape with distance clamping (lower gives jitter)

const double Jacobian::PseudoInverseThresholdFactor = 0.01;
const double Jacobian::MaxAngleJtranspose = 30.0*DegreesToRadians;
const double Jacobian::MaxAnglePseudoinverse = 5.0*DegreesToRadians;
const double Jacobian::MaxAngleDLS = 45.0*DegreesToRadians;
const double Jacobian::MaxAngleSDLS = 45.0*DegreesToRadians;
const double Jacobian::BaseMaxTargetDist = 0.4;

Jacobian::Jacobian(TreePtr tree)
{
	Jacobian::m_tree = tree;
	m_nEffector = tree->GetNumEffector();
	m_nJoint = tree->GetNumJoint();
	m_nRow = 3 * m_nEffector;
	m_nCol = m_nJoint;

	m_Jend.SetSize(m_nRow, m_nCol);				// The Jocobian matrix
	m_Jend.SetZero();
	m_Jtarget.SetSize(m_nRow, m_nCol);			// The Jacobian matrix based on target positions
	m_Jtarget.SetZero();
	SetJendActive();

	m_U.SetSize(m_nRow, m_nRow);				// The U matrix for SVD calculations
	m_w.SetLength(Min(m_nRow, m_nCol));
	m_V.SetSize(m_nCol, m_nCol);				// The V matrix for SVD calculations

	m_dS.SetLength(m_nRow);			// (Target positions) - (End effector positions)
	m_dTheta.SetLength(m_nCol);		// Changes in joint angles
	m_dPreTheta.SetLength(m_nCol);

	// Used by Jacobian transpose method & DLS & SDLS
	m_dT.SetLength(m_nRow);			// Linearized change in end effector positions based on dTheta

	// Used by the Selectively Damped Least Squares Method
	//dT.SetLength(nRow);
	m_dSclamp.SetLength(m_nEffector);
	m_errorArray.SetLength(m_nEffector);
	m_Jnorms.SetSize(m_nEffector, m_nCol);		// Holds the norms of the active J matrix

	Reset();
}

Jacobian::~Jacobian()
{
}
void Jacobian::Reset()
{
	// Used by Damped Least Squares Method
	m_DampingLambda = DefaultDampingLambda;
	m_DampingLambdaSq = Square(m_DampingLambda);
	// DampingLambdaSDLS = 1.5*DefaultDampingLambda;

	m_dSclamp.Fill(HUGE_VAL);
}

// Compute the deltaS vector, dS, (the error in end effector positions
// Compute the J and K matrices (the Jacobians)
void Jacobian::ComputeJacobian()
{
	// Traverse tree to find all end effectors
	VectorR3 temp;
	Node* n = m_tree->GetRoot();
	while ( n ) {
		if ( n->isEndeffector() ) {
			int i = n->getEffectorNum();
			const VectorR3& targetPos = n->getTarget();

			// Compute the delta S value (differences from end effectors to target positions.
			temp = targetPos;
			temp -= n->getGlobalPosition();
			m_dS.SetTriple(i, temp);

			// Find all ancestors (they will usually all be joints)
			// Set the corresponding entries in the Jacobians J, K.
			Node* m = m_tree->GetParent(n);
			while ( m ) {
				int j = m->getJointNum();
				assert ( 0 <=i && i<m_nEffector && 0<=j && j<m_nJoint );
				if ( m->isFrozen() ) {
					m_Jend.SetTriple(i, j, VectorR3::Zero);
					m_Jtarget.SetTriple(i, j, VectorR3::Zero);
				}
				else {
					temp = m->getGlobalPosition();			// joint pos.
					temp -= n->getGlobalPosition();			// -(end effector pos. - joint pos.)
					temp *= m->getGlobalRotAxis();			// cross product with joint rotation axis
					m_Jend.SetTriple(i, j, temp);
					temp = m->getGlobalPosition();			// joint pos.
					temp -= targetPos;		// -(target pos. - joint pos.)
					temp *= m->getGlobalRotAxis();			// cross product with joint rotation axis
					m_Jtarget.SetTriple(i, j, temp);
				}
				m = m_tree->GetParent( m );
			}
		}
		n = m_tree->GetSuccessor( n );
	}
}

// The delta theta values have been computed in dTheta array
// Apply the delta theta values to the joints
// Nothing is done about joint limits for now.
void Jacobian::UpdateThetas()
{
	// Traverse the tree to find all joints
	// Update the joint angles
	Node* n = m_tree->GetRoot();
	while ( n ) {
		if ( n->isJoint() ) {
			int i = n->getJointNum();
			double nextTheta = n->getTheta() + m_dTheta[i];
			// Check joint limits
			if (n->getMinTheta() < nextTheta && nextTheta < n->getMaxTheta())
			{
				n->addToTheta( m_dTheta[i] );
			}
		}
		n = m_tree->GetSuccessor( n );
	}

	// Update the positions and rotation axes of all joints/effectors
	m_tree->Compute();
}

void Jacobian::CalcDeltaThetas()
{
	switch (m_CurrentUpdateMode) {
		case JACOB_Undefined:
			ZeroDeltaThetas();
			break;
		case JACOB_JacobianTranspose:
			CalcDeltaThetasTranspose();
			break;
		case JACOB_PseudoInverse:
			CalcDeltaThetasPseudoinverse();
			break;
		case JACOB_DLS:
			CalcDeltaThetasDLS();
			break;
		case JACOB_SDLS:
			CalcDeltaThetasSDLS();
			break;
	}
}

void Jacobian::ZeroDeltaThetas()
{
	m_dTheta.SetZero();
}

// Find the delta theta values using inverse Jacobian method
// Uses a greedy method to decide scaling factor
void Jacobian::CalcDeltaThetasTranspose()
{
	const MatrixRmn& J = ActiveJacobian();

	J.MultiplyTranspose( m_dS, m_dTheta );

	// Scale back the dTheta values greedily
	J.Multiply ( m_dTheta, m_dT );						// dT = J * dTheta
	double alpha = Dot(m_dS,m_dT) / m_dT.NormSq();
	assert ( alpha>0.0 );
	// Also scale back to be have max angle change less than MaxAngleJtranspose
	double maxChange = m_dTheta.MaxAbs();
	double beta = MaxAngleJtranspose/maxChange;
	m_dTheta *= Min(alpha, beta);

}

void Jacobian::CalcDeltaThetasPseudoinverse()
{
	MatrixRmn& J = const_cast<MatrixRmn&>(ActiveJacobian());

	// Compute Singular Value Decomposition
	//	This an inefficient way to do Pseudoinverse, but it is convenient since we need SVD anyway

	J.ComputeSVD( m_U, m_w, m_V );

	// Next line for debugging only
    assert(J.DebugCheckSVD(m_U, m_w , m_V));

	// Calculate response vector dTheta that is the DLS solution.
	//	Delta target values are the dS values
	//  We multiply by Moore-Penrose pseudo-inverse of the J matrix
	double pseudoInverseThreshold = PseudoInverseThresholdFactor*m_w.MaxAbs();

	long diagLength = m_w.GetLength();
	double* wPtr = m_w.GetPtr();
	m_dTheta.SetZero();
	for ( long i=0; i<diagLength; i++ ) {
		double dotProdCol = m_U.DotProductColumn( m_dS, i );		// Dot product with i-th column of U
		double alpha = *(wPtr++);
		if ( fabs(alpha)>pseudoInverseThreshold ) {
			alpha = 1.0/alpha;
			MatrixRmn::AddArrayScale(m_V.GetNumRows(), m_V.GetColumnPtr(i), 1, m_dTheta.GetPtr(), 1, dotProdCol*alpha );
		}
	}

	// Scale back to not exceed maximum angle changes
	double maxChange = m_dTheta.MaxAbs();
	if ( maxChange>MaxAnglePseudoinverse ) {
		m_dTheta *= MaxAnglePseudoinverse /maxChange;
	}

}

void Jacobian::CalcDeltaThetasDLS()
{
	const MatrixRmn& J = ActiveJacobian();

	MatrixRmn::MultiplyTranspose(J, J, m_U);		// U = J * (J^T)
	m_U.AddToDiagonal( m_DampingLambdaSq );

	// Use the next four lines instead of the succeeding two lines for the DLS method with clamped error vector e.
	// CalcdTClampedFromdS();
	// VectorRn dTextra(3*nEffector);
	// U.Solve( dT, &dTextra );
	// J.MultiplyTranspose( dTextra, dTheta );

	// Use these two lines for the traditional DLS method
	m_U.Solve( m_dS, &m_dT );
	J.MultiplyTranspose( m_dT, m_dTheta );

	// Scale back to not exceed maximum angle changes
	double maxChange = m_dTheta.MaxAbs();
	if ( maxChange>MaxAngleDLS ) {
		m_dTheta *= MaxAngleDLS/maxChange;
	}
}

void Jacobian::CalcDeltaThetasDLSwithSVD()
{
	const MatrixRmn& J = ActiveJacobian();

	// Compute Singular Value Decomposition
	//	This an inefficient way to do DLS, but it is convenient since we need SVD anyway

	J.ComputeSVD( m_U, m_w, m_V );

	// Next line for debugging only
    assert(J.DebugCheckSVD(m_U, m_w , m_V));

	// Calculate response vector dTheta that is the DLS solution.
	//	Delta target values are the dS values
	//  We multiply by DLS inverse of the J matrix
	long diagLength = m_w.GetLength();
	double* wPtr = m_w.GetPtr();
	m_dTheta.SetZero();
	for ( long i=0; i<diagLength; i++ ) {
		double dotProdCol = m_U.DotProductColumn( m_dS, i );		// Dot product with i-th column of U
		double alpha = *(wPtr++);
		alpha = alpha/(Square(alpha)+m_DampingLambdaSq);
		MatrixRmn::AddArrayScale(m_V.GetNumRows(), m_V.GetColumnPtr(i), 1, m_dTheta.GetPtr(), 1, dotProdCol*alpha );
	}

	// Scale back to not exceed maximum angle changes
	double maxChange = m_dTheta.MaxAbs();
	if ( maxChange>MaxAngleDLS ) {
		m_dTheta *= MaxAngleDLS/maxChange;
	}
}


void Jacobian::CalcDeltaThetasSDLS()
{
	const MatrixRmn& J = ActiveJacobian();

	// Compute Singular Value Decomposition

	J.ComputeSVD( m_U, m_w, m_V );

	// Next line for debugging only
    assert(J.DebugCheckSVD(m_U, m_w , m_V));

	// Calculate response vector dTheta that is the SDLS solution.
	//	Delta target values are the dS values
	int nRows = J.GetNumRows();
	int numEndEffectors = m_tree->GetNumEffector();		// Equals the number of rows of J divided by three
	int nCols = J.GetNumColumns();
	m_dTheta.SetZero();

	// Calculate the norms of the 3-vectors in the Jacobian
	long i;
	const double *jx = J.GetPtr();
	double *jnx = m_Jnorms.GetPtr();
	for ( i=nCols*numEndEffectors; i>0; i-- ) {
		double accumSq = Square(*(jx++));
		accumSq += Square(*(jx++));
		accumSq += Square(*(jx++));
		*(jnx++) = sqrt(accumSq);
	}

	// Clamp the dS values
	CalcdTClampedFromdS();

	// Loop over each singular vector
	for ( i=0; i<nRows; i++ ) {

		double wiInv = m_w[i];
		if ( NearZero(wiInv,1.0e-10) ) {
			continue;
		}
		wiInv = 1.0/wiInv;

		double N = 0.0;						// N is the quasi-1-norm of the i-th column of U
		double alpha = 0.0;					// alpha is the dot product of dT and the i-th column of U

		const double *dTx = m_dT.GetPtr();
		const double *ux = m_U.GetColumnPtr(i);
		long j;
		for ( j=numEndEffectors; j>0; j-- ) {
			double tmp;
			alpha += (*ux)*(*(dTx++));
			tmp = Square( *(ux++) );
			alpha += (*ux)*(*(dTx++));
			tmp += Square(*(ux++));
			alpha += (*ux)*(*(dTx++));
			tmp += Square(*(ux++));
			N += sqrt(tmp);
		}

		// M is the quasi-1-norm of the response to angles changing according to the i-th column of V
		//		Then is multiplied by the wiInv value.
		double M = 0.0;
		double *vx = m_V.GetColumnPtr(i);
		jnx = m_Jnorms.GetPtr();
		for ( j=nCols; j>0; j-- ) {
			double accum=0.0;
			for ( long k=numEndEffectors; k>0; k-- ) {
				accum += *(jnx++);
			}
			M += fabs((*(vx++)))*accum;
		}
		M *= fabs(wiInv);

		double gamma = MaxAngleSDLS;
		if ( N<M ) {
			gamma *= N/M;				// Scale back maximum permissable joint angle
		}

		// Calculate the dTheta from pure pseudoinverse considerations
		double scale = alpha*wiInv;			// This times i-th column of V is the psuedoinverse response
		m_dPreTheta.LoadScaled( m_V.GetColumnPtr(i), scale );
		// Now rescale the dTheta values.
		double max = m_dPreTheta.MaxAbs();
		double rescale = (gamma)/(gamma+max);
		m_dTheta.AddScaled(m_dPreTheta,rescale);
		/*if ( gamma<max) {
			dTheta.AddScaled( dPreTheta, gamma/max );
		}
		else {
			dTheta += dPreTheta;
		}*/
	}

	// Scale back to not exceed maximum angle changes
	double maxChange = m_dTheta.MaxAbs();
	if ( maxChange>MaxAngleSDLS ) {
		m_dTheta *= MaxAngleSDLS/(MaxAngleSDLS+maxChange);
		//dTheta *= MaxAngleSDLS/maxChange;
	}
}

void Jacobian::CalcdTClampedFromdS()
{
	long len = m_dS.GetLength();
	long j = 0;
	for ( long i=0; i<len; i+=3, j++ ) {
		double normSq = Square(m_dS[i])+Square(m_dS[i+1])+Square(m_dS[i+2]);
		if ( normSq>Square(m_dSclamp[j]) ) {
			double factor = m_dSclamp[j]/sqrt(normSq);
			m_dT[i] = m_dS[i]*factor;
			m_dT[i+1] = m_dS[i+1]*factor;
			m_dT[i+2] = m_dS[i+2]*factor;
		}
		else {
			m_dT[i] = m_dS[i];
			m_dT[i+1] = m_dS[i+1];
			m_dT[i+2] = m_dS[i+2];
		}
	}
}

double Jacobian::UpdateErrorArray()
{
	double totalError = 0.0;

	// Traverse tree to find all end effectors
	VectorR3 temp;
	Node* n = m_tree->GetRoot();
	while ( n ) {
		if ( n->isEndeffector() ) {
			int i = n->getEffectorNum();
			const VectorR3& targetPos = n->getTarget();
			temp = targetPos;
			temp -= n->getGlobalPosition();
			double err = temp.Norm();
			m_errorArray[i] = err;
			totalError += err;
		}
		n = m_tree->GetSuccessor( n );
	}
	return totalError;
}

void Jacobian::UpdatedSClampValue()
{
	// Traverse tree to find all end effectors
	VectorR3 temp;
	Node* n = m_tree->GetRoot();
	while ( n ) {
		if ( n->isEndeffector() ) {
			int i = n->getEffectorNum();
			const VectorR3& targetPos = n->getTarget();

			// Compute the delta S value (differences from end effectors to target positions.
			// While we are at it, also update the clamping values in dSclamp;
			temp = targetPos;
			temp -= n->getGlobalPosition();
			double normSi = sqrt(Square(m_dS[i])+Square(m_dS[i+1])+Square(m_dS[i+2]));
			double changedDist = temp.Norm()-normSi;
			if ( changedDist>0.0 ) {
				m_dSclamp[i] = BaseMaxTargetDist + changedDist;
			}
			else {
				m_dSclamp[i] = BaseMaxTargetDist;
			}
		}
		n = m_tree->GetSuccessor( n );
	}
}

void Jacobian::DrawEigenVectors() const
{
	ARN_THROW_SHOULD_NOT_BE_USED_ERROR

	/*
	int i, j;
	VectorR3 tail;
	VectorR3 head;
	Node *node;

	for (i=0; i<w.GetLength(); i++) {
		if ( NearZero( w[i], 1.0e-10 ) ) {
			continue;
		}
		for (j=0; j<nEffector; j++) {
			node = tree->GetEffector(j);
			tail = node->GetS();
			U.GetTriple( j, i, &head );
			head += tail;
			glDisable(GL_LIGHTING);
			glColor3f(1.0f, 0.2f, 0.0f);
			glLineWidth(2.0);
			glBegin(GL_LINES);
			glVertex3f(tail.x, tail.y, tail.z);
			glVertex3f(head.x, head.y, tail.z);
			glEnd();
			Arrow(tail, head);
			glLineWidth(1.0);
			glEnable(GL_LIGHTING);
		}
	}
	*/
}

void Jacobian::CompareErrors( const Jacobian& j1, const Jacobian& j2, double* weightedDist1, double* weightedDist2 )
{
	const VectorRn& e1 = j1.m_errorArray;
	const VectorRn& e2 = j2.m_errorArray;
	double ret1 = 0.0;
	double ret2 = 0.0;
	int len = e1.GetLength();
	for ( long i=0; i<len; i++ ) {
		double v1 = e1[i];
		double v2 = e2[i];
		if ( v1<v2 ) {
			ret1 += v1/v2;
			ret2 += 1.0;
		}
		else if ( v1 != 0.0 ) {
			ret1 += 1.0;
			ret2 += v2/v1;
		}
		else {
			ret1 += 0.0;
			ret2 += 0.0;
		}
	}
	*weightedDist1 = ret1;
	*weightedDist2 = ret2;
}

void Jacobian::CountErrors( const Jacobian& j1, const Jacobian& j2, int* numBetter1, int* numBetter2, int* numTies )
{
	const VectorRn& e1 = j1.m_errorArray;
	const VectorRn& e2 = j2.m_errorArray;
	int b1=0, b2=0, tie=0;
	int len = e1.GetLength();
	for ( long i=0; i<len; i++ ) {
		double v1 = e1[i];
		double v2 = e2[i];
		if ( v1<v2 ) {
			b1++;
		}
		else if ( v2<v1 ) {
			b2++;
		}
		else {
			tie++;
		}
	}
	*numBetter1 = b1;
	*numBetter2 = b2;
	*numTies = tie;
}
