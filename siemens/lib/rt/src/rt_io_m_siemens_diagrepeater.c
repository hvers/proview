/* 
 * Proview   $Id: rt_io_m_siemens_diagrepeater.c,v 1.1 2007-04-26 12:20:45 claes Exp $
 * Copyright (C) 2005 SSAB Oxel�sund AB.
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
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* rt_io_m_siemens_diagrepeater.c -- io methods for a Siemens Diagnostic Repeater slave */

#pragma pack(1)

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>

#include "pwr.h"
#include "co_cdh.h"
#include "pwr_baseclasses.h"
#include "pwr_basecomponentclasses.h"
#include "pwr_profibusclasses.h"
#include "pwr_siemensclasses.h"
#include "rt_gdh.h"
#include "rt_io_base.h"
#include "rt_io_msg.h"
#include "rt_errh.h"
#include "co_cdh.h"
#include "rt_io_profiboard.h"
#include "rt_pb_msg.h"



/*----------------------------------------------------------------------------*\
   Read method for the Pb DP slave 
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoRackRead (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  pwr_sClass_Pb_Profiboard *mp;
  pwr_sClass_Siemens_DiagRepeater *sp;
  pwr_sClass_Siemens_DR_SegmStatus *drs;
  pwr_tUInt16 data_len, ii;
  short int *temp;
  
  sp = (pwr_sClass_Siemens_DiagRepeater *) rp->op;
  mp = (pwr_sClass_Pb_Profiboard *) ap->op;

  if ((sp->Super.Status == PB__NORMAL || sp->Super.Status == PB__NOCONN) && mp->Status == PB__NORMAL && sp->Super.DisableSlave != 1 && mp->DisableBus != 1) {

    sp->Super.Status = PB__NORMAL;
    data_len = sp->Super.BytesOfDiag;

    if (data_len > 0) {
      for (ii = 0; ii < 4; ii++) {
        if (sp->Super.Diag[6 + 19 * ii] & 0x40) { // Segment DP1
	  drs = &sp->DP1;
	}
	else if (sp->Super.Diag[6 + 19 * ii] & 0x20) { // Segment DP2
	  drs = &sp->DP2;
	}
	else if (sp->Super.Diag[6 + 19 * ii] & 0x10) { // Segment DP3
	  drs = &sp->DP3;
	}
	else if (sp->Super.Diag[6 + 19 * ii] & 0x80) { // Segment PG
	  drs = &sp->PG;
	}
	else 
	  continue;
	  
        drs->SlotNr = sp->Super.Diag[4 + 19 * ii];
	drs->LineDiag = (sp->Super.Diag[7 + 19 * ii] & 0x04);
	drs->TopologyON = !(sp->Super.Diag[7 + 19 * ii] & 0x02);
	drs->SegmentON = !(sp->Super.Diag[7 + 19 * ii] & 0x01);
	
        if (sp->Super.Diag[8 + 19 * ii] >= 0x7F)
          drs->FaultRate = 0;
        else
          drs->FaultRate = sp->Super.Diag[8 + 19 * ii];
        drs->SlaveX = sp->Super.Diag[9 + 19 * ii];
        drs->SlaveY = sp->Super.Diag[10 + 19 * ii];
        temp = (short int *) &sp->Super.Diag[11 + 19 * ii];
        drs->X_Dist = *temp;
        temp = (short int *) &sp->Super.Diag[13 + 19 * ii];
        drs->Y_Dist = *temp;
        temp = (short int *) &sp->Super.Diag[15 + 19 * ii];
        drs->DR_Dist = *temp;
        drs->ErrCause = sp->Super.Diag[17 + 19 * ii];
        drs->ErrCause |= (int) sp->Super.Diag[18 + 19 * ii] << 8;
        drs->ErrCause |= (int) sp->Super.Diag[19 + 19 * ii] << 16;
      }
    } else {
      memset(&sp->DP1, 0, sizeof(pwr_sClass_Siemens_DR_SegmStatus) * 4);
    }
  } else {
    memset(&sp->DP1, 0, sizeof(pwr_sClass_Siemens_DR_SegmStatus) * 4);
  }
  
  
  if (sp->Super.DisableSlave == 1 || mp->DisableBus == 1) sp->Super.Status = PB__DISABLED;

  return IO__SUCCESS;
}


/*----------------------------------------------------------------------------*\
  Every method to be exported to the workbench should be registred here.
\*----------------------------------------------------------------------------*/

pwr_dExport pwr_BindIoMethods(Siemens_DiagRepeater) = {
  pwr_BindIoMethod(IoRackRead),
  pwr_NullMethod
};