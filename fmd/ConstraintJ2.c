/**

  \file ConstraintJ2.c

  Constrain expectation value of J2


  (c) 2003 Thomas Neff

*/

#include <math.h>
#include <complex.h>

#include "Gaussian.h"
#include "gradGaussian.h"
#include "SlaterDet.h"
#include "gradSlaterDet.h"
#include "CenterofMass.h"
#include "gradCMSlaterDet.h"

#include "Constraint.h"
#include "ConstraintJ2.h"

#include "numerics/cmath.h"
#include "misc/physics.h"


Constraint ConstraintJ2 = {
  val : 0.0,
  label : "J2",
  me : calcConstraintJ2,
  gradme : calcgradConstraintJ2,
  output : outputConstraintJ2
};


Constraintod ConstraintJ2od = {
  val : 0.0,
  label : "J2",
  dim : 1,
  gradneedsme : 0,
  me : calcConstraintJ2od,
  gradme : calcgradConstraintJ2od,
  output : outputConstraintJ2
};


typedef struct {
  double Xcm[3];
  double Vcm[3];
} CMpara;


// matrix elements similar as in AngularMomentum.c
// J2 is calculated relativ to the relativ to center of mass 

static void ob_j2(const CMpara* par,
		  const Gaussian* G1, const Gaussian* G2, 
		  const GaussianAux* X, complex double* j2)
{
  double* Xcm = par->Xcm;
  double* Vcm = par->Vcm;
  complex double l2, s2, ls;
  complex double beta;
  complex double rho2cm = 0.0, pi2cm = 0.0, rhopicm = 0.0;
  complex double rhoxpicm[3];
  int i;
  
  for (i=0; i<3; i++)
    rho2cm += csqr(X->rho[i]-Xcm[i]);
  for (i=0; i<3; i++)
    pi2cm += csqr(X->pi[i]-mass(G1->xi)*Vcm[i]);
  for (i=0; i<3; i++)
    rhopicm += (X->rho[i]-Xcm[i])*(X->pi[i]-mass(G1->xi)*Vcm[i]);
  for (i=0; i<3; i++)
    rhoxpicm[i] = 
      (X->rho[(i+1)%3]-Xcm[(i+1)%3])*(X->pi[(i+2)%3]-mass(G1->xi)*Vcm[(i+2)%3]) -
      (X->rho[(i+2)%3]-Xcm[(i+2)%3])*(X->pi[(i+1)%3]-mass(G1->xi)*Vcm[(i+1)%3]);

  beta = I*X->lambda*(conj(G1->a)-G2->a);

  l2 = (2*X->lambda*rho2cm + 2*X->alpha*pi2cm - 2*beta*rhopicm +
	 rho2cm* pi2cm - csqr(rhopicm))* X->Q;
  s2 = 0.75*X->Q;
  ls = 0.5*cvec3mult(rhoxpicm, X->sig)* X->T* X->R;
  
  *j2 += l2 + s2 + 2*ls;
}


static void tb_j2(const CMpara* par,
		  const Gaussian* G1, const Gaussian* G2, 
		  const Gaussian* G3, const Gaussian* G4, 
		  const GaussianAux* X13, const GaussianAux* X24, 
		  complex double* j2)
{	
  double* Xcm = par->Xcm;
  double* Vcm = par->Vcm;
  complex double l2, s2, ls;
  complex double rhoxpicm13[3], rhoxpicm24[3];
  int i;

  for (i=0; i<3; i++)
    rhoxpicm13[i] = 
      (X13->rho[(i+1)%3]-Xcm[(i+1)%3])*(X13->pi[(i+2)%3]-mass(G1->xi)*Vcm[(i+2)%3]) -
      (X13->rho[(i+2)%3]-Xcm[(i+2)%3])*(X13->pi[(i+1)%3]-mass(G1->xi)*Vcm[(i+1)%3]);
  for (i=0; i<3; i++)
    rhoxpicm24[i] = 
      (X24->rho[(i+1)%3]-Xcm[(i+1)%3])*(X24->pi[(i+2)%3]-mass(G2->xi)*Vcm[(i+2)%3]) -
      (X24->rho[(i+2)%3]-Xcm[(i+2)%3])*(X24->pi[(i+1)%3]-mass(G2->xi)*Vcm[(i+1)%3]);

  l2 = cvec3mult(rhoxpicm13, rhoxpicm24)* X13->Q* X24->Q;
  s2 = 0.25*cvec3mult(X13->sig, X24->sig)* X13->T* X24->T* X13->R* X24->R;
  ls = 0.25*(cvec3mult(rhoxpicm13, X24->sig)*X13->S+
	     cvec3mult(X13->sig, rhoxpicm24)*X24->S)*
       X13->T* X24->T* X13->R* X24->R;

  *j2 += 2*(l2 + s2 + 2*ls);
}


static void gob_j2(const CMpara* par,
		   const Gaussian* G1, const Gaussian* G2, 
		   const GaussianAux* X, const gradGaussianAux* dX,
		   complex double* j2, gradGaussian* dj2)
{
  double* Xcm = par->Xcm;
  double* Vcm = par->Vcm;
  complex double l2m, s2m, lsm;
  complex double beta, dbeta;
  complex double rhocm[3], picm[3];
  complex double rhocm2, picm2, rhopicm;
  complex double rhoxpicm[3];
  gradScalar dl2m, drhocm2, dpicm2, drhopicm;
  gradGaussian dlsm;
  int i;

  beta = I*X->lambda*(conj(G1->a)-G2->a);
  dbeta = I*dX->dlambda*(conj(G1->a)-G2->a) + I*X->lambda;

  for (i=0; i<3; i++)
    rhocm[i] = X->rho[i]-Xcm[i];
  rhocm2 = cvec3mult(rhocm, rhocm);
  for (i=0; i<3; i++)
    picm[i] = X->pi[i]-mass(G1->xi)*Vcm[i];
  picm2 = cvec3mult(picm, picm);
  rhopicm = cvec3mult(rhocm, picm);
  cvec3cross(rhocm, picm, rhoxpicm);

  l2m = 2*X->lambda*rhocm2 + 2*X->alpha*picm2 - 2*beta*rhopicm 
	 + rhocm2*picm2 - csqr(rhopicm);
  s2m = 0.75*X->S;
  lsm = 0.5*cvec3mult(rhoxpicm, X->sig);
  
  *j2 += X->T*(l2m*X->S + 2*lsm + s2m)*X->R;
  
  drhocm2.a = 2*cvec3mult(rhocm, dX->drho.a);
  for (i=0; i<3; i++)
    drhocm2.b[i] = 2*rhocm[i]*dX->drho.b;

  dpicm2.a = 2*cvec3mult(picm, dX->dpi.a);
  for (i=0; i<3; i++)
    dpicm2.b[i] = 2*picm[i]*dX->dpi.b;
    
  drhopicm.a = cvec3mult(dX->drho.a, picm) + cvec3mult(rhocm, dX->dpi.a);
  for (i=0; i<3; i++)
    drhopicm.b[i] = dX->drho.b*picm[i] + rhocm[i]*dX->dpi.b;

  dl2m.a = 2*dX->dlambda*rhocm2 + 2*X->lambda*drhocm2.a +
    2*dX->dalpha*picm2 + 2*X->alpha*dpicm2.a -
    2*dbeta*rhopicm - 2*beta*drhopicm.a +
    drhocm2.a*picm2 + rhocm2*dpicm2.a -
    2*rhopicm*drhopicm.a;
  for (i=0; i<3; i++)
    dl2m.b[i] = 2*X->lambda*drhocm2.b[i] + 2*X->alpha*dpicm2.b[i] - 
      2*beta*drhopicm.b[i] +
      drhocm2.b[i]*picm2 + rhocm2*dpicm2.b[i] - 2*rhopicm*drhopicm.b[i];
    
  for (i=0; i<2; i++)
    dlsm.chi[i] = 0.5*cvec3mult(rhoxpicm, dX->dsig.chi[i]);
  dlsm.a = 0.5*(cvec3spat(dX->drho.a, picm, X->sig)+
		cvec3spat(rhocm, dX->dpi.a, X->sig));
  dlsm.b[0] = 0.5*((dX->drho.b*picm[1]-rhocm[1]*dX->dpi.b)*X->sig[2] - 
		   (dX->drho.b*picm[2]-rhocm[2]*dX->dpi.b)*X->sig[1]);
  dlsm.b[1] = 0.5*((dX->drho.b*picm[2]-rhocm[2]*dX->dpi.b)*X->sig[0] - 
		   (dX->drho.b*picm[0]-rhocm[0]*dX->dpi.b)*X->sig[2]);
  dlsm.b[2] = 0.5*((dX->drho.b*picm[0]-rhocm[0]*dX->dpi.b)*X->sig[1] - 
		   (dX->drho.b*picm[1]-rhocm[1]*dX->dpi.b)*X->sig[0]);

  for (i=0; i<2; i++)
    dj2->chi[i] += X->T* (l2m*dX->dS.chi[i]+2*dlsm.chi[i]+0.75*dX->dS.chi[i])* X->R;

  dj2->a += X->T* (dl2m.a* X->S*X->R +
		   2*dlsm.a* X->R +
		   (l2m*X->S + 2*lsm + s2m)*dX->dR.a);

  for (i=0; i<3; i++)
    dj2->b[i] += X->T* (dl2m.b[i]* X->S*X->R +
			2*dlsm.b[i]* X->R +
			(l2m*X->S + 2*lsm + s2m)*dX->dR.b[i]);
}


static void gtb_j2(const CMpara* par,
		   const Gaussian* G1, const Gaussian* G2, 
		   const Gaussian* G3, const Gaussian* G4, 
		   const GaussianAux* X13, const GaussianAux* X24,
		   const gradGaussianAux* dX13,
		   complex double* j2, gradGaussian* dj2)
{
  double* Xcm = par->Xcm;
  double* Vcm = par->Vcm;
  complex double rho13cm[3], rho24cm[3], pi13cm[3], pi24cm[3];
  complex double rhoxpi13cm[3], rhoxpi24cm[3];	
  complex double l2m, s2m, lsm;
  gradScalar dl2m;
  gradGaussian dlsm;
  int i;

  for (i=0; i<3; i++)
    rho13cm[i] = X13->rho[i]-Xcm[i];
  for (i=0; i<3; i++)
    pi13cm[i] = X13->pi[i]-mass(G1->xi)*Vcm[i];
  cvec3cross(rho13cm, pi13cm, rhoxpi13cm);

  for (i=0; i<3; i++)
    rho24cm[i] = X24->rho[i]-Xcm[i];
  for (i=0; i<3; i++)
    pi24cm[i] = X24->pi[i]-mass(G2->xi)*Vcm[i];
  cvec3cross(rho24cm, pi24cm, rhoxpi24cm);
  
  l2m = cvec3mult(rhoxpi13cm, rhoxpi24cm);
  s2m = 0.25*cvec3mult(X13->sig, X24->sig);
  lsm = 0.25*(cvec3mult(rhoxpi13cm, X24->sig)*X13->S+
	      cvec3mult(X13->sig, rhoxpi24cm)*X24->S);
  
  *j2 += 2* X13->T*X24->T* (l2m*X13->S*X24->S + 2*lsm + s2m) *X13->R*X24->R;
  
  dl2m.a = 
    cvec3mult(dX13->drho.a, rho24cm)*cvec3mult(pi13cm, pi24cm) +
    cvec3mult(rho13cm, rho24cm)*cvec3mult(dX13->dpi.a, pi24cm) -
    cvec3mult(dX13->drho.a, pi24cm)*cvec3mult(pi13cm, rho24cm) -
    cvec3mult(rho13cm, pi24cm)*cvec3mult(dX13->dpi.a, rho24cm);
  for (i=0; i<3; i++)
    dl2m.b[i] =
      dX13->drho.b*rho24cm[i]*cvec3mult(pi13cm, pi24cm) +
      cvec3mult(rho13cm, rho24cm)*dX13->dpi.b*pi24cm[i] -
      dX13->drho.b*pi24cm[i]*cvec3mult(pi13cm, rho24cm) -
      cvec3mult(rho13cm, pi24cm)*dX13->dpi.b*rho24cm[i];
      
  for (i=0; i<2; i++)
    dlsm.chi[i] = 0.25*( 
      cvec3mult(rhoxpi13cm, X24->sig)*dX13->dS.chi[i]+
      cvec3mult(dX13->dsig.chi[i], rhoxpi24cm)*X24->S);
  dlsm.a =
    0.25*cvec3spat(dX13->drho.a, pi13cm, X24->sig)*X13->S+
    0.25*cvec3spat(rho13cm, dX13->dpi.a, X24->sig)*X13->S;

  for (i=0; i<3; i++)
    dlsm.b[i] = 0.25*
      ((dX13->drho.b*pi13cm[(i+1)%3]-rho13cm[(i+1)%3]*dX13->dpi.b)*
       X13->S* X24->sig[(i+2)%3] - 
       (dX13->drho.b*pi13cm[(i+2)%3]-rho13cm[(i+2)%3]*dX13->dpi.b)*
       X13->S* X24->sig[(i+1)%3]);	
  
  for (i=0; i<2; i++)
    dj2->chi[i] += 2* X13->T*X24->T*
      (l2m*dX13->dS.chi[i]*X24->S +
       2*dlsm.chi[i] +
       0.25*cvec3mult(dX13->dsig.chi[i], X24->sig))* X13->R* X24->R;

  dj2->a += 2* X13->T*X24->T*
    (dl2m.a* X13->S*X24->S* X13->R*X24->R +
     2*dlsm.a* X13->R*X24->R +
     (l2m*X13->S*X24->S + 2*lsm + s2m)*dX13->dR.a*X24->R);

  for (i=0; i<3; i++)
    dj2->b[i] += 2* X13->T*X24->T*
      (dl2m.b[i]* X13->S*X24->S* X13->R*X24->R +
       2*dlsm.b[i]* X13->R*X24->R +
       (l2m*X13->S*X24->S + 2*lsm + s2m)*dX13->dR.b[i]*X24->R);  
}


void gcmob_j2(const CMpara* par,
	      const Gaussian* G1, const Gaussian* G2,
	      const GaussianAux* X,
	      gradCM* dCMj2)
{
  double* Xcm = par->Xcm;
  double* Vcm = par->Vcm;
  complex double beta;
  complex double rhocm[3], picm[3];
  complex double rhocm2, picm2, rhopicm;
  complex double rhoxpicm[3];
  gradCM dl2m, drhocm2, dpicm2, drhopicm;
  gradCM dlsm;
  int i;

  beta = I*X->lambda*(conj(G1->a)-G2->a);

  for (i=0; i<3; i++)
    rhocm[i] = X->rho[i]-Xcm[i];
  rhocm2 = cvec3mult(rhocm, rhocm);
  for (i=0; i<3; i++)
    picm[i] = X->pi[i]-mass(G1->xi)*Vcm[i];
  picm2 = cvec3mult(picm, picm);
  rhopicm = cvec3mult(rhocm, picm);
  cvec3cross(rhocm, picm, rhoxpicm);

  for (i=0; i<3; i++)
    drhocm2.X[i] = -2*rhocm[i];
  for (i=0; i<3; i++)
    dpicm2.V[i] = -2*mass(G1->xi)*picm[i];
  for (i=0; i<3; i++)
    drhopicm.X[i] = -picm[i];
  for (i=0; i<3; i++)
    drhopicm.V[i] = -mass(G1->xi)*rhocm[i];

  for (i=0; i<3; i++)
    dl2m.X[i] = 2*X->lambda*drhocm2.X[i] - 2*beta*drhopicm.X[i] +
      drhocm2.X[i]*picm2 - 2*rhopicm*drhopicm.X[i];
  for (i=0; i<3; i++)
    dl2m.V[i] = 2*X->alpha*dpicm2.V[i] - 2*beta*drhopicm.V[i] +
      rhocm2*dpicm2.V[i] - 2*rhopicm*drhopicm.V[i];

  for (i=0; i<3; i++)
    dlsm.X[i] = -0.5*(picm[(i+1)%3]*X->sig[(i+2)%3] - 
		      picm[(i+2)%3]*X->sig[(i+1)%3]);
  for (i=0; i<3; i++)
    dlsm.V[i] = 0.5*mass(G1->xi)*(rhocm[(i+1)%3]*X->sig[(i+2)%3] - 
				  rhocm[(i+2)%3]*X->sig[(i+1)%3]);

  for (i=0; i<3; i++)
    dCMj2->X[i] += X->T* (dl2m.X[i]* X->S*X->R + 2*dlsm.X[i]* X->R);
  for (i=0; i<3; i++)
    dCMj2->V[i] += X->T* (dl2m.V[i]* X->S*X->R + 2*dlsm.V[i]* X->R);
} 


void gcmtb_j2(const CMpara* par,
	      const Gaussian* G1, const Gaussian* G2,
	      const Gaussian* G3, const Gaussian* G4,
	      const GaussianAux* X13, const GaussianAux* X24,
	      gradCM* dCMj2)
{
  double* Xcm = par->Xcm;
  double* Vcm = par->Vcm;
  complex double rho13cm[3], rho24cm[3], pi13cm[3], pi24cm[3];
  gradCM dl2m, dlsm;
  int i;

  for (i=0; i<3; i++)
    rho13cm[i] = X13->rho[i]-Xcm[i];
  for (i=0; i<3; i++)
    pi13cm[i] = X13->pi[i]-mass(G1->xi)*Vcm[i];

  for (i=0; i<3; i++)
    rho24cm[i] = X24->rho[i]-Xcm[i];
  for (i=0; i<3; i++)
    pi24cm[i] = X24->pi[i]-mass(G2->xi)*Vcm[i];

  for (i=0; i<3; i++)
    dl2m.X[i] = - (rho24cm[i]+rho13cm[i])*cvec3mult(pi13cm, pi24cm) +
      pi24cm[i]*cvec3mult(pi13cm, rho24cm) +
      cvec3mult(rho13cm, pi24cm)*pi13cm[i];
  for (i=0; i<3; i++)
    dl2m.V[i] = - cvec3mult(rho13cm, rho24cm)*
      (mass(G1->xi)*pi24cm[i]+mass(G2->xi)*pi13cm[i]) +
      mass(G2->xi)*rho13cm[i]*cvec3mult(pi13cm, rho24cm) +
      cvec3mult(rho13cm, pi24cm)*mass(G1->xi)*rho24cm[i];
      
  for (i=0; i<3; i++)
    dlsm.X[i] = 
      -0.25*(pi13cm[(i+1)%3]*X13->S*X24->sig[(i+2)%3] -
	     pi13cm[(i+2)%3]*X13->S*X24->sig[(i+1)%3])
      -0.25*(X13->sig[(i+2)%3]*pi24cm[(i+1)%3]*X24->S -
	     X13->sig[(i+1)%3]*pi24cm[(i+2)%3]*X24->S);
  for (i=0; i<3; i++)
    dlsm.V[i] = 
      0.25*mass(G1->xi)*(rho13cm[(i+1)%3]*X13->S*X24->sig[(i+2)%3] -
			 rho13cm[(i+2)%3]*X13->S*X24->sig[(i+1)%3]) +
      0.25*mass(G2->xi)*(X13->sig[(i+2)%3]*rho24cm[(i+1)%3]*X24->S -
			 X13->sig[(i+1)%3]*rho24cm[(i+2)%3]*X24->S);

  for (i=0; i<3; i++)
    dCMj2->X[i] += 2*X13->T*X24->T* 
      (dl2m.X[i]* X13->S*X24->S + 2*dlsm.X[i])* X13->R* X24->R;
  for (i=0; i<3; i++)
    dCMj2->V[i] += 2*X13->T*X24->T* 
      (dl2m.V[i]* X13->S*X24->S + 2*dlsm.V[i])* X13->R* X24->R;
}


void calcConstraintJ2(const SlaterDet* Q, const SlaterDetAux* X, 
		      double* j2)
{
  CMpara para;
  double j2one, j2two;

  calcCMPosition(Q, X, para.Xcm);
  calcCMVelocity(Q, X, para.Vcm);
  
  OneBodyOperator op_ob_j2 = {dim: 1, opt: 1, par: &para, me: ob_j2};
  TwoBodyOperator op_tb_j2 = {dim: 1, opt: 1, par: &para, me: tb_j2};

  calcSlaterDetOBME(Q, X, &op_ob_j2, &j2one);
  calcSlaterDetTBME(Q, X, &op_tb_j2, &j2two);

  *j2 = j2one + j2two;
}



void calcgradConstraintJ2(const SlaterDet* Q, const SlaterDetAux* X, 
			  const gradSlaterDetAux* dX,
			  gradSlaterDet* dj2)
{
  CMpara para;
  double j2;

  calcCMPosition(Q, X, para.Xcm);
  calcCMVelocity(Q, X, para.Vcm);

  gradOneBodyOperator gop_ob_j2 = {opt: 1, par: &para, me: gob_j2};
  gradTwoBodyOperator gop_tb_j2 = {opt: 1, par: &para, me: gtb_j2};

  calcgradSlaterDetOBME(Q, X, dX, &gop_ob_j2, dj2);
  calcgradSlaterDetTBME(Q, X, dX, &gop_tb_j2, dj2);

  j2 = dj2->val;

  OneBodyOperator gcmop_ob_j2 = {dim: 6, opt: 1, par: &para, me: gcmob_j2};
  TwoBodyOperator gcmop_tb_j2 = {dim: 6, opt: 1, par: &para, me: gcmtb_j2};

  calcgradCMSlaterDetOBME(Q, X, dX, &gcmop_ob_j2, dj2);
  calcgradCMSlaterDetTBME(Q, X, dX, &gcmop_tb_j2, dj2);

  dj2->val = j2;
}


// gradient with respect to center of mass vanishes (?)

void calcConstraintJ2od(const SlaterDet* Q, const SlaterDet* Qp, 
			const SlaterDetAux* X, 
			complex double* j2)
{
  CMpara para = { { 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0} };
  complex double j2one, j2two;

  OneBodyOperator op_ob_j2 = {dim: 1, opt: 1, par: &para, me: ob_j2};
  TwoBodyOperator op_tb_j2 = {dim: 1, opt: 1, par: &para, me: tb_j2};

  calcSlaterDetOBMEod(Q, Qp, X, &op_ob_j2, &j2one);
  calcSlaterDetTBMEod(Q, Qp, X, &op_tb_j2, &j2two);

  *j2 = j2one + j2two;
}


void calcgradConstraintJ2od(const SlaterDet* Q, const SlaterDet* Qp, 	
			    const SlaterDetAux* X, 
			    const gradSlaterDetAux* dX,
			    const double* dummy,
			    gradSlaterDet* dj2)
{
  CMpara para = { {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0} };

  gradOneBodyOperator gop_ob_j2 = {opt: 1, par: &para, me: gob_j2};
  gradTwoBodyOperator gop_tb_j2 = {opt: 1, par: &para, me: gtb_j2};

  calcgradSlaterDetOBMEod(Q, Qp, X, dX, &gop_ob_j2, dj2);
  calcgradSlaterDetTBMEod(Q, Qp, X, dX, &gop_tb_j2, dj2);
}


double outputConstraintJ2(double val)
{
  return (val);
}
