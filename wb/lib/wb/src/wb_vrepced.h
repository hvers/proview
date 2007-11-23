/* 
 * Proview   $Id: wb_vrepced.h,v 1.1 2007-11-23 14:25:09 claes Exp $
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

#ifndef wb_vrepced_h
#define wb_vrepced_h

#include <string>

#include "wb_vrep.h"
#include "wb_erep.h"
#include "wb_attrname.h"
#include "wb_treeimport.h"
#include "co_dbs.h"
#include "co_tree.h"
#include "wb_import.h"

class wb_orepced;

class wb_vrepced : public wb_vrep
{
  wb_erep *m_erep;
  wb_merep *m_merep;
  unsigned int m_nRef;
  wb_vrep *m_vrep;
  int m_errorCount;
  tree_sTable *m_classbuild_th;
  tree_sTable *m_typebuild_th;

public:
  wb_vrepced( wb_erep *erep, wb_vrep *vrep) : 
    m_erep(erep), m_merep(erep->merep()), m_nRef(0), m_vrep(vrep) { 
    m_vid = m_vrep->vid();
    m_cid = m_vrep->cid();
  }

  ~wb_vrepced();

  virtual ldh_eVolRep type() const { return ldh_eVolRep_Ced;}
  pwr_tVid vid() const { return m_vid;}
  pwr_tCid cid() const { return m_cid;}

  wb_vrep *next() { return m_vrep->next();}

  virtual bool createSnapshot(const char *fileName, const pwr_tTime *time);

  virtual void unref();
  virtual wb_vrep *ref();
  virtual void name( const char *n);

  wb_erep *erep() {return m_vrep->erep();}
  wb_merep *merep() const { return m_vrep->merep();}

  virtual pwr_tOid oid(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->oid( sts, o);}
    
  virtual pwr_tVid vid(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->vid( sts, o);}
    
  virtual pwr_tOix oix(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->oix( sts, o);}
    

  virtual pwr_tCid cid(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->cid( sts, o);}
    
  virtual pwr_tOid poid(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->poid( sts, o);}
  virtual pwr_tOid foid(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->foid( sts, o);}
  virtual pwr_tOid loid(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->loid( sts, o);}
  virtual pwr_tOid boid(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->boid( sts, o);}
  virtual pwr_tOid aoid(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->aoid( sts, o);}
    
  virtual const char * objectName(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->objectName( sts, o);}
    
  virtual wb_name longName(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->longName( sts, o);}
    
  virtual pwr_tTime ohTime(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->ohTime( sts, o);}
  virtual pwr_tTime rbTime(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->rbTime( sts, o);}
  virtual pwr_tTime dbTime(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->dbTime( sts, o);}
  virtual pwr_mClassDef flags(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->flags( sts, o);}    
    
  virtual bool isOffspringOf(pwr_tStatus *sts, const wb_orep *child, const wb_orep *parent) { return m_vrep->isOffspringOf( sts, child, parent);}
    

  wb_orep *object(pwr_tStatus *sts) { return m_vrep->object( sts);}
  wb_orep *object(pwr_tStatus *sts, pwr_tOid oid) { return m_vrep->object( sts, oid);}
  wb_orep *object(pwr_tStatus *sts, pwr_tCid cid) { return m_vrep->object( sts, cid);}
  wb_orep *object(pwr_tStatus *sts, wb_name &name) { return m_vrep->object( sts, name);}
  wb_orep *object(pwr_tStatus *sts, const wb_orep *parent, wb_name &name) { return m_vrep->object( sts, parent, name);}

  wb_orep *createObject(pwr_tStatus *sts, wb_cdef cdef, wb_destination &d, wb_name &name,
			pwr_tOix oix = 0);

  wb_orep *copyObject(pwr_tStatus *sts, const wb_orep *orep, wb_destination &d, wb_name &name);
  bool copyOset(pwr_tStatus *sts, wb_oset *oset, wb_destination &d) { return m_vrep->copyOset( sts, oset, d);}

  bool moveObject(pwr_tStatus *sts, wb_orep *orep, wb_destination &d);

  bool deleteObject(pwr_tStatus *sts, wb_orep *orep);
  bool deleteFamily(pwr_tStatus *sts, wb_orep *orep);
  bool deleteOset(pwr_tStatus *sts, wb_oset *oset) { return m_vrep->deleteOset( sts, oset);}

  bool renameObject(pwr_tStatus *sts, wb_orep *orep, wb_name &name);


  bool commit(pwr_tStatus *sts);
  bool abort(pwr_tStatus *sts);

  virtual bool writeAttribute(pwr_tStatus *sts, wb_orep *o, pwr_eBix bix, size_t offset, size_t size, void *p);

  virtual void *readAttribute(pwr_tStatus *sts, const wb_orep *o, pwr_eBix bix, size_t offset, size_t size, void *p) { return m_vrep->readAttribute( sts, o, bix, offset, size, p);}

  virtual void *readBody(pwr_tStatus *sts, const wb_orep *o, pwr_eBix bix, void *p) { return readBody( sts, o, bix, p);}

  virtual bool writeBody(pwr_tStatus *sts, wb_orep *o, pwr_eBix bix, void *p);


  wb_orep *ancestor(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->ancestor( sts, o);}

  wb_orep *parent(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->parent( sts, o);}

  wb_orep *after(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->after( sts, o);}

  wb_orep *before(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->before( sts, o);}

  wb_orep *first(pwr_tStatus *sts, const wb_orep *o)  { return m_vrep->first( sts, o);}

  wb_orep *child(pwr_tStatus *sts, const wb_orep *o, wb_name &name) { return m_vrep->child( sts, o, name);}

  wb_orep *last(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->last( sts, o);}

  wb_orep *next(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->next( sts, o);}

  wb_orep *previous(pwr_tStatus *sts, const wb_orep *o) { return m_vrep->previous( sts, o);}

  wb_srep *newSession() {return m_vrep->newSession();}

  bool isLocal(const wb_orep *o) { return m_vrep->isLocal( o);}

  void objectName(const wb_orep *o, char *str) { m_vrep->objectName( o, str);}

  virtual bool exportVolume(wb_import &i) { return m_vrep->exportVolume( i);}
  virtual bool exportHead(wb_import &i) { return m_vrep->exportHead( i);}
  virtual bool exportRbody(wb_import &i) { return m_vrep->exportRbody( i);}
  virtual bool exportDbody(wb_import &i) { return m_vrep->exportDbody( i);}
  virtual bool exportDocBlock(wb_import &i) { return m_vrep->exportDocBlock( i);}
  virtual bool exportMeta(wb_import &i) { return m_vrep->exportMeta( i);}
  virtual bool exportTree(wb_treeimport &i, pwr_tOid oid) { return m_vrep->exportTree( i, oid);}
  virtual bool importTreeObject(wb_merep *merep, pwr_tOid oid, pwr_tCid cid, pwr_tOid poid,
				pwr_tOid boid, const char *name, pwr_mClassDef flags,
				size_t rbSize, size_t dbSize, void *rbody, void *dbody) {
    return m_vrep->importTreeObject( merep, oid, cid, poid, boid, name, flags, rbSize, dbSize, rbody, dbody);}
  virtual bool importTree( bool keepref) { return m_vrep->importTree( keepref);}
  virtual bool importPasteObject(pwr_tOid destination, ldh_eDest destcode,
				 bool keepoid, pwr_tOid oid, 
				 pwr_tCid cid, pwr_tOid poid,
				 pwr_tOid boid, const char *name, pwr_mClassDef flags,
				 size_t rbSize, size_t dbSize, void *rbody, void *dbody,
				 pwr_tOid *roid) {
    return m_vrep->importPasteObject( destination, destcode, keepoid, oid, cid, poid, boid, name, flags,
			       rbSize, dbSize, rbody, dbody, roid);}
  virtual bool importPaste() { return m_vrep->importPaste();}
  virtual void importIgnoreErrors() { return m_vrep->importIgnoreErrors();}
  virtual bool accessSupported( ldh_eAccess access) { return m_vrep->accessSupported( access);}
  virtual const char *fileName() { return m_vrep->fileName();}

 private:
  bool nextCix( pwr_tStatus *sts, pwr_tOix *cix);
  bool nextTix( pwr_tStatus *sts, pwr_tOix *tix);
  bool nextAix( pwr_tStatus *sts, wb_orep *co, pwr_tOix *aix);
  bool buildType( pwr_tStatus *sts, wb_orep *to);
  bool buildClass( pwr_tStatus *sts, wb_orep *co);
  void error( char *msg, wb_orep *o);
  bool classeditorCheck( ldh_eDest dest_code, wb_orep *dest, pwr_tCid cid,
			 pwr_tOix *oix, char *name, pwr_tStatus *sts, 
			 bool import_paste);
  void printPaletteFile();

};

#endif








