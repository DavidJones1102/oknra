// don't calculate multiplier each time in loop
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
/* Include PAPI */
// #include <papi.h> 

/* PAPI macro helpers definitions */
#define NUM_EVENT 2
#define THRESHOLD 100000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }
static double gtod_ref_time_sec = 0.0;
// double CORRECT_CHECK = -4.605781e+16
/* Adapted from the bl2_clock() routine in the BLIS library */

double dclock()
{
  double the_time, norm_sec;
  struct timeval tv;
  gettimeofday( &tv, NULL );
  if ( gtod_ref_time_sec == 0.0 )
    gtod_ref_time_sec = ( double ) tv.tv_sec;
  norm_sec = ( double ) tv.tv_sec - gtod_ref_time_sec;
  the_time = norm_sec + tv.tv_usec * 1.0e-6;
  return the_time;
}

int ge(double ** A, int SIZE)
{
  register int i,j,k;
  register double diag, mul;
  for (k = 0; k < SIZE; k++) {
    diag = A[k][k]; 
    for (i = k+1; i < SIZE; i++) { 
    mul = (A[i][k]/diag);
      for (j = k+1; j < SIZE; j++) { 
         A[i][j] = A[i][j]-A[k][j]*mul;
      } 
    }
  }
  return 0;
}

int main( int argc, const char* argv[] )
{
  /* PAPI counters variables */
  int EventSet = PAPI_NULL;
  /*must be initialized to PAPI_NULL before calling PAPI_create_event*/

  int event_codes[NUM_EVENT]={PAPI_TOT_INS,PAPI_L2_ICH}; 
  char errstring[PAPI_MAX_STR_LEN];
  long long values[NUM_EVENT];
  int retval;
    
  int i,j,iret;
  double dtime;
  double ** A;
  double * A_;
  int SIZE = atoi(argv[1]);
 
  A_ = (double*) malloc(SIZE*SIZE*sizeof(double));
  A = (double**) malloc(SIZE*sizeof(double*));
  for (i = 0; i < SIZE; i++) {
    A[i] = A_ + i*SIZE;
  }
  srand(1);
  for (i = 0; i < SIZE; i++) { 
    for (j = 0; j < SIZE; j++) { 
      A[i][j] = rand();
    }
  }

  /* initializing library */
  if((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT ) {
      fprintf(stderr, "Error: %s\n", errstring);
      exit(1);            
  }
  /* Creating event set   */
  if ((retval=PAPI_create_eventset(&EventSet)) != PAPI_OK){
      ERROR_RETURN(retval);
  }
  /* Add the array of events PAPI_TOT_INS and PAPI_TOT_CYC to the eventset*/
  if ((retval=PAPI_add_events(EventSet, event_codes, NUM_EVENT)) != PAPI_OK){
      ERROR_RETURN(retval);
  }
  /* Start counting */
  if ( (retval=PAPI_start(EventSet)) != PAPI_OK){
      ERROR_RETURN(retval);
  }
  dtime = dclock();
  iret = ge(A, SIZE);
  dtime = dclock()-dtime;
  /* Stop counting, this reads from the counter as well as stop it. */
  if ( (retval=PAPI_stop(EventSet,values)) != PAPI_OK){
      ERROR_RETURN(retval);
  }
 
  
  /* calculate checksum */
  double check=0.0;
  for (i = 0; i < SIZE; i++) {
    for (j = 0; j < SIZE; j++) {
      check = check + A[i][j];
    }
  }

  // Size, time, checksum, total instructions executed, L2 hits
  printf("%d,%le,%lld,%lld,%lf\n", SIZE, dtime, values[0], values[1], check);
  // printf("%d,%le,%lf\n", SIZE, dtime, check);
  fflush( stdout );


  return iret;
}


