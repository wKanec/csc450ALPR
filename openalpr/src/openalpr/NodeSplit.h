
#ifndef _NODESPLIT_H_INCLUDED_
#define _NODESPLIT_H_INCLUDED_


#include <iostream>
#include <vector>
#include <fstream> 
#include <stdint.h>
#include <string>

#ifdef WIN32
  #define OPENALPR_DLL_EXPORT __declspec( dllexport )
#else
  #define OPENALPR_DLL_EXPORT 
#endif



int Split1();


#endif // OPENALPR_APLR_H

