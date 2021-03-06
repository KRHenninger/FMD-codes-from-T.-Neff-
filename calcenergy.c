/**

  \file calcenergy.c

  calculate energies and other observables for a Slater determinant


  (c) 2003 Thomas Neff

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <complex.h>

#include "fmd/SlaterDet.h"
#include "fmd/Observables.h"

#include "misc/utils.h"


int main(int argc, char *argv[])
{
  createinfo(argc, argv);
  
  int c;
  int cm=1;
  int deformation=0;

  if (argc < 3) {
    fprintf(stderr, "\nusage: %s interaction slaterdetfile\n",
	    argv[0]);
    exit(-1);
  }
  
  char* interactionfile = argv[optind];
  char* slaterdetfile = argv[optind+1];

  Interaction Int;
  readInteractionfromFile(&Int, interactionfile);
  Int.cm = cm;

  SlaterDet Q;
  readSlaterDetfromFile(&Q, slaterdetfile);

  Observables Obs;

  calcObservables(&Int, &Q, &Obs);

  fprintinfo(stdout);
  fprintObservables(stdout, &Int, &Q, &Obs);

  return 0;
}
