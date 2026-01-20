/* ============================================================================
Header file for RavenMUD condition limits.
Written by Xiuhtecuhtli of RavenMUD for RavenMUD.
============================================================================ */
#ifndef _MUDLIMITS_H_
#define _MUDLIMITS_H_

/* ============================================================================
Public functions.
============================================================================ */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6);

/* Everything that was once in this header file has been removed. I found that
 * public functions for mudlimits.h were being kept in utils.h. I don't know
 * which location makes more sense, but I made the decision to kill off this
 * header file and leave utils.h defining these functions in the global scope.
 * -Xiuh 08.27.09 */

#endif /* _MUDLIMITS_H_ */
