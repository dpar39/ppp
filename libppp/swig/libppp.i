%module example
 %{
 /* Includes the header in the wrapper code */
 #include "libppp.h"
 %}

 /* Parse the header file to generate wrappers */
 %include "../include/libppp.h"