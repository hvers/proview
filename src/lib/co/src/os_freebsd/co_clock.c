/* 
 * Proview   Open Source Process Control.
 * Copyright (C) 2005-2011 SSAB Oxelosund AB.
 *
 * This file is part of Proview.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with Proview. If not, see <http://www.gnu.org/licenses/>
 *
 * Linking Proview statically or dynamically with other modules is
 * making a combined work based on Proview. Thus, the terms and 
 * conditions of the GNU General Public License cover the whole 
 * combination.
 *
 * In addition, as a special exception, the copyright holders of
 * Proview give you permission to, from the build function in the
 * Proview Configurator, combine Proview with modules generated by the
 * Proview PLC Editor to a PLC program, regardless of the license
 * terms of these modules. You may copy and distribute the resulting
 * combined work under the terms of your choice, provided that every 
 * copy of the combined work is accompanied by a complete copy of 
 * the source code of Proview (the version used to produce the 
 * combined work), being distributed under the terms of the GNU 
 * General Public License plus this exception.
 */

/* co_clock.c -- .

 */


#ifndef OS_FREEBSD
# error This file is only for FreeBSD
#endif

#include <sys/time.h>
#include <errno.h>

#include "pwr.h"
#include "co_time.h"
#include "co_time_msg.h"

int
clock_gettime (
  clockid_t		clockid,
  struct timespec	*pt
)
{
  if (clockid == CLOCK_REALTIME) {
    struct timeval tv;

    gettimeofday( &tv, 0);

    pt->tv_sec = tv.tv_sec;
    pt->tv_nsec = tv.tv_usec * 1000;
  }
  else if ( clockid == CLOCK_MONOTONIC) {
    // TODO
    struct timeval tv;

    gettimeofday( &tv, 0);

    pt->tv_sec = tv.tv_sec;
    pt->tv_nsec = tv.tv_usec * 1000;
  }
  else {    
    errno = EINVAL;
    return -1;
  }

  return 0;
}