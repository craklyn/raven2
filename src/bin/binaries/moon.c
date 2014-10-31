/* ************************************************************************
 *  file:  moon.c                                       Part of RavenMUD   *
 *  Usage: This IS the infamous RavenMUD executable code. moon.c is a      *
 *  wrapper for the main function in comm.c. The wrapper is build from the *
 *  static library files.                                                  *
 *                                                                         *
 ************************************************************************* */

int moon_main (int, char **);

int
main (int argc, char **argv)
{
  return ( moon_main (argc, argv));
}

