/**

  \file calctwobodydensitiesmomlme

  calculate two-body densities me in momentum space for 
  individual partial waves


  (c) 2012 Thomas Neff

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#include "fmd/SlaterDet.h"
#include "fmd/Projection.h"
#include "fmd/TwoBodyDensity.h"

#include "misc/utils.h"
#include "misc/physics.h"




int main(int argc, char* argv[])
{
  createinfo(argc, argv);

  // enough arguments ?

  if (argc < 2) {
    fprintf(stderr, "\nusage: %s [OPTIONS] PROJPARA [SYMMETRY:]MBSTATE [SYMMETRY:]MBSTATE"
	    "\n     -q QMAX         calculate to max separation"
	    "\n     -n NPOINTS      number of points"
	    "\n     -l LMAX         densities up to LMAX"
	    "\n     -L LAMBDAMAX    lambda max parameter\n",
	    argv[0]);
    exit(-1);
  }

  // grid in coordinate space

  int npoints = 51;
  double qmax = 3.0;
  int lmax = 4;
  int lambdamax = 6;

  /* manage command-line options */

  char c;
  while ((c = getopt(argc, argv, "q:n:l:L:")) != -1)
    switch (c) {
    case 'q':
      qmax = atof(optarg);
      break;
    case 'n':
      npoints = atoi(optarg);
      break;
    case 'l':
      lmax = atoi(optarg);
      break;
    case 'L':
      lambdamax = atoi(optarg);
      break;
    }

  char* projpar = argv[optind];
  char** mbfile = &argv[optind+1];

  SlaterDet Q[2];
  Symmetry S[2];

  int i;
  for (i=0; i<2; i++) {
    extractSymmetryfromString(&mbfile[i], &S[i]);
    if (readSlaterDetfromFile(&Q[i], mbfile[i]))
      goto cleanup;
  }

  // odd numer of nucleons ?
  int odd = Q[0].A % 2;

  // Projection parameters
  Projection P;
  initProjection(&P, odd, projpar);


  TBDensQLPara TBDP = {
    qmax : qmax,
    npoints : npoints,
    lmax : lmax,
    lambdamax : lambdamax
  };

  // initialize operator

  initOpTwoBodyDensityQL(&TBDP);

  void* tbdensme;

  // read or calculate matrix elements

  tbdensme = initprojectedMBME(&P, &OpTwoBodyDensityQL);

  if (readprojectedMBMEfromFile(mbfile[0], mbfile[1], &P,
				&OpTwoBodyDensityQL, S[0], S[1],
				tbdensme)) {
    calcprojectedMBME(&P, &OpTwoBodyDensityQL, &Q[0], &Q[1],
			  S[0], S[1], tbdensme);
    writeprojectedMBMEtoFile(mbfile[0], mbfile[1], &P, 
			     &OpTwoBodyDensityQL, S[0], S[1], 
			     tbdensme);
      }

 cleanup:

  return 0;
}
