//--------------------------------------------------------------------------
// Program to pull the information out of various types of EXIF digital 
// camera files and show it in a reasonably consistent way
//
// This module parses the very complicated exif structures.
//
// Matthias Wandel
//--------------------------------------------------------------------------
// Modified by hialan Liu <hialan.liu@gamil.com>
//--------------------------------------------------------------------------

#ifndef __EXIF_H__
#define __EXIF_H__

void ShowImageInfo(int ShowFileInfo);

int Get32s(void * Long);
void Put32u(void * Value, unsigned PutValue);
unsigned Get32u(void * Long);
void PrintFormatNumber(void * ValuePtr, int Format, int ByteCount);
double ConvertAnyFormat(void * ValuePtr, int Format);
void process_EXIF (unsigned char * ExifSection, unsigned int length);
void create_EXIF(void);
const char * ClearOrientation(void);
int RemoveThumbnail(unsigned char * ExifSection);
//int Exif2tm(struct tm * timeptr, char * ExifTime);
void ShowConciseImageInfo(void);
int ExifRotate(const char * fname, int new_angle);

#endif
