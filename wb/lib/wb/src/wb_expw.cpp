/** 
 * Proview   Open Source Process Control.
 * Copyright (C) 2005-2014 SSAB EMEA AB.
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
 **/

/* wb_bckw.cpp -- Backupfile display window */

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>

#include "co_cdh.h"
#include "co_time.h"
#include "co_dcli.h"

#include "co_lng.h"
#include "cow_xhelp.h"
#include "cow_wow.h"
#include "wb_ldh.h"
#include "wb_expw.h"


WbExpW::WbExpW(
	void *l_parent_ctx,
	ldh_tSesContext l_ldhses,
	const char *expw_name,
	int l_type,
	int l_editmode,
	pwr_tStatus *status) :
  parent_ctx(l_parent_ctx), ldhses(l_ldhses), expwnav(NULL),
  size(0), max_size(500), type(l_type), editmode(l_editmode), wow(0)
{
  *status = 1;
  strcpy( name, expw_name);
  if ( type == expw_eType_Export) {
    strcpy( action, "Export files");
    strcpy( btext, "_Export files");
    strcpy( typetext, "export");
  }
  else if ( type == expw_eType_Import) {
    strcpy( action, "Import files");
    strcpy( btext, "_Import files");
    strcpy( typetext, "import");
  }
  else if ( type == expw_eType_BuildDirectories) {
    strcpy( action, "Build Directories");
    strcpy( btext, "_Build Directories");
    strcpy( typetext, "build");
  }
      
}

WbExpW::~WbExpW() { 
}

void WbExpW::show()
{
  expwnav->show();
}

void WbExpW::activate_export() 
{ 
  char text[80];

  sprintf( text, "Do you want to %s to marked files", typetext);
  wow->DisplayQuestion( this, action, text, 
			export_ok, 0, 0);
}

void WbExpW::export_ok( void *ctx, void *data)
{
  WbExpW *expw = (WbExpW *)ctx;

  expw->expwnav->exp();
}



