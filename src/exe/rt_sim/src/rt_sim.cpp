/* 
 * Proview   Open Source Process Control.
 * Copyright (C) 2005-2012 SSAB EMEA AB.
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

#include <stddef.h>

#include "pwr.h"
#include "co_cdh.h"
#include "rt_gdh.h"
#include "co_dcli.h"
#include "co_error.h"
#include "rt_aproc.h"
#include "rt_ini_event.h"
#include "rt_plc_timer.h"
#include "pwr_baseclasses.h"
#include "pwr_nmpsclasses.h"
#include "rt_sim.h"
#include "rt_gdh_msg.h"
#include "rt_sim_msg.h"
#include "rt_pwr_msg.h"
#include "rt_qcom_msg.h"
#include "nmps.h"

pwr_tCid cellclass[] = {
  pwr_cClass_NMpsCell,
  pwr_cClass_NMpsCell60,
  pwr_cClass_NMpsCell120,
  pwr_cClass_NMpsStoreCell,
  pwr_cClass_NMpsStoreCell60,
  pwr_cClass_NMpsStoreCell120,
  0};


pwr_tStatus rt_sim::print_object( FILE *fp, pwr_tOid oid)
{
  pwr_tStatus sts;
  pwr_tOid child;
  pwr_tTid tid;
  char *ap;
  pwr_tAttrRef aref = cdh_ObjidToAref( oid);
  pwr_tAName aname = "";
  pwr_tOName oname;
  pwr_tObjName cname;
  gdh_sVolumeInfo info;
  pwr_tCid skip_cid[] = {
    pwr_eClass_Node, pwr_cClass_PlcProcess, pwr_cClass_PlcThread, 0};
  int skip;
  
  sts = gdh_GetAttrRefTid( &aref, &tid);
  if ( EVEN(sts)) return sts;

  skip = 0;
  for ( int i = 0; skip_cid[i]; i++) {
    if ( tid == skip_cid[i]) {
      skip = 1;
      break;
    }
  }

  if ( !skip) {
    sts = gdh_ObjidToName( oid, oname, sizeof(oname), cdh_mName_volumeStrict);
    if ( EVEN(sts)) return sts;

    sts = gdh_GetVolumeInfo( oid.vid, &info);
    if ( EVEN(sts)) return sts;

    fprintf( fp, "// %s\n", oname);
    if ( info.cid == pwr_eClass_DynamicVolume) {
      sts = gdh_ObjidToName( cdh_ClassIdToObjid(tid), cname, sizeof(cname), 
			     cdh_mName_object);
      if ( EVEN(sts)) throw co_error(sts);

      fprintf( fp, "<dynamicname> %s %s\n", oname, cname);
    }
    else if ( info.cid == pwr_eClass_SystemVolume) {
      fprintf( fp, "<oname> %s\n", oname);
    }
    else {
      fprintf( fp, "<oid> %s\n", cdh_ObjidToString( 0, oid, 1));
    }

    sts = gdh_GetObjectSize( oid, &aref.Size);
    if ( EVEN(sts)) return sts;

    ap = (char *) calloc( 1, aref.Size);
    if ( !ap) return GDH__INSVIRMEM;

    sts = gdh_GetObjectInfoAttrref( &aref, ap, aref.Size);
    if ( EVEN(sts)) return sts;

    gdh_FWriteObjectR( fp, ap, aname, &aref, tid);
  }
  for ( sts = gdh_GetChild( oid, &child); ODD(sts); sts = gdh_GetNextSibling( child, &child)) {
    print_object( fp, child);
  }
  return 1;
}

pwr_tStatus rt_sim::load()
{
  char 		*ap = 0;
  pwr_tStatus 	sts;
  pwr_tTid 		tid;
  char 		line[512];
  char		line_elem[2][512];
  int 		nr;
  pwr_tTypeId  	a_tid;
  unsigned int 	a_size;
  unsigned int 	a_offs;
  unsigned int 	a_elem;
  char		buffer[512];
  pwr_tAttrRef  	aref;
  int 		line_cnt = 0;
  pwr_tAttrRef	oaref;
  pwr_tFileName fname;
  pwr_tOid	oid;
  FILE		*fp;
  
  dcli_trim( fname, conf->LoadFile);
  dcli_translate_filename( fname, fname);
  fp = fopen( fname, "r");
  if ( !fp)
    return SIM__FILE;
    
  while ( dcli_read_line( line, sizeof(line), fp)) {
    line_cnt++;

    dcli_trim( line, line);
    if ( strcmp( line, "") == 0 ||
	 line[0] == '#' || 
	 (line[0] == '/' && line[1] == '/'))
      continue;
      
    if ( strncmp( line, "<oid> ", 6) == 0) {
      sts = cdh_StringToObjid( &line[6], &oid);
      if ( EVEN(sts)) {
	printf( "** Objid syntax error, line %d\n", line_cnt);
	ap = 0;
	continue;
      }

      oaref = cdh_ObjidToAref( oid);

      sts = gdh_GetAttrRefTid( &oaref, &tid);
      if ( EVEN(sts)) {
	printf( "** Object not found %s, line %d\n", &line[6], line_cnt); 
	ap = 0;
	continue;
      }
      
      if ( !cdh_tidIsCid(tid))
	return GDH__NOOBJECT;

      if ( tid == pwr_eClass_Security ||
	   tid == pwr_eClass_System ||
	   tid == pwr_cClass_SimulateConfig) {
	ap = 0;
	continue;
      }

      sts = gdh_AttrRefToPointer( &oaref, (void **)&ap);
      if ( EVEN(sts)) {
	printf( "** Unable to link to object %s, line %d\n", &line[6], line_cnt); 
	ap = 0;
	continue;
      }
    }
    else if ( strncmp( line, "<oname> ", 8) == 0) {
      sts = gdh_NameToObjid( &line[8], &oid);
      if ( EVEN(sts)) {
	printf( "** Object not found, line %d\n", line_cnt);
	ap = 0;
	continue;
      }

      oaref = cdh_ObjidToAref( oid);

      sts = gdh_GetAttrRefTid( &oaref, &tid);
      if ( EVEN(sts)) {
	printf( "** Object not found %s, line %d\n", &line[8], line_cnt); 
	ap = 0;
	continue;
      }

      if ( !cdh_tidIsCid(tid))
	return GDH__NOOBJECT;

      if ( tid == pwr_eClass_Security ||
	   tid == pwr_eClass_System ||
	   tid == pwr_cClass_SimulateConfig) {
	ap = 0;
	continue;
      }

      sts = gdh_AttrRefToPointer( &oaref, (void **)&ap);
      if ( EVEN(sts)) {
	printf( "** Unable to link to object %s, line %d\n", &line[8], line_cnt); 
	ap = 0;
	continue;
      }
    }
    else if ( strncmp( line, "<dynamicname> ", 14) == 0) {
      nr = dcli_parse( line, " 	", "",
		       (char *) line_elem, sizeof( line_elem)/sizeof( line_elem[0]), 
		       sizeof( line_elem[0]), 1);
      if ( nr != 3)
	continue;

      sts = gdh_ClassNameToId( line_elem[2], &tid);
      if ( EVEN(sts)) {
	printf( "** Unable to find object class %s, line %d\n", line_elem[1], line_cnt); 
	ap = 0;
	continue;
      }

      sts = gdh_CreateObject( line_elem[1], tid, 0, &oid, pwr_cNOid, 0, pwr_cNOid);
      if ( EVEN(sts)) {
	// Object exist, link to object
	sts = gdh_NameToObjid( line_elem[1], &oid);
	if ( EVEN(sts)) {
	  printf( "** Object not found, line %d\n", line_cnt);
	  ap = 0;
	  continue;
	}

	oaref = cdh_ObjidToAref( oid);

	sts = gdh_GetAttrRefTid( &oaref, &tid);
	if ( EVEN(sts)) {
	  printf( "** Object not found %s, line %d\n", line_elem[1], line_cnt); 
	  ap = 0;
	  continue;
	}
	if ( !cdh_tidIsCid(tid))
	  return GDH__NOOBJECT;	
      }

      sts = gdh_AttrRefToPointer( &oaref, (void **)&ap);
      if ( EVEN(sts)) {
	printf( "** Unable to link to object %s, line %d\n", line_elem[1], line_cnt); 
	ap = 0;
	continue;
      }
    }
    else {
      if ( !ap)
	continue;

      nr = dcli_parse( line, " 	", "",
		       (char *) line_elem, sizeof( line_elem)/sizeof( line_elem[0]), 
		       sizeof( line_elem[0]), 1);
      if ( nr != 2)
	continue;
	
      sts = gdh_ArefANameToAref( &oaref, line_elem[0], &aref);
      if ( EVEN(sts)) continue;
      
      sts = gdh_GetAttributeCharAttrref( &aref, &a_tid, &a_size, &a_offs, &a_elem);
      if ( EVEN(sts)) continue;
      
      switch ( a_tid) {
      case pwr_eType_String:
      case pwr_eType_Text:
      case pwr_eType_Objid:
      case pwr_eType_AttrRef:
      case pwr_eType_ClassId:
      case pwr_eType_TypeId:
      case pwr_eType_CastId:
	if ( line_elem[1][0] == '"' && line_elem[1][strlen(line_elem[1]) - 1] == '"') {
	  line_elem[1][strlen(line_elem[1]) - 1] = 0;     
	  sts = gdh_AttrStringToValue( a_tid, &line_elem[1][1], buffer, sizeof(buffer), 
				       a_size);
	}
	else
	  sts = gdh_AttrStringToValue( a_tid, line_elem[1], buffer, sizeof(buffer), 
				       a_size);
	break;
      default:
	sts = gdh_AttrStringToValue( a_tid, line_elem[1], buffer, sizeof(buffer), 
				     a_size);
      }
      if ( EVEN(sts)) continue;
      
      sts = gdh_SetObjectInfoAttrref( &aref, buffer, a_size);
    }
  }
  fclose( fp);

  return SIM__SUCCESS;
}

pwr_tStatus rt_sim::store()
{
  pwr_tFileName fname;
  pwr_tStatus sts;
  pwr_tOid oid;
  FILE *fp;
  
  dcli_trim( conf->LoadFile, conf->LoadFile);
  if ( strchr( conf->LoadFile, '.') == 0)
    strcat( conf->LoadFile, ".rtdbs");
  dcli_translate_filename( fname, conf->LoadFile);
  
  fp = fopen( fname, "w");
  if ( !fp) {
    printf( "Unable to open file %s\n", fname);
    return SIM__FILE;
  }

  for ( sts = gdh_GetRootList( &oid); ODD(sts); sts = gdh_GetNextSibling( oid, &oid)) {    
    sts = print_object( fp, oid);
    if ( EVEN(sts)) return sts;
  }

  fclose( fp);

  store_nmps();

  return SIM__SUCCESS;
}

void rt_sim::init( qcom_sQid *qid)
{
  qcom_sQid qini;
  qcom_sQattr qAttr;
  pwr_tStatus sts;

  sts = gdh_Init("rt_sim");
  if ( EVEN(sts)) {
    errh_Fatal( "gdh_Init, %m", sts);
    exit(sts);
  }

  errh_Init("pwr_sim", errh_eAnix_sim);
  errh_SetStatus( PWR__SRVSTARTUP);

  if (!qcom_Init(&sts, 0, "pwr_sim")) {
    errh_Fatal("qcom_Init, %m", sts); 
    errh_SetStatus( PWR__SRVTERM);
    exit(sts);
  } 

  qAttr.type = qcom_eQtype_private;
  qAttr.quota = 100;
  if (!qcom_CreateQ(&sts, qid, &qAttr, "events")) {
    errh_Fatal("qcom_CreateQ, %m", sts);
    errh_SetStatus( PWR__SRVTERM);
    exit(sts);
  } 

  qini = qcom_cQini;
  if (!qcom_Bind(&sts, qid, &qini)) {
    errh_Fatal("qcom_Bind(Qini), %m", sts);
    errh_SetStatus( PWR__SRVTERM);
    exit(-1);
  }
}

void rt_sim::clear_timers()
{
  for ( unsigned int i = 0; i < thread_cnt; i++) {
    void *cp, *next;

    cp = RELPTR(threadp[i]->TimerStart);
    threadp[i]->TimerStart = 0;
    while( cp) {
      next = RELPTR(((plc_sTimer *)cp)->TimerNext);
      ((plc_sTimer *)cp)->TimerNext = 0;
      cp = next;
    }
  }
}

typedef struct {
  pwr_tCid cid;
  unsigned int timer_offset;
} sim_sTimerObject;

void rt_sim::restore_timers( int thread_idx, pwr_tOid oid)
{
  pwr_tStatus sts;
  pwr_tOid child;
  pwr_tCid cid;
  char *op;
  plc_sTimer *timerp;
  sim_sTimerObject tlist[] = {
    {pwr_cClass_ASup, offsetof(pwr_sClass_ASup, TimerFlag)},
    {pwr_cClass_dorder, offsetof(pwr_sClass_dorder, TimerFlag)},
    {pwr_cClass_drive, offsetof(pwr_sClass_drive, TimerFlag)},
    {pwr_cClass_DSup, offsetof(pwr_sClass_DSup, TimerFlag)},
    {pwr_cClass_inc3p, offsetof(pwr_sClass_inc3p, TimerFlag)},
    {pwr_cClass_lorder, offsetof(pwr_sClass_lorder, TimerFlag)},
    {pwr_cClass_mvalve, offsetof(pwr_sClass_mvalve, TimerFlag)},
    {pwr_cClass_pos3p, offsetof(pwr_sClass_pos3p, TimerFlag)},
    {pwr_cClass_posit, offsetof(pwr_sClass_posit, TimerFlag)},
    {pwr_cClass_pulse, offsetof(pwr_sClass_pulse, TimerFlag)},
    {pwr_cClass_timer, offsetof(pwr_sClass_timer, TimerFlag)},
    {pwr_cClass_valve, offsetof(pwr_sClass_valve, TimerFlag)},
    {pwr_cClass_waith, offsetof(pwr_sClass_waith, TimerFlag)},
    {pwr_cClass_wait, offsetof(pwr_sClass_wait, TimerFlag)},
    {0,0}};
  
  sts = gdh_GetObjectClass( oid, &cid);
  if ( EVEN(sts)) throw co_error(sts);

  // If activated timer, insert object in thread timerlist
  for ( unsigned int i = 0; tlist[i].cid != 0; i++) {
    if ( cid == tlist[i].cid) {
      sts = gdh_ObjidToPointer( oid, (void **)&op);
      if ( EVEN(sts)) throw co_error(sts);

      timerp = (plc_sTimer *)(op + tlist[i].timer_offset);
      if ( timerp->TimerFlag) {
	if ( timerp->TimerCount > 0) {
	  timerp->TimerNext = threadp[thread_idx]->TimerStart;
	  PTRREL(&threadp[thread_idx]->TimerStart, &timerp->TimerFlag);
	}
	else
	  ((plc_sTimer *)(op + tlist[i].timer_offset))->TimerFlag = 0;
      }
      break;
    }
  }

  // Examine children
  for ( sts = gdh_GetChild( oid, &child);
	ODD(sts);
	sts = gdh_GetNextSibling( child, &child)) {
    restore_timers( thread_idx, child);
  }
}

void rt_sim::restore_timers()
{
  pwr_sClass_plc *plcp;
  pwr_tOid plcoid;
  pwr_tStatus sts;

  for ( unsigned int i = 0; i < thread_cnt; i++) {
    // Find all PlcPgm for this thread
    for ( sts = gdh_GetClassList( pwr_cClass_plc, &plcoid);
	  ODD(sts);
	  sts = gdh_GetNextObject( plcoid, &plcoid)) {
      sts = gdh_ObjidToPointer( plcoid, (void **)&plcp);
      if ( EVEN(sts)) throw co_error(sts);

      if ( cdh_ObjidIsEqual( plcp->ThreadObject, conf->PlcThreads[i]))
	restore_timers( i, plcoid);
    }
  }
}

void rt_sim::delete_children( pwr_tOid oid)
{
  pwr_tStatus sts;
  pwr_tOid coid, next;

  int last = 0;
  for ( sts = gdh_GetChild( oid, &coid); ODD(sts); ) {
    sts = gdh_GetNextSibling( coid, &next);
    if ( EVEN(sts)) 
      last = 1;

    delete_children( coid);

    sts = gdh_DeleteObject( coid);
    if ( EVEN(sts)) throw co_error(sts);

    if ( last)
      break;

    coid = next;
  }
}

void rt_sim::clear_nmps( sim_eNMpsClear mode)
{
  pwr_tStatus sts;
  pwr_tVid vid;
  gdh_sVolumeInfo info;
  pwr_tOid oid;

  // Unref and remove all data objects in NMps cells
  for ( int i = 0; cellclass[i]; i++) {
    for ( sts = gdh_GetClassList( cellclass[i], &oid);
	  ODD(sts);
	  sts = gdh_GetNextObject( oid, &oid)) {
      pwr_sClass_NMpsCell *cellp;
      plc_t_DataInfo *dip;

      sts = gdh_ObjidToPointer( oid, (void **)&cellp);
      if ( EVEN(sts)) throw co_error(sts);

      if ( mode == sim_eNMpsClear_NoBackup &&
	   cellp->Function & NMPS_CELLFUNC_BACKUP)
	continue;

      dip = (plc_t_DataInfo *)&cellp->Data1P;
      for ( int i = 0; i < cellp->LastIndex; i++) {
	gdh_UnrefObjectInfo( dip->Data_Dlid);
	memset( dip, 0, sizeof(*dip));
	dip++;
      }

      dip = (plc_t_DataInfo *)&cellp->DataLP;
      memset( dip, 0, sizeof(*dip));
      dip = (plc_t_DataInfo *)&cellp->DataLastP;
      memset( dip, 0, sizeof(*dip));
      cellp->LastIndex = 0;
      cellp->NumberOfData = 0;    
    }
  }

  // Remove all dynamic objects 
  for  ( sts = gdh_GetVolumeList( &vid);
	 ODD(sts);
	 sts = gdh_GetNextVolume( vid, &vid)) {
    sts = gdh_GetVolumeInfo( vid, &info);
    if ( EVEN(sts)) throw co_error(sts);

    if ( info.cid == pwr_eClass_DynamicVolume) {
      oid.oix = 0;
      oid.vid = vid;

      delete_children( oid);

    }
  }
}

void rt_sim::store_nmps()
{
  pwr_tStatus sts;
  pwr_tOid oid;
  int backup_found = 0;

  for ( int i = 0; cellclass[i]; i++) {
    for ( sts = gdh_GetClassList( cellclass[i], &oid);
	  ODD(sts);
	  sts = gdh_GetNextObject( oid, &oid)) {
      pwr_sClass_NMpsCell *cellp;

      sts = gdh_ObjidToPointer( oid, (void **)&cellp);
      if ( EVEN(sts)) throw co_error(sts);

      if ( cellp->Function & NMPS_CELLFUNC_BACKUP)
	backup_found = 1;
    }
  }

  if ( backup_found) {
    // Copy nmps backup files
    sts = gdh_GetClassList( pwr_cClass_NMpsBackupConfig, &oid);
    if ( ODD(sts)) {
      pwr_sClass_NMpsBackupConfig *bckp;
      pwr_tCmd cmd;
      pwr_tFileName fname;
      char *s;

      sts = gdh_ObjidToPointer( oid, (void **)&bckp);
      if ( EVEN(sts)) throw co_error(sts);

      strcpy( fname, conf->LoadFile);
      if ( (s = strrchr( fname, '.')))
	*s = 0;
      strcat( fname, "_nmpsbck");
	   
      sprintf( cmd, "cp %s1 %s.bck1;cp %s2 %s.bck2", bckp->BackupFile, fname,
	       bckp->BackupFile, fname);
      system( cmd);
    }
  }
}

void rt_sim::load_nmps()
{
  pwr_tCmd cmd;
  pwr_tFileName fname;
  char *s;

  strcpy( fname, conf->LoadFile);
  if ( (s = strrchr( fname, '.')))
    *s = 0;
  strcat( fname, "_nmpsbck");
	   
  sprintf( cmd, "rs_nmps_bck -l %s", fname);
  system( cmd);
}

void rt_sim::scan()
{
  pwr_tStatus sts;

  // Set PlcPgm thread and plcpgm status
  for ( unsigned int i = 0; i < plcpgm_cnt; i++) {
    conf->PlcPgmThreadStatus[i] = conf->ThreadStatus[plcpgm_thread_idx[i]];
    conf->PlcPgmStatus[i] = windowplcp[i]->ScanOff ? SIM__SCANOFF : SIM__SCANON;
  }

  if ( conf->Disable) {
    if ( conf->PlcHalt)
      conf->PlcHalt = 0;
    if ( conf->PlcContinue)
      conf->PlcContinue = 0;
    if ( conf->PlcStep)
      conf->PlcStep = 0;
    if ( conf->Load)
      conf->Load = 0;
    if ( conf->Store)
      conf->Store = 0;
    if ( conf->PlcPgmScanOff)
      conf->PlcPgmScanOff = 0;
    if ( conf->PlcPgmScanOn)
      conf->PlcPgmScanOn = 0;

    
    if ( !disable_old) {    
      // Set running status on all threads
      for ( unsigned int i = 0; i < thread_cnt; i++) {
	if ( conf->ThreadStatus[i] == SIM__THREAD_HALT)
	  conf->ThreadStatus[i] = SIM__THREAD_RUNNING;
      }
      // Set scan on of plcpgm
      for ( unsigned int i = 0; i < plcpgm_cnt; i++) {
	if ( plcpgm_scanoff_set[i]) {
	  windowplcp[i]->ScanOff = 0;
	  plcpgm_scanoff_set[i] = 0;
	}
      }

      conf->PlcHaltOrder = 0;
      conf->PlcStepOrder = 0;
      conf->PlcContinueOrder = 0;
      conf->PlcLoadOrder = 0;
    }

    disable_old = conf->Disable;
    conf->Message = SIM__DISABLED;
    return;
  }

  if ( !conf->Disable && disable_old)
    conf->Message = SIM__ACTIVE;

  disable_old = conf->Disable;

  // Select all threads request
  if ( conf->SelectAllThreads) {
    conf->SelectAllThreads = 0;
    for ( unsigned int i = 0; i < thread_cnt; i++)
      conf->ThreadSelected[i] = 1;
  }

  // Clear all threads request
  if ( conf->ClearAllThreads) {
    conf->ClearAllThreads = 0;
    for ( unsigned int i = 0; i < thread_cnt; i++)
      conf->ThreadSelected[i] = 0;
  }

  // Select all plcpgm request
  if ( conf->SelectAllPlcPgm) {
    conf->SelectAllPlcPgm = 0;
    for ( unsigned int i = 0; i < plcpgm_cnt; i++)
      conf->PlcPgmSelected[i] = 1;
  }

  // Clear all plcpgm request
  if ( conf->ClearAllPlcPgm) {
    conf->ClearAllPlcPgm = 0;
    for ( unsigned int i = 0; i < plcpgm_cnt; i++)
      conf->PlcPgmSelected[i] = 0;
  }

  // Plc halt request
  if ( conf->PlcHalt) {
    conf->PlcHalt = 0;
    conf->PlcContinueStatus = 0;

    // Count selected threads in running state
    select_thread_cnt = 0;
    for ( unsigned int i = 0; i < thread_cnt; i++) {
      if ( conf->ThreadSelected[i] && conf->ThreadStatus[i] == SIM__THREAD_RUNNING)
	select_thread_cnt++;
    }

    if ( select_thread_cnt > 0) {
      conf->PlcHaltOrder = select_thread_cnt;
      halt_order_active = true;
      conf->PlcHaltStatus = SIM__THREADRESPOND;
      conf->Message = SIM__THREADRESPOND;
    }
    else {
      conf->PlcHaltStatus = SIM__NORUNNING;
      conf->Message = SIM__NORUNNING;
    }
  }

  if ( halt_order_active) {
    if ( conf->PlcHaltOrder == 0) {
      conf->PlcHaltStatus = SIM__SUCCESS;
      conf->Message = SIM__HALTED;
      halt_order_active = false;
    }
  }

  // Plc continue request
  if ( conf->PlcContinue) {
    conf->PlcContinue = 0;
    conf->PlcHaltStatus = 0;

    // Count selected threads
    select_thread_cnt = 0;
    for ( unsigned int i = 0; i < thread_cnt; i++) {
      if ( conf->ThreadSelected[i] && conf->ThreadStatus[i] == SIM__THREAD_HALT)
	select_thread_cnt++;
    }

    if ( select_thread_cnt > 0) {
      conf->PlcContinueOrder = select_thread_cnt;
      continue_order_active = true;
      conf->Message = SIM__THREADRESPOND;
    }
    else {
      conf->Message = SIM__NOHALTED;
    }
  }

  if ( continue_order_active) {
    if ( conf->PlcContinueOrder == 0) {
      continue_order_active = false;
      conf->Message = SIM__CONTINUED;
    }
  }

  // Plc step request
  if ( conf->PlcStep) {
    conf->PlcStep = 0;

    // Count selected threads
    select_thread_cnt = 0;
    for ( unsigned int i = 0; i < thread_cnt; i++) {
      if ( conf->ThreadSelected[i] && conf->ThreadStatus[i] == SIM__THREAD_HALT)
	select_thread_cnt++;
    }
    
    if ( select_thread_cnt > 0) {
      conf->PlcStepOrder = select_thread_cnt;
      step_order_active = true;
      conf->Message = SIM__THREADRESPOND;
    }
    else {
      conf->Message = SIM__NOHALTED;
    }
  }

  if ( step_order_active) {
    if ( conf->PlcStepOrder == 0) {
      step_order_active = false;
      conf->Message = SIM__STEPPED;
    }
  }

  // Load database request
  if ( conf->Load) {
    conf->Load = 0;

    // Check that all thread are halted
    int not_halted = 0;
    for ( unsigned int i = 0; i < thread_cnt; i++) {
      if ( conf->ThreadStatus[i] != SIM__THREAD_HALT) {
	not_halted = 1;
	break;
      }
    }


    if ( not_halted) {
      conf->Message = SIM__NOTALLHALTED;
    }
    else {
      conf->Message = SIM__LOADING;

      qcom_SignalOr(&sts, &qcom_cQini, ini_mEvent_simLoadStart);

      // Clear plc thread timer lists
      clear_timers();

      clear_nmps( sim_eNMpsClear_All);

      load();

      clear_nmps( sim_eNMpsClear_NoBackup);

      // Restore timers
      restore_timers();

      load_nmps();

      qcom_SignalAnd(&sts, &qcom_cQini, ~ini_mEvent_simLoadStart);
      qcom_SignalOr(&sts, &qcom_cQini, ini_mEvent_simLoadDone);

      // Plc load request
      conf->PlcLoadOrder = thread_cnt;
      load_order_active = true;
      conf->Message = SIM__THREADRESPOND;
		     
      qcom_SignalAnd(&sts, &qcom_cQini, ~ini_mEvent_simLoadDone);
				    
    }
  }


  if ( load_order_active) {
    if ( conf->PlcLoadOrder == 0) {
      load_order_active = false;

      conf->Message = SIM__LOADED;
    }
  }

  if ( conf->Store) {
    conf->Store = 0;

    // Check that all thread are halted
    int not_halted = 0;
    for ( unsigned int i = 0; i < thread_cnt; i++) {
      if ( conf->ThreadStatus[i] != SIM__THREAD_HALT) {
	not_halted = 1;
	break;
      }
    }

    if ( not_halted) {
      conf->Message = SIM__NOTALLHALTED;
    }
    else {
      conf->Message = SIM__STORING;
      store();
      conf->Message = SIM__STORED;
    }
  }

  // PlcPgm scan on request
  if ( conf->PlcPgmScanOn) {
    conf->PlcPgmScanOn = 0;

    int found = 0;
    for ( unsigned int i = 0; i < plcpgm_cnt; i++) {
      if ( conf->PlcPgmSelected[i]) {
	windowplcp[i]->ScanOff = 0;
	conf->PlcPgmStatus[i] = SIM__SCANON;
	plcpgm_scanoff_set[i] = 0;
	found = 1;
      }
    }
    if ( found)
      conf->Message = SIM__SCANON_SET;
    else
      conf->Message = SIM__NOSELPLCPGM;
  }

  // PlcPgm scan off request
  if ( conf->PlcPgmScanOff) {
    conf->PlcPgmScanOff = 0;

    int found = 0;
    for ( unsigned int i = 0; i < plcpgm_cnt; i++) {
      if ( conf->PlcPgmSelected[i]) {
	windowplcp[i]->ScanOff = 1;
	conf->PlcPgmStatus[i] = SIM__SCANOFF;
	plcpgm_scanoff_set[i] = 1;
	found = 1;
      }
    }
    if ( found)
      conf->Message = SIM__SCANOFF_SET;
    else
      conf->Message = SIM__NOSELPLCPGM;
  }
}

void rt_sim::close()
{
}

void rt_sim::open()
{
  pwr_tStatus sts;
  pwr_tOid oid;
  pwr_tOid child;
  pwr_tCid cid;
  pwr_tAttrRef aref, taref;
  pwr_tOid thread_oid;

  // Find server configuration object SimulateConfig
  sts = gdh_GetClassList( pwr_cClass_SimulateConfig, &oid);
  if ( ODD(sts)) {
    sts = gdh_ObjidToPointer( oid, (void **)&conf);
    if ( EVEN(sts)) throw co_error( sts);

    aproc_RegisterObject( oid);
  }
  else {
    errh_Info( "No Simulate configuration");
    errh_SetStatus( 0);
    exit(0);
  }

  // Get all plc threads and insert into array
  thread_cnt = 0;
  for ( sts = gdh_GetClassList( pwr_cClass_PlcThread, &oid); 
	ODD(sts); 
	sts = gdh_GetNextObject( oid, &oid)) {
    conf->PlcThreads[thread_cnt] = oid;
    conf->ThreadSelected[thread_cnt] = 1;

    pwr_tAttrRef aref = cdh_ObjidToAref( oid);
    sts = gdh_DLRefObjectInfoAttrref( &aref, (void **)&threadp[thread_cnt], &thread_dlid[thread_cnt]);
    if ( EVEN(sts)) {
      errh_Fatal( "Unable to link to plc thread object, %m", sts);
      errh_SetStatus( PWR__SRVTERM);
      exit(0);
    }

    thread_cnt++;
    if ( thread_cnt >= sizeof(conf->PlcThreads)/sizeof(conf->PlcThreads[0]))
      break;

  }

  // Get all plcpgm and insert into array
  plcpgm_cnt = 0;
  for ( sts = gdh_GetClassList( pwr_cClass_plc, &oid); 
	ODD(sts); 
	sts = gdh_GetNextObject( oid, &oid)) {
    conf->PlcPgm[plcpgm_cnt] = oid;
    conf->PlcPgmSelected[plcpgm_cnt] = 1;

    int found = 0;
    for ( sts = gdh_GetChild( oid, &child);
	  ODD(sts);
	  sts = gdh_GetNextSibling( child, &child)) {
      sts = gdh_GetObjectClass( child, &cid);
      if ( cid == pwr_cClass_windowplc) {
    
	pwr_tAttrRef aref = cdh_ObjidToAref( child);
	sts = gdh_DLRefObjectInfoAttrref( &aref, (void **)&windowplcp[plcpgm_cnt], &windowplc_dlid[plcpgm_cnt]);
	if ( EVEN(sts)) {
	  errh_Fatal( "Unable to link to plc window object, %m", sts);
	  errh_SetStatus( PWR__SRVTERM);
	  exit(0);
	}
	found = 1;
	break;
      }
    }
    if ( !found) {
      errh_Error( "Unable to find plc window object");
    }

    aref = cdh_ObjidToAref( oid);
    sts = gdh_ArefANameToAref( &aref, "ThreadObject", &taref);    
    if ( EVEN(sts)) co_error(sts);

    sts = gdh_GetObjectInfoAttrref( &taref, &thread_oid, sizeof(thread_oid));
    if ( EVEN(sts)) {
      errh_Error( "No valid thread object in PlcPgm");
      continue;
    }
    
    found = 0;
    for ( unsigned int i = 0; i < thread_cnt; i++) {
      if ( cdh_ObjidIsEqual( thread_oid, conf->PlcThreads[i])) {
	plcpgm_thread_idx[plcpgm_cnt] = i;
	found = 1;
	break;
      }
    }
    if ( !found)
      continue;

    plcpgm_cnt++;
    if ( plcpgm_cnt >= sizeof(conf->PlcPgm)/sizeof(conf->PlcPgm[0]))
      break;

  }

  conf->Status = PWR__SRUN;
  conf->InitDone = 1;
  disable_old = conf->Disable;
}

int main( int argc, char *argv[])
{
  pwr_tStatus sts;
  int tmo;
  char mp[2000];
  qcom_sQid qid = qcom_cNQid;
  qcom_sGet get;
  int swap = 0;
  bool first_scan = true;
  rt_sim *sim;

  sim = new rt_sim();
  sim->init( &qid);

  try {
    sim->open();
  }
  catch ( co_error& e) {
    errh_Error( (char *)e.what().c_str());
    errh_Fatal( "rt_sim aborting");
    errh_SetStatus( PWR__SRVTERM);
    exit(0);
  }

  aproc_TimeStamp(sim->scantime(), 10);
  errh_SetStatus( PWR__SRUN);

  first_scan = true;
  for (;;) {
    if ( first_scan) {
      tmo = (int) (sim->scantime() * 1000 - 1);
    }

    get.maxSize = sizeof(mp);
    get.data = mp;
    qcom_Get( &sts, &qid, &get, tmo);
    if (sts == QCOM__TMO || sts == QCOM__QEMPTY) {
      if ( !swap)
	sim->scan();
    } 
    else {
      ini_mEvent  new_event;
      qcom_sEvent *ep = (qcom_sEvent*) get.data;

      new_event.m  = ep->mask;
      if (new_event.b.oldPlcStop && !swap) {
	errh_SetStatus( PWR__SRVRESTART);
	sim->conf->Status = PWR__SRVRESTART;
        swap = 1;
	sim->close();
      } else if (new_event.b.swapDone && swap) {
        swap = 0;
	sim->open();
	errh_SetStatus( PWR__SRUN);
	sim->conf->Status = PWR__SRUN;
      } else if (new_event.b.terminate) {
	exit(0);
      }
    }
    first_scan = false;
  }

}