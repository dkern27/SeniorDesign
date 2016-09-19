#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <sys/stat.h>

#include "timestamp.h"
#include "events.h"

char * Options = "i:o:a:v?" ;
enum { FIRST_OPT, LAST_OPT, NEVT_OPT } ;

static struct option longopts[] = {
  {"first", required_argument, NULL, FIRST_OPT},
  {"last", required_argument, NULL, LAST_OPT},
  {"nevt", required_argument, NULL, NEVT_OPT},
  {NULL, 0, NULL, 0}
} ;
static int Verbose = 0 ;
static int FirstEvt = 0, LastEvt = 0 ;

#define DEFAULT_OUTPUT "muons.txt"

static char * InputName = NULL ;
static FILE * InFile = NULL ;
static char * OutputName = NULL ;
static FILE * OutFile = NULL ;
static int NbSelected = 0 ;
static int TotalMuons = 0 ;
static char * HowOpen = "w" ;

static double TimeStamp ;

static void Help()
{
  puts( "anamu [<options>] -i <path>" ) ;
  puts( "Select muon buffers from a muon file (64 muons per buffer). " ) ;
  puts( "Generates ascii file(s) containing the muon samples (63 samples" ) ;
  puts( " per muon)" ) ;
  puts( "Options" ) ;
  puts( " -i <path>       : input file (binary). Mandatory" ) ;
  puts( " -o <path>       : output file (ascii)." ) ;
  puts( " -a <path>       : Same as '-o' but data are appended to file" ) ;
  puts( "    --nevt=<nn>  : keep only <nevts> buffers." ) ;
  puts( "    --first=<nn> : First muon to keep" ) ;
  puts( "    --last=<nn>  : Last ..." ) ;
  puts( " -v              : Verbose" ) ;
  exit( 1 ) ;
}

static void HandleOptions( int argc, char ** argv )
{
  int opt ;

  while( (opt = getopt_long( argc, argv, Options, longopts, NULL ) ) != EOF ) {
    switch( opt ) {
    case FIRST_OPT:
      sscanf( optarg, "%d", &FirstEvt ) ;
      break ;
    case LAST_OPT:
      sscanf( optarg, "%d", &LastEvt ) ;
      break ;
    case NEVT_OPT:
      sscanf( optarg, "%d", &NbSelected ) ;
      break ;
    case 'i':
      InputName = malloc( strlen(optarg) + 1 ) ;
      strcpy( InputName, optarg ) ;
      break ;
    case 'o':
      OutputName = malloc( strlen(optarg) + 1 ) ;
      strcpy( OutputName, optarg ) ;
      HowOpen = "w" ;
      break ;
    case 'a':
      OutputName = malloc( strlen(optarg) + 1 ) ;
      strcpy( OutputName, optarg ) ;
      HowOpen = "a+" ;
      break ;
    case 'v':
      Verbose++ ;
      break ;
    case '?':
    default:
      //( "Option: %d\n", opt ) ;
      Help() ;
    }
  }
}

static void check_muons( unsigned int * data, int size )
{
  int i, nmuons = 0, idx = 0 ;
  unsigned int the_date ;
  int start = 0 ;

  for( i = 0 ; i < (size/sizeof( unsigned int)) ; i++, data++ ) {
    if ( (*data & 0x80000000 ) != 0 ) {
      /* Start of a burst */
      nmuons++ ;
      the_date = *data & 0x3FFFFFFF ;
      start = 1 ;
      idx = 0 ;
    }
    if ( OutFile != NULL ) {
      if ( start == 1 ) start = 0 ;
      else {
	fprintf( OutFile, "%d %d %d %d [%x %u] %.9lf\n",
		 idx,
		 *data & 0x3FF, (*data >> 10)&0x3FF, (*data >> 20)&0x3FF,
		 *data, the_date, TimeStamp ) ;
	idx++ ;
      }
    }
  }
  TotalMuons += nmuons ;
  if ( Verbose ) printf( "  Nb of Muons: %d\n", nmuons ) ;
}

static void ShowMu()
{
  MUON_EVENT pmuon ;
  int count = 0, good = 0 ;

  /* Read one-time and bufsize
     then read the rest */
  ONE_TIME date ;
  int bufsize ;

  if ( NbSelected == 0 ) printf( "All buffers from the file\n" ) ;
  else printf( "Nb of buffersselected: %d\n", NbSelected ) ;

  while ( fread( &date, 1, sizeof( ONE_TIME ), InFile ) ==
	  sizeof( ONE_TIME ) ) {
    /* Read bufsize */
    fread( &bufsize, 1, sizeof( bufsize ), InFile ) ;
    if ( Verbose) printf( "Buffer size: %d\n", bufsize ) ;
    pmuon.date = date ;
    pmuon.bufsize = bufsize ;
    /* Read the rest */
    fread( pmuon.data, 1, bufsize, InFile ) ;
    if ( count < FirstEvt ) {
      count++ ;
      continue ;
    }
    TimeStamp = pmuon.date.second + ((double)pmuon.date.nano/100000000.) ;
    if ( Verbose > 1 ) {
      printf( "*** Muon Buffer %d\n", count ) ;
      printf( " Timestamp (GPS): %.9lf\n", TimeStamp ) ;
      printf( " Buffer Size: %d\n", pmuon.bufsize ) ;
    }
    /* Check Nb of Muons */
    check_muons( pmuon.data, bufsize ) ;

    count++ ;
    good++ ;
    if ( LastEvt != 0 && count == LastEvt ) break ;
    if ( Verbose ) printf( "NbSelected %d, good: %d\n", NbSelected, good ) ;
    if ( NbSelected != 0 && good == NbSelected ) break ;
  }

  printf( "Finished with %d muon buffers read and % d muons\n",
	  count, TotalMuons ) ;

}

int main( int argc, char ** argv )
{
  FILE * fin ;
  int evid ;

  HandleOptions( argc, argv ) ;

  if ( InputName == NULL ) Help() ;
  if ( Verbose ) printf( "In: %s\n", InputName ) ;

  InFile = fopen( InputName, "r" ) ;
  if ( OutputName == NULL ) OutputName = DEFAULT_OUTPUT ;
  OutFile = fopen( OutputName, HowOpen ) ;
  printf( "OutputName: '%s'\n", OutputName ) ;

  /* Do the job */
  ShowMu() ;

  fclose( InFile ) ;
  if ( OutFile != NULL ) fclose( OutFile ) ;

  return 0 ;
}
