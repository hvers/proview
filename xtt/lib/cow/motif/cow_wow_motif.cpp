/** 
 * Proview   $Id: co_wow_motif.cpp,v 1.4 2008-12-01 16:42:35 claes Exp $
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
 **/

/* cow_wow_motif.cpp -- useful windows */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <X11/Intrinsic.h>
#include <Xm/MessageB.h>
#include <Xm/MainW.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>

#include "pwr.h"
#include "co_dcli.h"
#include "cow_wow_motif.h"
#include "co_api.h"
#include "flow_x.h"

#define WOW_MAXNAMES 400

typedef struct {
  char		        str[200];
  int 			len;
  int			received;
  pwr_tStatus		sts;
  Atom			atom;
} wow_sSelection;

void CoWowMotif::error_ok_cb(Widget w)
{
    XtDestroyWidget( w);
}

void CoWowMotif::question_ok_cb ( Widget dialog, 
				  XtPointer data, 
				  XmAnyCallbackStruct *cbs)
{
  wow_t_question_cb *cbdata = (wow_t_question_cb *) data;
  
  if (cbdata->questionbox_ok)
    (cbdata->questionbox_ok)( cbdata->ctx, cbdata->data);

  XtFree ((char *)cbdata);
  XtDestroyWidget (dialog);
}

void CoWowMotif::question_cancel_cb ( Widget dialog, 
				      XtPointer data, 
				      XmAnyCallbackStruct *cbs)
{
  wow_t_question_cb *cbdata = (wow_t_question_cb *) data;
  
  if (cbdata->questionbox_cancel)
    (cbdata->questionbox_cancel)( cbdata->ctx, cbdata->data);

  XtFree ((char *)cbdata);
  XtDestroyWidget (dialog);
}

void CoWowMotif::question_help_cb( Widget dialog,XtPointer data, 
				   XmAnyCallbackStruct *cbs)
{
  wow_t_question_cb *cbdata = (wow_t_question_cb *) data;
  
  if (cbdata->questionbox_help)
    (cbdata->questionbox_help)( cbdata->ctx, cbdata->data);
}


/************************************************************************
*
* Name: DisplayQuestion		
*
* Description:	Displays an question box widget 
*
*************************************************************************/

void CoWowMotif::DisplayQuestion( void *ctx, const char *title, const char *text,
				  void (* questionbox_ok) ( void *, void *),
				  void (* questionbox_cancel) ( void *, void *),
				  void *data)
{
    Arg	    arg[10];
    Widget  question_widget, w;
    XmString CStr2, TitleStr, okstr, cancelstr;
    wow_t_question_cb *cbdata;
    XmFontList fontlist;
    XFontStruct *font;
    XmFontListEntry fontentry;

    // Set default fontlist
    font = XLoadQueryFont( XtDisplay(m_parent),
	      "-*-Helvetica-Bold-R-Normal--12-*-*-*-P-*-ISO8859-1");
    fontentry = XmFontListEntryCreate( (char*) "tag1", XmFONT_IS_FONT, font);
    fontlist = XmFontListAppendEntry( NULL, fontentry);
    XtFree( (char *)fontentry);

    CStr2 = XmStringCreateLtoR( (char*) text, XmSTRING_DEFAULT_CHARSET);
    TitleStr = XmStringCreateLtoR( (char*) title, XmSTRING_DEFAULT_CHARSET);    
    okstr = XmStringCreateLtoR( (char*) " Yes ", XmSTRING_DEFAULT_CHARSET );    
    cancelstr = XmStringCreateLtoR( (char*) " No  ", XmSTRING_DEFAULT_CHARSET );    
    XtSetArg(arg[0],XmNheight,75);
    XtSetArg(arg[1],XmNwidth,200);
    XtSetArg(arg[2],XmNmessageString, CStr2);
    XtSetArg(arg[3],XmNx,400);
    XtSetArg(arg[4],XmNy,300);
    XtSetArg(arg[5],XmNdialogTitle,TitleStr);
    XtSetArg(arg[6], XmNokLabelString, okstr);
    XtSetArg(arg[7], XmNcancelLabelString, cancelstr);
    XtSetArg(arg[8], XmNbuttonFontList, fontlist);
    XtSetArg(arg[9], XmNlabelFontList, fontlist);

    cbdata = (wow_t_question_cb *) XtCalloc( 1, sizeof(*cbdata));
    cbdata->questionbox_ok = questionbox_ok;
    cbdata->questionbox_cancel = questionbox_cancel;
    cbdata->questionbox_help = 0;
    cbdata->ctx = ctx;
    cbdata->data = data;

    question_widget = XmCreateQuestionDialog( m_parent,(char*) "questionDialog",arg,10);
    XtAddCallback( question_widget, XmNokCallback,
		(XtCallbackProc) question_ok_cb, cbdata);
    XtAddCallback( question_widget, XmNcancelCallback, 
		(XtCallbackProc) question_cancel_cb, cbdata);

    XmStringFree( CStr2);
    XmStringFree( TitleStr);
    XmStringFree( okstr);
    XmStringFree( cancelstr);
    XmFontListFree( fontlist);
    
    XtManageChild( question_widget);	       
    
    w = XmMessageBoxGetChild( question_widget, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(w);    

}


/************************************************************************
*
* Name: DisplayError(Text)		
*
* Type:	void	
*
* Widget	Father		I	The widget that is the father of
*					the message box
* char		*Text		I	Message text
*
* Description:	Displays an error message box widget 
*
*************************************************************************/

void CoWowMotif::DisplayError( const char *title, const char *text)
{
    Arg	    arg[10];
    Widget  err_widget, w;
    XmString cstr, ctitle;
    XmFontList fontlist;
    XFontStruct *font;
    XmFontListEntry fontentry;

    // Set default fontlist
    font = XLoadQueryFont( XtDisplay(m_parent),
	      "-*-Helvetica-Bold-R-Normal--12-*-*-*-P-*-ISO8859-1");
    fontentry = XmFontListEntryCreate( (char*) "tag1", XmFONT_IS_FONT, font);
    fontlist = XmFontListAppendEntry( NULL, fontentry);
    XtFree( (char *)fontentry);

    cstr = XmStringCreateLtoR( (char*) text, XmSTRING_DEFAULT_CHARSET);
    ctitle = XmStringCreateLtoR( (char*) title, XmSTRING_DEFAULT_CHARSET);    
    XtSetArg(arg[0],XmNheight,75);
    XtSetArg(arg[1],XmNwidth,200);
    XtSetArg(arg[2],XmNmessageString, cstr);
    XtSetArg(arg[3],XmNx,400);
    XtSetArg(arg[4],XmNy,300);
    XtSetArg(arg[5],XmNdialogTitle, ctitle);
    XtSetArg(arg[6], XmNbuttonFontList, fontlist);
    XtSetArg(arg[7], XmNlabelFontList, fontlist);

    err_widget = XmCreateErrorDialog( m_parent,(char*) "err_widget",arg,8);
    XtAddCallback(err_widget, XmNokCallback, 
		(XtCallbackProc) error_ok_cb, NULL);

    XmStringFree( cstr);
    XmStringFree( ctitle);
    XmFontListFree( fontlist);
      
    XtManageChild(err_widget);	       
    
    w = XmMessageBoxGetChild(err_widget, XmDIALOG_CANCEL_BUTTON);
    XtUnmanageChild( w);    
    
    w = XmMessageBoxGetChild(err_widget, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild( w);    

} /* END DisplayErrorBox */

/************************************************************************
*
* Description: Create a window with a scrolled list and Ok and Cancel
*              buttons.
*
*************************************************************************/

typedef struct {
  Widget    dialog;
  void      (*file_selected_cb)( void *, char *, wow_eFileSelType);
  void      *parent_ctx;
  wow_eFileSelType file_type;
} *wow_tFileSelCtx;

void CoWowMotif::list_ok_cb( Widget w, XtPointer data, 
			     XmAnyCallbackStruct *cbs)
{
  wow_tListCtx ctx = (wow_tListCtx) data;
  int 		*pos_list, pos_cnt;
  static char   selected_text[512];
  
  if ( ctx->action_cb) {
    if (XmListGetSelectedPos( ctx->list, &pos_list, &pos_cnt)) {
      strcpy( selected_text, ctx->texts + (pos_list[0] - 1) * ctx->textsize);
      (ctx->action_cb)( ctx->parent_ctx, selected_text);

      XtFree( (char *)pos_list);
    }
  }

  XtDestroyWidget( ctx->toplevel);
  free( ctx->texts);
  free( ctx);
}

void CoWowMotif::list_cancel_cb( Widget w, XtPointer data, 
				 XmAnyCallbackStruct *cbs)
{
  wow_tListCtx ctx = (wow_tListCtx) data;
  
  if ( ctx->cancel_cb)
    (ctx->cancel_cb)( ctx->parent_ctx);

  XtDestroyWidget( ctx->toplevel);
  free( ctx->texts);
  free( ctx);
}

void CoWowMotif::list_action_cb( Widget w, XtPointer data,
				 XmListCallbackStruct *cbs)
{
  wow_tListCtx ctx = (wow_tListCtx) data;
  char          *item_str;
  static char   action_text[512];
  
  if ( cbs->event->type == KeyPress)
    // The ok callback will be called later
    return;

  if ( ctx->action_cb) {
    XmStringGetLtoR( cbs->item, XmSTRING_DEFAULT_CHARSET, &item_str);
    strcpy( action_text, item_str);

    XtFree( item_str);
    (ctx->action_cb)( ctx->parent_ctx, action_text);
  }

  XtDestroyWidget( ctx->toplevel);
  free( ctx->texts);
  free( ctx);
}

void *CoWowMotif::CreateList( const char *title, const char *texts, int textsize,
			      void (action_cb)( void *, char *),
			      void (cancel_cb)( void *),
			      void *parent_ctx,
			      int show_apply_button)
{
  Arg	    args[15];
  XmString cstr;
  Widget mainwindow;
  Widget ok_button;
  Widget cancel_button;
  Widget form;
  char *name_p;
  int i;
  wow_tListCtx ctx;
  XmFontList fontlist;
  XFontStruct *font;
  XmFontListEntry fontentry;

  ctx = (wow_tListCtx) calloc( 1, sizeof(*ctx));
  ctx->action_cb = action_cb;
  ctx->cancel_cb = cancel_cb;
  ctx->parent_ctx = parent_ctx;
  
  i=0;
  XtSetArg( args[i], XmNiconName, title); i++;

  ctx->toplevel = XtCreatePopupShell (
        title, topLevelShellWidgetClass, m_parent, args, i);

  // Set default fontlist
  font = XLoadQueryFont( XtDisplay(ctx->toplevel),
	      "-*-Helvetica-Bold-R-Normal--12-*-*-*-P-*-ISO8859-1");
  fontentry = XmFontListEntryCreate( (char*) "tag1", XmFONT_IS_FONT, font);
  fontlist = XmFontListAppendEntry( NULL, fontentry);
  XtFree( (char *)fontentry);

  i=0;
  XtSetArg( args[i], XmNbuttonFontList, fontlist); i++;
  XtSetArg( args[i], XtNallowShellResize, TRUE); i++;
  XtSetValues( ctx->toplevel, args, i);

  mainwindow = XmCreateMainWindow( ctx->toplevel, (char*) "mainWindow", NULL, 0);
  XtManageChild( mainwindow);

  i=0;
  XtSetArg(args[i],XmNwidth, 200);i++;
  XtSetArg(args[i],XmNheight, 400);i++;
  XtSetArg(args[i],XmNresizePolicy,XmRESIZE_NONE); i++;

  form = XmCreateForm( mainwindow, (char*) "form", args, i);
  XtManageChild( form);

  cstr = XmStringCreateLtoR( (char*) "Ok", XmSTRING_DEFAULT_CHARSET);

  i=0;
  XtSetArg( args[i], XmNbottomAttachment, XmATTACH_FORM); i++;
  XtSetArg( args[i], XmNbottomOffset, 20); i++;
  XtSetArg( args[i], XmNleftAttachment, XmATTACH_FORM); i++;
  XtSetArg( args[i], XmNleftOffset, 20); i++;
  XtSetArg( args[i], XmNwidth, 50); i++;
  XtSetArg( args[i], XmNlabelString, cstr); i++;

  ok_button = XmCreatePushButton( form, (char*) "okButton", args, i);
  XtAddCallback( ok_button, XmNactivateCallback,
		(XtCallbackProc) list_ok_cb, ctx);
  XtManageChild( ok_button);

  XmStringFree( cstr);

  cstr = XmStringCreateLtoR( (char*) "Cancel", XmSTRING_DEFAULT_CHARSET);

  i=0;
  XtSetArg( args[i], XmNbottomAttachment, XmATTACH_FORM); i++;
  XtSetArg( args[i], XmNbottomOffset, 20); i++;
  XtSetArg( args[i], XmNrightAttachment, XmATTACH_FORM); i++;
  XtSetArg( args[i], XmNrightOffset, 20); i++;
  XtSetArg( args[i], XmNwidth, 50); i++;
  XtSetArg( args[i], XmNlabelString, cstr); i++;

  cancel_button = XmCreatePushButton( form, (char*) "okButton", args, i);
  XtAddCallback( cancel_button, XmNactivateCallback,
		(XtCallbackProc) list_cancel_cb, ctx);
  XtManageChild( cancel_button);

  XmStringFree( cstr);

  i = 0;
  XtSetArg( args[i], XmNdefaultButton, ok_button); i++;
  XtSetArg( args[i], XmNcancelButton, cancel_button); i++;
  XtSetValues( form, args, i);

  i=0;
  XtSetArg( args[i], XmNbottomAttachment, XmATTACH_WIDGET); i++;
  XtSetArg( args[i], XmNbottomWidget, ok_button); i++;
  XtSetArg( args[i], XmNbottomOffset, 15); i++;
  XtSetArg( args[i], XmNrightAttachment, XmATTACH_FORM); i++;
  XtSetArg( args[i], XmNrightOffset, 15); i++;
  XtSetArg( args[i], XmNtopAttachment, XmATTACH_FORM); i++;
  XtSetArg( args[i], XmNtopOffset, 15); i++;
  XtSetArg( args[i], XmNleftAttachment, XmATTACH_FORM); i++;
  XtSetArg( args[i], XmNleftOffset, 15); i++;
  XtSetArg( args[i], XmNselectionPolicy, XmSINGLE_SELECT); i++;
  XtSetArg( args[i], XmNfontList, fontlist); i++;
  ctx->list = XmCreateScrolledList( form, (char*) "scrolledList", args, i);
  XtAddCallback( ctx->list, XmNdefaultActionCallback,
		(XtCallbackProc) list_action_cb, ctx);

  XmFontListFree( fontlist);
  XtManageChild( ctx->list);

  name_p = (char *)texts;
  i = 0;
  while ( strcmp( name_p, "") != 0) {
    cstr = XmStringCreateSimple( name_p);
    XmListAddItemUnselected( ctx->list, cstr, 0);
    XmStringFree(cstr);	  
    name_p += textsize;
    i++;
  }

  ctx->texts = (char *) calloc( i+1, textsize);
  ctx->textsize = textsize;
  memcpy( ctx->texts, texts, (i+1) * textsize); 
  XtPopup( ctx->toplevel, XtGrabNone);

  // Set input focus to the scrolled list widget
  XmProcessTraversal( ctx->list, XmTRAVERSE_CURRENT);


  return ctx;
}


void CoWowMotif::file_ok_cb( Widget widget, XtPointer udata, XtPointer data)
{
  XmFileSelectionBoxCallbackStruct *cbs = (XmFileSelectionBoxCallbackStruct *) data;
  wow_tFileSelCtx ctx = (wow_tFileSelCtx) udata;
  char *filename;

  if ( !XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &filename))
    return;

  if ( !filename)
    return;
  
  if ( ctx->file_selected_cb)
    (ctx->file_selected_cb)( ctx->parent_ctx, filename, ctx->file_type);
  free( (char *)ctx);
  XtDestroyWidget( widget);
  XtFree( filename);
}

void CoWowMotif::file_cancel_cb( Widget widget, XtPointer udata, XtPointer data)
{
  free( (char *)udata);
  XtDestroyWidget( widget);
}

void CoWowMotif::file_search_cb( Widget widget, XtPointer data)
{
  XmFileSelectionBoxCallbackStruct *cbs = (XmFileSelectionBoxCallbackStruct *) data;
  XmString names[WOW_MAXNAMES];
  char *mask;
  char found_file[200];
  int sts;
  int file_cnt;
  int i;

  if ( !XmStringGetLtoR( cbs->mask, XmFONTLIST_DEFAULT_TAG, &mask))
    return;

  file_cnt = 0;
  sts = dcli_search_file( mask, found_file, DCLI_DIR_SEARCH_INIT);
  while( ODD(sts)) {
    if ( file_cnt >= WOW_MAXNAMES)
      break;
    names[file_cnt++] = XmStringCreateLocalized( found_file);
    sts = dcli_search_file( mask, found_file, DCLI_DIR_SEARCH_NEXT);
  }
  sts = dcli_search_file( mask, found_file, DCLI_DIR_SEARCH_END);
  XtFree( mask);

  if ( file_cnt) {
    XtVaSetValues( widget, 
		   XmNfileListItems, names,
		   XmNfileListItemCount, file_cnt,
		   XmNdirSpec, names[0],
		   XmNlistUpdated, True,
		   NULL);
    for ( i = 0; i < file_cnt; i++)
      XmStringFree( names[i]);
  }
  else
    XtVaSetValues( widget,
		   XmNfileListItems, NULL,
		   XmNfileListItemCount, 0,
		   XmNlistUpdated, True,
		   NULL);
}

void CoWowMotif::CreateFileSelDia( const char *title, void *parent_ctx,
				   void (*file_selected_cb)(void *, char *, wow_eFileSelType),
				   wow_eFileSelType file_type)
{
  Arg args[10];
  XmString ctitle, cdirectory, cpattern;
  char directory[200];
  int i;
  wow_tFileSelCtx ctx;
  Widget help;

  ctx = (wow_tFileSelCtx) calloc( 1, sizeof(*ctx));
  ctx->file_selected_cb = file_selected_cb;
  ctx->parent_ctx = parent_ctx;
  ctx->file_type = file_type;

  ctitle = XmStringCreateLtoR( (char*) title, XmSTRING_DEFAULT_CHARSET);    

  i = 0;
  XtSetArg( args[i], XmNdialogTitle, ctitle); i++;
  XtSetArg( args[i], XmNfileTypeMask, XmFILE_REGULAR); i++;
  XtSetArg( args[i], XmNfileSearchProc, file_search_cb); i++;

  switch( file_type) {
  case wow_eFileSelType_All:
    break;
  case wow_eFileSelType_Dbs:
    dcli_translate_filename( directory, "$pwrp_load/");
    cdirectory = XmStringCreateLtoR( directory, XmSTRING_DEFAULT_CHARSET);
    cpattern = XmStringCreateLtoR( (char*) "*.dbs", XmSTRING_DEFAULT_CHARSET);
    XtSetArg( args[i], XmNdirectory, cdirectory); i++;
    XtSetArg( args[i], XmNpattern, cpattern); i++;
    break;
  case wow_eFileSelType_Wbl:
  case wow_eFileSelType_WblClass:
    dcli_translate_filename( directory, "$pwrp_db/");
    cdirectory = XmStringCreateLtoR( directory, XmSTRING_DEFAULT_CHARSET);
    cpattern = XmStringCreateLtoR( (char*) "*.wb_load", XmSTRING_DEFAULT_CHARSET);
    XtSetArg( args[i], XmNdirectory, cdirectory); i++;
    XtSetArg( args[i], XmNpattern, cpattern); i++;
    break;
  default: ;
  }

  ctx->dialog = XmCreateFileSelectionDialog( m_parent, (char*) "fileseldia", args, i);
  XtAddCallback( ctx->dialog, XmNokCallback, file_ok_cb, ctx);
  XtAddCallback( ctx->dialog, XmNcancelCallback, file_cancel_cb, ctx);

  help = XmFileSelectionBoxGetChild( ctx->dialog, XmDIALOG_HELP_BUTTON);
  XtUnmanageChild( help);
  XtManageChild( ctx->dialog);

  XmStringFree( ctitle);
  XmStringFree( cdirectory);
  XmStringFree( cpattern);
}

void CoWowMotif::warranty_ok_cb( void *ctx, void *data)
{
}

void CoWowMotif::warranty_cancel_cb( void *ctx, void *data)
{
  exit(0);
}

void CoWowMotif::warranty_help_cb( void *ctx, void *data)
{
  ((CoWowMotif *)data)->DisplayLicense();
}

int CoWowMotif::DisplayWarranty()
{
    char    text[4000];
    Arg	    arg[12];
    Widget  question_widget;
    XmString CStr2, TitleStr, okstr, cancelstr, helpstr;
    wow_t_question_cb *cbdata;
    XmFontList fontlist;
    XFontStruct *font;
    XmFontListEntry fontentry;
    char 	title[80];
    FILE 	*fp;
    int 	i;
    char 	fname[256];

    // Display only once
    if ( HideWarranty())
      return 1;

    sprintf( fname, "$pwr_exe/%s/acceptlicense.txt", lng_get_language_str());
    dcli_translate_filename( fname, fname);

    fp = fopen( fname, "r");
    if ( !fp) {
      strcpy( fname, "$pwr_exe/en_us/acceptlicense.txt");
      dcli_translate_filename( fname, fname);
      fp = fopen( fname, "r");
      if ( !fp) return 1;
    }

    for ( i = 0; i < (int)sizeof(text) - 1; i++) {
      text[i] = fgetc( fp);
      if ( text[i] == EOF)
	break;
    }
    text[i] = 0;
    fclose( fp);

    strcpy( title, lng_translate("Accept License Terms"));

    // Set default fontlist
    font = XLoadQueryFont( XtDisplay(m_parent),
	      "-*-Helvetica-Bold-R-Normal--12-*-*-*-P-*-ISO8859-1");
    fontentry = XmFontListEntryCreate( (char*) "tag1", XmFONT_IS_FONT, font);
    fontlist = XmFontListAppendEntry( NULL, fontentry);
    XtFree( (char *)fontentry);
    
    CStr2 = XmStringCreateLtoR( text, XmSTRING_DEFAULT_CHARSET);
    TitleStr = XmStringCreateLtoR( title, XmSTRING_DEFAULT_CHARSET);    
    okstr = XmStringCreateLtoR( lng_translate( "I Accept"), XmSTRING_DEFAULT_CHARSET );    
    cancelstr = XmStringCreateLtoR( lng_translate( "Quit"), XmSTRING_DEFAULT_CHARSET );    
    helpstr = XmStringCreateLtoR( lng_translate( "Show License"), XmSTRING_DEFAULT_CHARSET );    
    XtSetArg(arg[0],XmNheight,75);
    XtSetArg(arg[1],XmNwidth,700);
    XtSetArg(arg[2],XmNmessageString, CStr2);
    XtSetArg(arg[3],XmNx,400);
    XtSetArg(arg[4],XmNy,300);
    XtSetArg(arg[5],XmNdialogTitle,TitleStr);
    XtSetArg(arg[6], XmNokLabelString, okstr);
    XtSetArg(arg[7], XmNcancelLabelString, cancelstr);
    XtSetArg(arg[8], XmNhelpLabelString, helpstr);
    XtSetArg(arg[9], XmNbuttonFontList, fontlist);
    XtSetArg(arg[10], XmNlabelFontList, fontlist);
    XtSetArg(arg[11], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);

    cbdata = (wow_t_question_cb *) XtCalloc( 1, sizeof(*cbdata));
    cbdata->questionbox_ok = warranty_ok_cb;
    cbdata->questionbox_cancel = warranty_cancel_cb;
    cbdata->questionbox_help = warranty_help_cb;
    cbdata->ctx = 0;
    cbdata->data = (void *)this;

    question_widget = XmCreateMessageDialog( m_parent,(char*) "questionDialog",arg, 12);
    XtAddCallback( question_widget, XmNokCallback,
		(XtCallbackProc) question_ok_cb, cbdata);
    XtAddCallback( question_widget, XmNcancelCallback, 
		(XtCallbackProc) question_cancel_cb, cbdata);
    XtAddCallback( question_widget, XmNhelpCallback, 
		(XtCallbackProc) question_help_cb, cbdata);

    XmStringFree( CStr2);
    XmStringFree( TitleStr);
    XmStringFree( okstr);
    XmStringFree( cancelstr);
    XmStringFree( helpstr);
    XmFontListFree( fontlist);
   
    XtManageChild( question_widget);	       
    return 1;
}

void CoWowMotif::DisplayLicense()
{
    char text[20000];
    Arg	    arg[11];
    Widget  question_widget;
    XmString CStr2, TitleStr, cancelstr;
    XmFontList fontlist;
    XFontStruct *font;
    XmFontListEntry fontentry;
    char title[80];
    Widget w;
    FILE *fp;
    char fname[200];
    int i;
    Widget wcancel;

    strcpy( title, lng_translate("License"));

    sprintf( fname, "$pwr_exe/%s/license.txt", lng_get_language_str());
    dcli_translate_filename( fname, fname);

    fp = fopen( fname, "r");
    if ( !fp) {
      strcpy( fname, "$pwr_exe/en_us/lincense.txt");
      dcli_translate_filename( fname, fname);
      fp = fopen( fname, "r");
      if ( !fp)
	return;
    }

    for ( i = 0; i < (int)sizeof(text) - 1; i++) {
      text[i] = fgetc( fp);
      if ( text[i] == EOF)
	break;
    }
    fclose( fp);
    text[i] = 0;

    // Set default fontlist
    font = XLoadQueryFont( XtDisplay(m_parent),
	      "-*-Helvetica-Bold-R-Normal--12-*-*-*-P-*-ISO8859-1");
    fontentry = XmFontListEntryCreate( (char*) "tag1", XmFONT_IS_FONT, font);
    fontlist = XmFontListAppendEntry( NULL, fontentry);
    XtFree( (char *)fontentry);

    CStr2 = XmStringCreateLtoR( (char*) "", XmSTRING_DEFAULT_CHARSET);
    TitleStr = XmStringCreateLtoR( title, XmSTRING_DEFAULT_CHARSET);    
    cancelstr = XmStringCreateLtoR( (char*) " Close ", XmSTRING_DEFAULT_CHARSET );    
    XtSetArg(arg[0],XmNheight,400);
    XtSetArg(arg[1],XmNwidth,600);
    XtSetArg(arg[2],XmNmessageString, CStr2);
    XtSetArg(arg[3],XmNx,400);
    XtSetArg(arg[4],XmNy,300);
    XtSetArg(arg[5],XmNdialogTitle,TitleStr);
    XtSetArg(arg[6], XmNcancelLabelString, cancelstr);
    XtSetArg(arg[7], XmNbuttonFontList, fontlist);
    XtSetArg(arg[8], XmNlabelFontList, fontlist);

    question_widget = XmCreateMessageDialog( m_parent,(char*) "questionDialog",arg,9);
    XmStringFree( CStr2);
    XmStringFree( TitleStr);
    XmStringFree( cancelstr);
    XmFontListFree( fontlist);
    wcancel = XmMessageBoxGetChild(question_widget, XmDIALOG_CANCEL_BUTTON);
   

    XtSetArg(arg[0], XmNscrollHorizontal, True);
    XtSetArg(arg[1], XmNscrollVertical, True);
    XtSetArg(arg[2], XmNeditMode, XmMULTI_LINE_EDIT);
    XtSetArg(arg[3], XmNeditable, False);
    XtSetArg(arg[4], XmNcursorPositionVisible, False);
    XtSetArg(arg[5], XmNrows, 30);
    XtSetArg(arg[6], XmNvalue, text);
    XtSetArg(arg[7], XmNfontList, fontlist);
    w = XmCreateScrolledText( question_widget, (char*) "text", arg, 7);
    XtVaSetValues( XtParent(w), 
		   XmNleftAttachment, XmATTACH_FORM,
		   XmNrightAttachment, XmATTACH_FORM,
		   XmNtopAttachment, XmATTACH_FORM,
		   XmNbottomAttachment, XmATTACH_WIDGET,
		   XmNbottomWidget, wcancel,
		   NULL);
    XtManageChild(w);

    w = XmMessageBoxGetChild(question_widget, XmDIALOG_OK_BUTTON);
    XtUnmanageChild( w);    
    
    w = XmMessageBoxGetChild(question_widget, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild( w);    

    XtManageChild( question_widget);	       
}


void CoWowMotif::GetAtoms( Widget w, Atom *graph_atom, Atom *objid_atom, Atom *attrref_atom)
{
  if ( graph_atom)
    *graph_atom = XInternAtom( flow_Display(w), "PWR_GRAPH", False); 
  if ( objid_atom)
    *objid_atom = XInternAtom( flow_Display(w), "PWR_OBJID", False); 
  if ( attrref_atom)
    *attrref_atom = XInternAtom( flow_Display(w), "PWR_ATTRREF", False); 
}

static void wow_get_selection_cb( Widget w, XtPointer clientdata, Atom *selection,
				   Atom *type, XtPointer value, unsigned long *len,
				   int *format)
{
  wow_sSelection *data = (wow_sSelection *)clientdata;

  if ( *len != 0 && value != NULL) {
    if ( *type == data->atom) {
      if ( *len > sizeof(data->str) - 1) {
	data->sts = 0;
	return;
      }
      strncpy( data->str, (char *)value, *len);
      data->str[*len] = 0;
      data->len = *len;
      data->sts = 1;
    }
    else
      data->sts = 0;
  }
  else
    data->sts = 0;
  XtFree( (char *)value);
  data->received = 1;
}

int CoWowMotif::GetSelection( Widget w, char *str, int size, Atom atom)
{
  wow_sSelection data;

  data.received = 0;
  data.atom = atom;
  XtGetSelectionValue( w, XA_PRIMARY, atom,
		       wow_get_selection_cb, &data, CurrentTime);
  
  while( !data.received) {
    XEvent e;
    XtAppNextEvent( XtWidgetToApplicationContext(w), &e);
    XtDispatchEvent( &e);
  }
  if ( data.sts && data.len < size)
    strcpy( str, data.str);
  return data.sts;
}

CoWowTimer *CoWowMotif::timer_new()
{
  return new CoWowTimerMotif( m_parent);
}

CoWowTimerMotif::~CoWowTimerMotif()
{
  if ( m_timerid)
    XtRemoveTimeOut( m_timerid);    
}

void CoWowTimerMotif::add( int time, void (* callback)(void *data), void *data)
{
  m_callback = callback;
  m_data = data;
  m_timerid = XtAppAddTimeOut( XtWidgetToApplicationContext(m_w), time,
			       (XtTimerCallbackProc)timer_cb, this);
}

void CoWowTimerMotif::remove()
{
  if ( m_timerid) {
    XtRemoveTimeOut( m_timerid);    
    m_timerid = 0;
  }
}

void CoWowTimerMotif::timer_cb( void *data)
{
  CoWowTimerMotif *timer = (CoWowTimerMotif *)data;
  timer->m_timerid = 0;
  (timer->m_callback)( timer->m_data);
}


void CoWowMotif::PopupPosition( Widget parent, int x_event, int y_event, int *x, int *y)
{
  short x0, y0, x1, y1;
  Arg args[2];
  Widget grandparent;

  x0 = (short) x_event;
  y0 = (short) y_event;
  grandparent = XtParent(parent);
  while( grandparent) {
    XtSetArg( args[0], XmNx, &x1);
    XtSetArg( args[1], XmNy, &y1);
    XtGetValues( parent, args, 2);
    if ( XtIsShell( parent))
      break;
    x0 += x1;
    y0 += y1;
    parent = grandparent;
    grandparent = XtParent( parent);
  }
  *x = x0;
  *y = y0;
}

//
//  FocusTimer
//
void CoWowFocusTimerMotif::disable( Widget w, int time) 
{
  set_focus_disabled++;
  if ( request_cnt > 1)
    request_cnt = 0;
  focus_timerid = XtAppAddTimeOut( XtWidgetToApplicationContext(w), time,
				   enable_set_focus, this);
}

int CoWowFocusTimerMotif::disabled() {
  // Return false on first request after disable
  request_cnt++;
  return (request_cnt > 1);
}
    
void CoWowFocusTimerMotif::enable_set_focus( void *ft, XtIntervalId *id) 
{
  ((CoWowFocusTimerMotif *)ft)->set_focus_disabled--;
  ((CoWowFocusTimerMotif *)ft)->request_cnt = 0;
}

CoWowFocusTimerMotif::~CoWowFocusTimerMotif() 
{
  if ( set_focus_disabled)
    XtRemoveTimeOut( focus_timerid);
}











