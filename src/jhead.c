//--------------------------------------------------------------------------
// Program to pull the information out of various types of EXIF digital 
// camera files and show it in a reasonably consistent way
//
// Version 2.82
//
// Compiling under Windows:  
//   Make sure you have Microsoft's compiler on the path, then run make.bat
//
// Dec 1999 - Apr 2008
//
// by Matthias Wandel   www.sentex.net/~mwandel
//--------------------------------------------------------------------------
// This file is extracted from jhead.c of jhead project 
//   by hialan <hialan.liu@gmail.com>
//--------------------------------------------------------------------------
#include "jhead.h"

int ShowTags     = FALSE;    // Do not show raw by default.
int DumpExifMap  = FALSE;

//--------------------------------------------------------------------------
// Error exit handler
//--------------------------------------------------------------------------
void ErrFatal(char * msg)
{
//    fprintf(stderr,"Error : %s\n", msg);
//    if (CurrentFile) fprintf(stderr,"in file '%s'\n",CurrentFile);
      exit(EXIT_FAILURE);
} 

//--------------------------------------------------------------------------
// Report non fatal errors.  Now that microsoft.net modifies exif headers,
// there's corrupted ones, and there could be more in the future.
//--------------------------------------------------------------------------
void ErrNonfatal(char * msg, int a1, int a2)
{
    /*
    if (SupressNonFatalErrors) return;

    fprintf(stderr,"Nonfatal Error : ");
    if (CurrentFile) fprintf(stderr,"'%s' ",CurrentFile);
    fprintf(stderr, msg, a1, a2);
    fprintf(stderr, "\n");
    */
    return;
} 

//--------------------------------------------------------------------------
// Set file time as exif time.
//--------------------------------------------------------------------------
void FileTimeAsString(char * TimeStr)
{
    struct tm ts;
    ts = *localtime(&ImageInfo.FileDateTime);
    strftime(TimeStr, 20, "%Y:%m:%d %H:%M:%S", &ts);
}

