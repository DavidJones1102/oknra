// use AVX
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <immintrin.h>
/* Include PAPI */
#include <papi.h> 

/* PAPI macro helpers definitions */
#define NUM_EVENT 2
#define THRESHOLD 100000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define IDX(i, j, n) (((j)+ (i)*(n)))

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

int ge(double * A, int SIZE)
{
  register int i,j,k;
  register double diag, mul;
  register __m256d mm_multiplier;
  register __m256d tmp0, tmp1, tmp2, tmp3;
  for (k = 0; k < SIZE; k++) {
    diag = A[IDX(k,k,SIZE)];
    for (i = k+1; i < SIZE; i++) { 
      mul = (A[IDX(i,k,SIZE)]/diag);
      mm_multiplier[0] = mul;
      mm_multiplier[1] = mul;
      mm_multiplier[2] = mul;
      mm_multiplier[3] = mul;
      for (j = k+1; j < SIZE; ) { 
        if (j < max(SIZE-8, 0)){
          tmp0 = _mm256_loadu_pd(A+IDX(i,j, SIZE));
          tmp1 = _mm256_loadu_pd(A+IDX(k,j, SIZE));

          tmp2 = _mm256_loadu_pd(A+IDX(i,j+4, SIZE));
          tmp3 = _mm256_loadu_pd(A+IDX(k,j+4, SIZE));
          
          tmp1 = _mm256_mul_pd( tmp1 , mm_multiplier );
          tmp3 = _mm256_mul_pd( tmp3 , mm_multiplier );

          tmp0 = _mm256_sub_pd( tmp0 , tmp1 ) ;
          tmp2 = _mm256_sub_pd( tmp2 , tmp3 ) ;

          _mm256_storeu_pd(A+IDX(i,j,SIZE), tmp0);
          _mm256_storeu_pd(A+IDX(i,j+4,SIZE), tmp2);

          j+=8;
        }else{
         A[IDX(i,j,SIZE)] = A[IDX(i,j,SIZE)]-A[IDX(k,j,SIZE)]*mul;
         j++;
        }
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
    
  int i,j,iret;
  double dtime;
  double * A;
  int SIZE = atoi(argv[1]);
  int retval;
 
  A = malloc(SIZE * SIZE * sizeof(double));

  srand(1);
  for (i = 0; i < SIZE; i++) { 
    for (j = 0; j < SIZE; j++) { 
      A[IDX(i,j,SIZE)] = rand();
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
      check = check + A[IDX(i,j,SIZE)];
    }
  }

  // Size, time, checksum, total instructions executed, L2 hits
  printf("%d,%le,%lld,%lld,%lf\n", SIZE, dtime, values[0], values[1], check);
  // printf("%d,%le,%lf\n", SIZE, dtime, check);
  fflush( stdout );


  return iret;
}


