/* ARec.c

   An array of records is a Tcl data structure which is designed to allow
   the exchange of an array of structures with lower level C routines
   with very little overhead.  It allows the data to be stored
   in a format that is easily accessed at the C level and does not require
   the low level C routines to continually interact with the Tcl object API.

*/
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>

#include <tcl.h>
#include "arec.h"

ARecType  *ARecTypeType = NULL;
ARecField *ARecTypeInst = NULL;


static Tcl_ObjType *TclDictType;
static Tcl_ObjType *TclListType;

static Tcl_CmdInfo ARecTypedefInfo;


int   ARecTypeFields   (Tcl_Interp *ip, ARecType *type, int types, int fields, int offset, int objc, Tcl_Obj **objv);
int   ARecTypeAddField (Tcl_Interp *ip, ARecType *type, int objc, Tcl_Obj **objv);
void  ARecTypeAddField1(ARecType *type, Tcl_Obj *nameobj, int length, int array, ARecType *field);
void *ARecRealloc(ARecField *inst, int nrecs, int more);
int   ARecRange(Tcl_Interp *ip, ARecField *inst, int *objc, Tcl_Obj ***objv, int *n, int *m, int *islist);

Tcl_Obj *ARecGetDouble(Tcl_Interp *ip, ARecType *type, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) { return Tcl_NewDoubleObj(*((double *) here)); }
Tcl_Obj *ARecGetFloat( Tcl_Interp *ip, ARecType *type, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) { return Tcl_NewDoubleObj(*((float  *) here)); }
Tcl_Obj *ARecGetULong( Tcl_Interp *ip, ARecType *type, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) { return Tcl_NewLongObj(  *((unsigned long   *) here)); }
Tcl_Obj *ARecGetLong(  Tcl_Interp *ip, ARecType *type, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) { return Tcl_NewLongObj(  *((long   *) here)); }
Tcl_Obj *ARecGetUInt(  Tcl_Interp *ip, ARecType *type, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) { return Tcl_NewLongObj(  *((unsigned int  *) here)); }
Tcl_Obj *ARecGetInt(   Tcl_Interp *ip, ARecType *type, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) { return Tcl_NewIntObj(   *((int    *) here)); }
Tcl_Obj *ARecGetUShort(Tcl_Interp *ip, ARecType *type, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) { return Tcl_NewIntObj(   *((unsigned short *) here)); }
Tcl_Obj *ARecGetShort( Tcl_Interp *ip, ARecType *type, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) { return Tcl_NewIntObj(   *((short  *) here)); }
Tcl_Obj *ARecGetUChar( Tcl_Interp *ip, ARecType *type, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) { return Tcl_NewIntObj(   *((char   *) here)); }
Tcl_Obj *ARecGetChar(  Tcl_Interp *ip, ARecType *type, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) { return Tcl_NewIntObj(   *((unsigned char *) here)); }
Tcl_Obj *ARecGetString(Tcl_Interp *ip, ARecType *type, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) { 
    if ( *((char  **) here) == NULL ) { return Tcl_NewStringObj("(null)", -1); }

    return Tcl_NewStringObj(*((char  **) here), -1);
}
Tcl_Obj *ARecGetTclObj(Tcl_Interp *ip, ARecType *type, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) {
    if ( *((char  **) here) == NULL ) { return Tcl_NewStringObj("(null)", -1); }

    return *((Tcl_Obj  **) here);
}

int ARecSetDouble(Tcl_Interp *ip, ARecType *type, Tcl_Obj *obj, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) {
    if ( !obj ) {
	 *((double *)here) = 0.0;
	return TCL_OK;
    }

    return Tcl_GetDoubleFromObj(NULL, obj, (double *) here);
}
int ARecSetFloat( Tcl_Interp *ip, ARecType *type, Tcl_Obj *obj, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) {
    	double dbl;

    if ( !obj ) {
	*((float *)here) = 0.0;
	return TCL_OK;
    }

    if ( Tcl_GetDoubleFromObj( NULL, obj, &dbl) == TCL_ERROR ) { return TCL_ERROR; }
    *((float *)here) = dbl;

    return TCL_OK;
}
int ARecSetULong(  Tcl_Interp *ip, ARecType *type, Tcl_Obj *obj, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) {
    double value;

    if ( !obj ) {
	*((unsigned long *)here) = 0;
	return TCL_OK;
    }

    int ret = Tcl_GetDoubleFromObj(NULL, obj, &value);
    *((unsigned long  *) here) = (unsigned long) value;

    return ret;
}
int ARecSetLong(  Tcl_Interp *ip, ARecType *type, Tcl_Obj *obj, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) {
    double value;

    if ( !obj ) {
	*((long *)here) = 0;
	return TCL_OK;
    }

    int ret = Tcl_GetDoubleFromObj(NULL, obj, &value);
    *((long  *) here) = (long) value;

    return ret;
}
int ARecSetUInt(  Tcl_Interp *ip, ARecType *type, Tcl_Obj *obj, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) {
    double value;

    if ( !obj ) {
	*((unsigned int *)here) = 0;
	return TCL_OK;
    }

    int ret = Tcl_GetDoubleFromObj(NULL, obj, &value);
    *((unsigned int *) here) = (unsigned int) value;

    return ret;
}
int ARecSetInt(   Tcl_Interp *ip, ARecType *type, Tcl_Obj *obj, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) {
    double value;

    if ( !obj ) {
	*((int *)here) = 0;
	return TCL_OK;
    }

    int ret = Tcl_GetDoubleFromObj(NULL, obj, &value);
    *((int    *) here) = (int) value;

    return ret;
}
int ARecSetUShort(Tcl_Interp *ip, ARecType *type, Tcl_Obj *obj, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) {
    	int i;

    if ( !obj ) {
	*((unsigned short *)here) = 0;
	return TCL_OK;
    }

    if ( Tcl_GetIntFromObj( NULL, obj, &i) == TCL_ERROR ) { return TCL_ERROR; }
    *((unsigned short *)here) = i;

    return TCL_OK;
}
int ARecSetShort( Tcl_Interp *ip, ARecType *type, Tcl_Obj *obj, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) {
    	int i;

    if ( !obj ) {
	*((short *)here) = 0;
	return TCL_OK;
    }

    if ( Tcl_GetIntFromObj( NULL, obj, &i) == TCL_ERROR ) { return TCL_ERROR; }
    *((short *)here) = i;

    return TCL_OK;
}
int ARecSetUChar(Tcl_Interp *ip, ARecType *type, Tcl_Obj *obj, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) {
    	int i;

    if ( !obj ) {
	*((unsigned char *)here) = 0;
	return TCL_OK;
    }

    if ( Tcl_GetIntFromObj( NULL, obj, &i) == TCL_ERROR ) { return TCL_ERROR; }
    *((unsigned char *)here) = i;

    return TCL_OK;
}
int ARecSetChar(Tcl_Interp *ip, ARecType *type, Tcl_Obj *obj, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) {
    	int i;

    if ( !obj ) { 
	*((char *)here) = 0;
	return TCL_OK;
    }

    if ( Tcl_GetIntFromObj( NULL, obj, &i) == TCL_ERROR ) { return TCL_ERROR; }
    *((char *)here) = i;

    return TCL_OK;
}
int ARecSetString(Tcl_Interp *ip, ARecType *type, Tcl_Obj *obj, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) {
    	char *str = *((char **)here);

    if ( str ) { free((void *) str); }

    if ( !obj ) {
	*((char **)here) = NULL;
	return TCL_OK;
    }

    *(char **) here = strdup(Tcl_GetString(obj));

    return TCL_OK;
}
int ARecSetTclObj(Tcl_Interp *ip, ARecType *type, Tcl_Obj *obj, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) {
    	Tcl_Obj *old = *((Tcl_Obj **)here);

    if ( old ) { Tcl_DecrRefCount(old); }
    
    if ( !obj ) {
	*((Tcl_Obj **)here) = NULL;
	return TCL_OK;
    }

    *(Tcl_Obj **) here = obj;

    if ( obj ) { Tcl_IncrRefCount(obj); }

    return TCL_OK;
}

ARecField *ARecLookupField(int n, ARecField *table, Tcl_Obj *nameobj)
{
    char *name = Tcl_GetString(nameobj);

    for ( ; n--; table++ ) {
	if ( !strcmp(Tcl_GetString(table->nameobj), name) ) {
	    return table;
	}
    }

    return NULL;
}

int ARecSetField(Tcl_Interp *ip, ARecField *table, char *record, Tcl_Obj *obj) {
	return table == NULL ? TCL_ERROR : table->type->set(ip, table->type, obj, record + table->offset, 0, 0, NULL, 0);
}

int ARecSetStruct  (Tcl_Interp *ip, ARecType *type, Tcl_Obj *obj, void *here, int m, int objc, Tcl_Obj*const* objv, int flags) {
    if ( flags & AREC_ISLIST ) {
	int i;

	if ( Tcl_ListObjGetElements(ip, obj, &objc, (Tcl_Obj***) &objv) == TCL_ERROR ) {
	    return TCL_ERROR;
	}

	for ( i = 0; i < m; i++ ) {
	    if ( ARecSetFromDict(ip, type, here, 1, 1, &objv[i%objc]) == TCL_ERROR ) {
		return TCL_ERROR;
	    }
	    here += type->size;
	}
    } else {
	return ARecSetFromDict(ip, type, here, 1, 1, &obj);
    }

    return TCL_OK;
}

ARecType *ARecLookupType(Tcl_Obj *nameobj)
{
    int i;

    char *name = Tcl_GetString(nameobj);

    ARecType *types = ARecTypeInst->recs;

    for ( i = 0; i < ARecTypeInst->nrecs; i++ ) {
	if ( !strcmp(Tcl_GetString(types[i].nameobj), name) ) {
	    return &types[i];
	}
    }

    return NULL;
}


int ARecDelInst(ClientData data)
{
        ARecField *inst = (ARecField *) data;

    Tcl_DecrRefCount(inst->nameobj);

    Tcl_Free((void *) inst->recs);
    Tcl_Free((void *) inst);

    return TCL_OK;
}

int ARecDelType(ClientData data)
{
    int i;
    ARecType *type = (ARecType *) data;

    type = ARecLookupType(type->nameobj);


    Tcl_DecrRefCount(type->nameobj);

    for ( i = 0; i < type->nfield; i++ ) { Tcl_DecrRefCount(type->field[i].nameobj); }

    Tcl_Free((void *) type->shadow);
    Tcl_Free((void *) type->field);

    ARecInstDeleteRecs(ARecTypeInst, (char *) type, 1);

    return TCL_OK;
}

int ARecTypeObjCmd(data, ip, objc, objv)
    ClientData       data;
    Tcl_Interp      *ip;
    int              objc;
    Tcl_Obj        **objv;
{
    ARecType *type = (ARecType *) data;
    Tcl_Obj *result = Tcl_GetObjResult(ip);

    ARecCmd(ip, type, "create", " ?inst? ...", objc >= 3, objc, objv,
	return ARecNewInst(ip, objc, objv, data);
    );
    ARecCmd(ip, type, "size", "", objc >= 1, objc, objv,
	Tcl_SetObjResult(ip, Tcl_NewIntObj(type->size));	

	return TCL_OK;
    );

    ARecCmd(ip, type, "types",   " ?field? ...", objc >= 2, objc, objv, return ARecTypeFields(ip, type, 1, 0, 0, objc-2, objv+2); );
    ARecCmd(ip, type, "names",   " ?field? ...", objc >= 2, objc, objv, return ARecTypeFields(ip, type, 0, 1, 0, objc-2, objv+2); );
    ARecCmd(ip, type, "fields",  " ?field? ...", objc >= 2, objc, objv, return ARecTypeFields(ip, type, 1, 1, 0, objc-2, objv+2); );
    ARecCmd(ip, type, "offsets", " ?field? ...", objc >= 2, objc, objv, return ARecTypeFields(ip, type, 0, 0, 1, objc-2, objv+2); );

    ARecCmd(ip, type, "add-field", " type name", objc >= 4, objc, objv,
	return ARecTypeAddField(ip, type, objc, objv);
    );

    return TCL_ERROR;
}

ARecType *ARecTypeCreate(Tcl_Interp *ip, Tcl_Obj *name, int stype)
{
    ARecType *type = ARecTypeAddType(ARecTypeInst, name, 0, 1, stype, ARecSetStruct, ARecGetStruct);

    //printf("Create a command %s\n", Tcl_GetString(name));

    Tcl_CreateObjCommand(ip, Tcl_GetString(name)
	, ARecTypeObjCmd
	, (ClientData) type->shadow
	, (Tcl_CmdDeleteProc *) ARecDelType);

    return type;
}

int ARecTypeCreateObjCmd(Tcl_Interp *ip, int objc, Tcl_Obj *const *objv)
{
	Tcl_Obj     *result = Tcl_GetObjResult(ip);

    if ( objc != 3 ) {
	Tcl_SetStringObj(result, "TypeCreate type stype", -1);
	return TCL_ERROR;
    }

    if ( ARecLookupType(objv[1]) != NULL ) {
	Tcl_SetStringObj(result, "An arec::typedef of this name already exists", -1);
	return TCL_ERROR;
    }

    ARecTypeCreate(ip, objv[1], strcmp(Tcl_GetString(objv[2]), "union") ? AREC_STRUCT : AREC_UNION);

    Tcl_SetObjResult(ip, objv[1]);	

    return TCL_OK;
}

int ARecInstDeleteRecs(ARecField *inst, char *recs, int m)
{
    memmove(recs, recs+m*inst->type->size,  (inst->nrecs - m) * inst->type->size - ((recs- (char *) inst->recs)));
    inst->nrecs -= m;

    return TCL_OK;
}


int ARecCallAction(Tcl_Interp *ip, ARecPath *path, int npath, Tcl_Obj **objv, int objc, Tcl_ObjCmdProc *action, int top, int slice) {
    int j;
    int      reply;

    if ( npath == 1 ) {
	reply = action(path, ip, objc, objv);
    } else {

	Tcl_Obj *result = Tcl_NewObj();

	if ( path[0].array ) {
	    int	  	  slicec;
	    Tcl_Obj	**slicev;

	    if ( objc && slice ) {
		if ( Tcl_ListObjGetElements(ip, objv[objc-1], &slicec, &slicev) == TCL_ERROR ) {
		    return TCL_ERROR;
		}
	    } 

	    for ( j = path[0].first; j <= path[0].last; j++ ) {

		if ( objc && slice ) {
		    objv[objc-1] = slicev[j % slicec];
		}

		path[1].recs = path[0].recs + path[0].inst->type->size * j + path[1].inst->offset;

		switch ( (reply = ARecCallAction(ip, &path[1], npath-1, objv, objc, action, 0, slice)) ) {
		 case TCL_ERROR: return TCL_ERROR;
		 case AREC_TCL_VALUE:

		    Tcl_ListObjAppendElement(ip, result, Tcl_GetObjResult(ip));
		}
	    }

	    Tcl_SetObjResult(ip, result);

	} else {
	    path[1].recs = path[0].recs + path[0].inst->type->size * path[0].first + path[1].inst->offset;

	    if ( (reply = ARecCallAction(ip, &path[1], npath-1, objv, objc, action, 0, slice)) == TCL_ERROR ) {
		return TCL_ERROR;
	    }
	}
    }

    if ( top ) {
	switch ( reply ) {
	 case TCL_OK:
	 case AREC_TCL_VALUE: return TCL_OK;
	 default: 	 	  return reply;
	}
    } else {
	return reply;
    }
}

int ARecSetDictAction(void *data, Tcl_Interp *ip, int objc, Tcl_Obj *const*objv) {
    ARecPath *path = (ARecPath *) data;

    if ( objc == 1 ) {
	return ARecSetStruct(ip, path->inst->type, objv[0], path->recs+path->first*path->inst->type->size, path->last-path->first+1, 0, NULL, path->array ? AREC_ISLIST : 0);
    } else {
	return ARecSetFromArgs(ip, path->inst->type, path->recs+path->first*path->inst->type->size, path->last-path->first+1, objc, objv, 0);
    }
}

int ARecSetListAction(void *data, Tcl_Interp *ip, int objc, Tcl_Obj *const*objv) {
    ARecPath *path = (ARecPath *) data;

    if ( objc == 1 ) {
	return ARecSetFromList(ip, path->inst->type, path->recs+path->first*path->inst->type->size, path->last-path->first+1, 1, objv, path->array ? AREC_ISLIST : 0);
    } else {
	return ARecSetFromArgs(ip, path->inst->type, path->recs+path->first*path->inst->type->size, path->last-path->first+1, objc, objv, 1);
    }
}

int ARecGetDictAction(void *data, Tcl_Interp *ip, int objc, Tcl_Obj *const*objv) {
    Tcl_Obj *reply;
    ARecPath *path = (ARecPath *) data;

    if ( !(reply = ARecGetStruct(ip, path->inst->type, path->recs+path->first*path->inst->type->size, path->last-path->first+1, objc, objv, ( path->array ? AREC_ISLIST : 0 ) | AREC_ASDICT)) ) {
	return TCL_ERROR;
    }

    Tcl_SetObjResult(ip, reply);
    return AREC_TCL_VALUE;
}

int ARecGetListAction(void *data, Tcl_Interp *ip, int objc, Tcl_Obj *const*objv) {
    Tcl_Obj *reply;
    ARecPath *path = (ARecPath *) data;

    if ( !(reply = ARecGetStruct(ip, path->inst->type, path->recs+path->first*path->inst->type->size, path->last-path->first+1, objc, objv, path->array ? AREC_ISLIST : 0)) ) {
	return TCL_ERROR;
    }

    Tcl_SetObjResult(ip, reply);
    return AREC_TCL_VALUE;
}


int ARecInstObjCmd(data, ip, objc, objv)
    ClientData       data;
    Tcl_Interp      *ip;
    int              objc;
    Tcl_Obj        **objv;
{
    objc--; objv++;
    Tcl_Obj   *result = Tcl_GetObjResult(ip);

    if ( objc == 0 ) {
	Tcl_AppendStringsToObj(result , " no action ", NULL);

	return TCL_ERROR;
    }

    ARecField *inst   = (ARecField *) data;
    Tcl_Obj   *actionObj = (objv++)[0]; 		objc--;

    ARecPath   path[10];
    int        npath = 0;
    int		i;

    ARecField	*this = inst;
    ARecField	*next;

    int		 slice = 0;



    // Parse any selection / iteration path
    //
    for ( i = 0; i < 10 && objc >= 0; i++ ) {
	path[npath].inst  = this;
	path[npath].recs  = this->recs;
	path[npath].first = 0;
	path[npath].last  = this->nrecs-1;
	path[npath].array = this->arecs >= 0;

	npath++;

	if ( !objc ) 							 { break; }
	if ( objv[0]->typePtr == TclListType ) 				 { break; }
	if ( objv[0]->typePtr == TclDictType ) 				 { break; }

	if ( this->arecs >= 0 ) {
	    if ( ARecRange(ip, inst, &objc, &objv, &path[npath-1].first, &path[npath-1].last, &path[npath-1].array) == TCL_ERROR ) {
		return TCL_ERROR;
	    }

	    if ( path[npath-1].first < 0 || path[npath-1].last > this->nrecs )  {
		char index[50];
		snprintf(index, 50, "%d %d : %d", path[npath-1].first, path[npath-1].last, this->nrecs);

		Tcl_AppendStringsToObj(result, Tcl_GetString(inst->nameobj), " : index out of range, path ", index, NULL);

		return TCL_ERROR;
	    }
	} 



	if ( !objc ) 							 { break; }
	if ( objv[0]->typePtr == TclListType ) 				 { break; }
	if ( objv[0]->typePtr == TclDictType ) 				 { break; }
        if ( !strcmp(Tcl_GetString(objv[0]), "=") ) { objv++; objc--;      break; }
        if ( !strcmp(Tcl_GetString(objv[0]), ":") ) { objv++; objc--;      break; }
        if ( !strcmp(Tcl_GetString(objv[0]), "/") ) { objv++; objc--; slice++;  break; }

        if ( !(next = ARecLookupField(this->type->nfield, this->type->field, objv[0])) ) {
		break;
	} 

	if ( !next->type->nfield ) 				 { break; }
	objc--; objv++;

	this = next;
    }

    if ( i >= 10 ) { fprintf(stderr, "huh?\n"); exit(1); }

    
    char *actionName = Tcl_GetString(actionObj);

    if ( !strcmp(actionName, "length") && ( objc == 0 || objc == 1 ) ) {

	int n;

	if ( objc == 1 ) {
	    if ( npath > 1 ) {
		Tcl_AppendStringsToObj(result , Tcl_GetString(inst->type->nameobj), " cannot set inner length ", NULL);
		
		return TCL_ERROR;
	    }

	    if ( Tcl_GetIntFromObj(ip, objv[0], &n) != TCL_OK  ) { return TCL_ERROR; }

	    ARecRealloc(this, n, 0);
	    inst->nrecs = n;
	}

	Tcl_SetIntObj(result, this->nrecs);

	return TCL_OK;
    }

    if ( !strcmp(actionName, "type") && objc >= 1 ) {
	ARecTypeObjCmd(this->type, ip, ++objc, --objv);	

	return TCL_OK;
    }

    if ( !strcmp(actionName, "size") && objc == 0 ) {
	Tcl_SetObjResult(ip, Tcl_NewIntObj(this->type->size));	

	return TCL_OK;
    }

    /* */  if ( !strcmp(actionName, "set")	) {
	path[0].recs = ARecRealloc(path[0].inst, path->last+1, 10);

	return ARecCallAction(ip, path, npath, objv, objc, ARecSetListAction, 1, slice);
    } else if ( !strcmp(actionName, "setdict")  ) {
	path[0].recs = ARecRealloc(path[0].inst, path->last+1, 10);

	return ARecCallAction(ip, path, npath, objv, objc, ARecSetDictAction, 1, slice);
    } else if ( !strcmp(actionName, "setlist")  ) {
	path[0].recs = ARecRealloc(path[0].inst, path->last+1, 10);

	return ARecCallAction(ip, path, npath, objv, objc, ARecSetListAction, 1, slice);
    }

    if ( path[0].first < 0 || + path[0].last >= path[0].inst->nrecs ) {			// All commands below here do not allow extension
	char index[50];
	snprintf(index, 50, " %d %d : %d ", path[0].first, path[0].last, path[0].inst->nrecs);

	Tcl_AppendStringsToObj(result , Tcl_GetString(inst->type->nameobj), " index out of range ", index, NULL);
	
	return TCL_ERROR;
    }


    /* */  if ( !strcmp(actionName, "get")  	) {
	return ARecCallAction(ip, path, npath, objv, objc, ARecGetListAction, 1, slice);
    } else if ( !strcmp(actionName, "getdict")  ) {
	return ARecCallAction(ip, path, npath, objv, objc, ARecGetDictAction, 1, slice);
    } else if ( !strcmp(actionName, "getlist")  ) {
	return ARecCallAction(ip, path, npath, objv, objc, ARecGetListAction, 1, slice);
    } else if ( !strcmp(actionName, "getbytes")  ) {
	Tcl_SetByteArrayObj(result, (unsigned char *) path->recs+path[npath-1].first*path[npath-1].inst->type->size
						   , (path[npath-1].last-path[npath-1].first+1)*path[npath-1].inst->type->size);

	return TCL_OK;
    } else if ( !strcmp(actionName, "setbytes")  ) {
	int           nbytes;
	unsigned char *bytes = Tcl_GetByteArrayFromObj(objv[0], &nbytes);


	if ( nbytes % path[npath-1].inst->type->size != 0
	 || ( npath > 1 
	     && nbytes != (path[npath-1].last-path[npath-1].first+1)*path[npath-1].inst->type->size ) 
	   ) {
	    Tcl_AppendStringsToObj(result , Tcl_GetString(path[npath-1].inst->type->nameobj), " set bytes with incompatible data block size ", NULL);

	    return TCL_ERROR;
	}

	if ( npath == 1 ) {
	    path[0].recs = ARecRealloc(path[0].inst, path[0].first+nbytes/path[0].inst->type->size, 0);
	}

	memcpy(path[npath-1].recs+path[npath-1].inst->type->size*path[npath-1].first, bytes, nbytes);

	return TCL_OK;
    } else if ( !strcmp(actionName, "getptr")  ) {
	Tcl_SetLongObj(result, (long) path->recs+path->first*path->inst->type->size);

	return TCL_OK;
    } else if ( !strcmp(actionName, "delete")  ) {
	return ARecInstDeleteRecs(inst, path->recs+this->type->size*path->first, path->first-path->last+1);
    } 

    {
	Tcl_CmdInfo cmdInfo;
	char *typeName = Tcl_GetString(inst->type->nameobj);

	char cmdName[256];

	snprintf(cmdName, 256, "%s::%s", typeName, actionName);

	if ( ARecTypedefInfo.deleteProc == NULL ) {
	    if ( !Tcl_GetCommandInfo(ip, "::arec::typedef", &ARecTypedefInfo) ) {
		Tcl_AppendStringsToObj(result , "no typedef info? ", NULL);
		return TCL_ERROR;
	    }
	}

	if ( !Tcl_GetCommandInfo(ip, cmdName, &cmdInfo) ) {
	    Tcl_AppendStringsToObj(result , "unknown method ", cmdName, " of type ", typeName, NULL);
	    return TCL_ERROR;
	}

	if ( !cmdInfo.isNativeObjectProc ) {
	    Tcl_AppendStringsToObj(result , cmdName, " must be an object proc.", NULL);
	    return TCL_ERROR;
	}

	if ( cmdInfo.deleteProc == ARecTypedefInfo.deleteProc ) {
	    Tcl_AppendStringsToObj(result , cmdName, " must be native command.", NULL);
	    return TCL_ERROR;
	}

	path[npath-1].clientData = cmdInfo.objClientData;

	objc++;
	objv--;

	Tcl_DecrRefCount(objv[0]);
	objv[0] = Tcl_NewStringObj(cmdName, -1);

	return ARecCallAction(ip, path, npath, objv, objc, cmdInfo.objProc, 1, slice);
    }
}

ARecField *ARecInstCreate(Tcl_Interp *ip, Tcl_Obj *nameobj, ARecType *type, int n)
{
	ARecField *inst;

    inst          = (ARecField *) Tcl_Alloc(sizeof(ARecField));
    inst->nameobj = nameobj;
    Tcl_IncrRefCount(inst->nameobj);

    inst->type    = type;
    inst->nrecs   = n;
    inst->arecs   = n;
    inst->recs    = Tcl_Alloc(type->size * inst->nrecs);

    memset(inst->recs, 0, type->size * inst->nrecs);

    Tcl_CreateObjCommand(ip, Tcl_GetString(nameobj)
	, ARecInstObjCmd
	, (ClientData) inst
	, (Tcl_CmdDeleteProc *) ARecDelInst);

    Tcl_SetObjResult(ip, nameobj);

    //inst->next = type->instances;
    type->instances = inst;

    return inst;
}

int ARecNewInst(Tcl_Interp *ip, int objc, Tcl_Obj **objv, ARecType *type)
{
	Tcl_Obj     *result = Tcl_GetObjResult(ip);
	int	     n = 1;

    if ( objc == 4 ) {
	if ( Tcl_GetIntFromObj(ip, objv[3], &n) != TCL_OK  ) {
	    Tcl_SetStringObj(result, "cannot convert size arg to int", -1);				\
	    return TCL_ERROR;
	}
    }

    ARecInstCreate(ip, objv[2], type, n);

    return TCL_OK;
}

ARecField **ARecFieldMap(Tcl_Obj *result, int objc, Tcl_Obj *const*objv, ARecType *type, int *nmap)
{
    int 	i;
    int		max = objc > type->nfield ? objc : type->nfield;
    ARecField **map = (ARecField **) Tcl_Alloc(sizeof(ARecField *) * max);

    if ( !type->nfield ) { 
	Tcl_Free((char *) map);

	Tcl_AppendStringsToObj(result, Tcl_GetString(type->nameobj), " no fields in this type? ", NULL);
	return NULL;
    }

    if ( !objc ) {
	if ( type->stype == AREC_STRUCT ) { 
	    for ( i = 0; i < type->nfield; i++ ) {
		map[i] = &type->field[i];
	    }
	} else {
	    i = 1;
	    map[0] = &type->field[type->stype-1];
	}
    } else {
	for ( i = 0; i < objc; i++ ) {
	    if ( !(map[i] = ARecLookupField(type->nfield, type->field, objv[i])) ) {
		Tcl_Free((char *) map);

		Tcl_AppendStringsToObj(result, " in type \"", Tcl_GetString(type->nameobj), "\" cannot lookup field \"", Tcl_GetString(objv[i]), "\"", NULL);
		return NULL;
	    }
	}
    }

    *nmap = i;
    return map;
}

int ARecTypeFields(Tcl_Interp *ip, ARecType *type, int types, int fields, int offset, int objc, Tcl_Obj **objv)
{
    Tcl_Obj  	 *result = Tcl_NewObj();
    int i;

    ARecField **map;
    int	       nmap;

    if ( !(map = ARecFieldMap(result, objc, objv, type, &nmap)) ) {
	Tcl_SetObjResult(ip, result);
	return TCL_ERROR;
    }

    for ( i = 0; i < nmap; i++ ) {
	if ( types  ) { Tcl_ListObjAppendElement(ip, result , map[i]->type->nameobj); 	     }
	if ( fields ) { Tcl_ListObjAppendElement(ip, result,  map[i]->nameobj);       	     }
	if ( offset ) { Tcl_ListObjAppendElement(ip, result,  Tcl_NewIntObj(map[i]->offset)); }
    }

    Tcl_Free((char *) map);
    Tcl_SetObjResult(ip, result);

    return TCL_OK;
}

void ARecTypeAddField1(ARecType *type, Tcl_Obj *nameobj, int length, int array, ARecType *field) {
    int  i;
    int size = 0;
    int maxx;

    if ( type != ARecTypeType ) { type = ARecLookupType(type->nameobj); }

    if ( type->nfield >= type->afield-1 ) {
	type->afield += 10;
	type->field = (ARecField *) Tcl_Realloc((char *) type->field, sizeof(ARecField) * type->afield);
    }

    maxx = type->align;

    for ( i = 0; i < type->nfield; i++ ) {
	if ( type->stype == AREC_STRUCT ) {
	    size = ARecPadd(size + type->field[i].type->size*type->field[i].nrecs, type->field[i].type->align);
	} else {
	    size = Max(size, type->field[i].type->size*type->field[i].nrecs);
	}

	if ( type->field[i].type->align > maxx ) {
	    maxx = type->field[i].type->align;
	}
    }

    type->field[type->nfield].nameobj = nameobj;
    Tcl_IncrRefCount(nameobj);

    type->field[type->nfield].type   = field->shadow;
    type->field[type->nfield].nrecs  = length;
    type->field[type->nfield].arecs  = array ? length : -1;

    if ( type->stype == AREC_STRUCT ) {
	type->field[type->nfield].offset = ARecPadd(size, field->align);

	size	= ARecPadd(size + field->size*length, field->align);
    } else {
	type->field[type->nfield].offset = 0;

	size	= Max(size, ARecPadd(field->size*length, field->align));
    }
    type->size	= ARecPadd(size, maxx);
    type->align = maxx;

    type->nfield++;
    memcpy(type->shadow, type, sizeof(ARecType));
}

int ARecTypeAddField(Tcl_Interp *ip, ARecType *type, int objc, Tcl_Obj **objv)
{
    Tcl_Obj  	 *result = Tcl_GetObjResult(ip);
    int i;

    ARecType *field;

    if ( type->instances ) {
	Tcl_AppendStringsToObj(result, Tcl_GetString(objv[0]), " already has instances", NULL); 
	return TCL_ERROR;
    }

    if ( !(field = ARecLookupType(objv[2])) ) {
	Tcl_AppendStringsToObj(result, Tcl_GetString(objv[2]), " unknown data type ", NULL); 
	return TCL_ERROR;
    }

    for ( i = 3; i < objc; i++ ) {
	Tcl_Obj *name   = objv[i];
	long	 length = 1;
	int	 array  = 0;

	if ( ARecLookupField(type->nfield, type->field, objv[i]) ) {
	    Tcl_AppendStringsToObj(result, Tcl_GetString(objv[0]), " already has a field named ", Tcl_GetString(objv[i]), NULL);
	    return TCL_ERROR;
	}

	if ( i < objc-1 && isdigit(*Tcl_GetString(objv[i+1])) ) {
	    i++;

	    if ( Tcl_GetLongFromObj(NULL, objv[i], &length) != TCL_OK ) {
		Tcl_AppendStringsToObj(result, Tcl_GetString(objv[0]), " for field ", name, " cannot convert to length ", Tcl_GetString(objv[i]), NULL);
		return TCL_ERROR;
	    }

	    array = 1;
	}

	ARecTypeAddField1(type, name, length, array, field);
    }

    return TCL_OK;
}



int ARecIndex(ARecField *inst, Tcl_Obj *result, int *objc, Tcl_Obj ***objv, int *n, int *islist)
{
	char *here = NULL;

    Tcl_Obj *index = (*objv)[0];

    if ( Tcl_GetIntFromObj(NULL, index, n) == TCL_OK  ) {
	(*objc)--;
	(*objv)++;
	if ( islist ) { *islist = 1; }
    } else {
	char *end = Tcl_GetString((*objv)[0]);
	char *here = &end[3];

	if ( !strncmp(end, "end", 3) ) {
	    *n = inst->nrecs - 1;

	    if ( end[3] ) {
		*n += strtol(&end[3], &here, 10);
	    }

	    if ( *here ) {
		Tcl_AppendStringsToObj(result
		    , Tcl_GetString(inst->type->nameobj)
		    , " cannot index with "
		    , Tcl_GetString(index)
		    , NULL);

		return TCL_ERROR;
	    }

	    (*objc)--;								
	    (*objv)++;
	    if ( islist ) { *islist = 1; }
	}
    }

    return TCL_OK;
}

int ARecRange(Tcl_Interp *ip, ARecField *inst, int *objc, Tcl_Obj ***objv, int *n, int *m, int *islist)
{
    Tcl_Obj  	 *result = Tcl_GetObjResult(ip);
    int	indx = 0;

    if ( *objc >= 1 && ARecIndex(inst, result, objc, objv, n,  &indx)   != TCL_OK  ) { return TCL_ERROR; }

    if ( !indx ) { return TCL_OK; }
    *islist = 0;
    *m = *n;								

    if ( *objc >= 1 && ARecIndex(inst, result, objc, objv, m, islist) != TCL_OK  ) { return TCL_ERROR; }

    return TCL_OK;
}

void *ARecRealloc(ARecField *inst, int nrecs, int more) 
{
    if ( nrecs >  inst->arecs ) {
	inst->arecs = nrecs + more;
	inst->recs  = Tcl_Realloc((char *) inst->recs, inst->arecs * inst->type->size);


	memset(&((char *)inst->recs)[inst->nrecs * inst->type->size], 0, inst->type->size * ((nrecs - inst->nrecs) + more));
    }
    inst->nrecs = Max(nrecs, inst->nrecs);

    return inst->recs;
}

int ARecSetFromArgs(Tcl_Interp *ip, ARecType *type, char *recs, int n, int objc, Tcl_Obj *const*objv, int islist)
{
    	int i, j;
	int list = 0;
	Tcl_Obj *result = Tcl_GetObjResult(ip);

    for ( j = 0; j < n; j++ ) {
	for ( i = 0; i < objc; i += 2 ) {
	    ARecField *field = ARecLookupField(type->nfield, type->field, objv[i+0]);

	    if ( !field ) {
		Tcl_AppendStringsToObj(result , Tcl_GetString(type->nameobj) , " field "
			, Tcl_GetString(objv[i+0]), " not defined "
			, NULL);
		return TCL_ERROR;
	    }
	    if ( type->stype > 0 ) {
		ARecSetField(ip, &type->field[type->stype-1], recs, NULL);			// Union clear old value.
	    }

	    if ( ARecSetField(ip, field, recs, objv[i+1]) == TCL_ERROR ) {
		Tcl_AppendStringsToObj(result , "type \"", Tcl_GetString(type->nameobj) , "\" cannot set field \""
			, Tcl_GetString(objv[i+0]), "\" from \"" , Tcl_GetString(objv[i+1]) , "\"" , NULL);
		return TCL_ERROR;
	    } else {
		if ( type->stype != AREC_STRUCT ) {
		    type->stype = (field - type->field) + 1;				// Union record current field
		}
	    }
	}

	recs += type->size;
    }

    return TCL_OK;
}

int ARecSetFromDict(Tcl_Interp *ip, ARecType *type, char *recs, int n, int objc, Tcl_Obj *const*objv)
{
    	int i, j;
	Tcl_Obj *result = Tcl_GetObjResult(ip);

	int	  elemc;
	Tcl_Obj	**elemv;

    for ( j = 0; j < n; j++ ) {
	if ( Tcl_ListObjGetElements(ip, objv[j % objc], &elemc, &elemv) == TCL_ERROR ) {
	    return TCL_ERROR;
	}

        if ( elemc % 2 ) {
	    Tcl_AppendStringsToObj(result
		    , Tcl_GetString(type->nameobj) , " cannot set fields from an odd number of elements"
		    , NULL);
	    return TCL_ERROR;
	}

	for ( i = 0; i < elemc; i += 2 ) {
	    if ( ARecSetField(ip, ARecLookupField(type->nfield, type->field, elemv[i+0]), recs, elemv[i+1]) == TCL_ERROR ) {
		Tcl_AppendStringsToObj(result, Tcl_GetString(type->nameobj), " cannot set field ", Tcl_GetString(elemv[i+0]), " from ", Tcl_GetString(elemv[i+1]), NULL);
		return TCL_ERROR;
	    }
	}

	recs += type->size;
    }

    return TCL_OK;
}

int ARecSetFromList(Tcl_Interp *ip, ARecType *type, char *inst, int n, int objc, Tcl_Obj *const*objv, int flags)
{
	ARecField *table = type->field;
	Tcl_Obj  	 *result = Tcl_GetObjResult(ip);

        ARecField **map;
	int		  nmap;
	int		 j, i, m;

	int	  elemc;
	Tcl_Obj	**elemv;
	int	  incr = 0;

    if ( !objc ) {
	Tcl_AppendStringsToObj(result , Tcl_GetString(type->nameobj) , " too few args to setlist " , NULL);
	return TCL_ERROR;
    }
    if ( !(map = ARecFieldMap(result, objc - 1, objv, type, &nmap)) ) {
	return TCL_ERROR;
    }

    if ( (flags & AREC_ISLIST) && Tcl_ListObjGetElements(ip, objv[objc - 1], &objc, (Tcl_Obj ***) &objv) == TCL_ERROR ) {
	Tcl_Free((void *) map);
	return TCL_ERROR;
    }

    for ( j = 0; j < n; j++ ) {
	if ( Tcl_ListObjGetElements(ip, objv[j % objc], &elemc, &elemv) == TCL_ERROR ) {
	    Tcl_Free((void *) map);

	    Tcl_SetObjResult(ip, result);
	    return TCL_ERROR;
	}

	for ( i = 0, m = 0; i < elemc && m < nmap; i++, m++ ) {
	    if ( ARecSetField(ip, map[m], inst, elemv[i]) == TCL_ERROR ) {
		Tcl_AppendStringsToObj(result, Tcl_GetString(type->nameobj), " cannot set field ", Tcl_GetString(map[m]->nameobj), " of type ", Tcl_GetString(map[m]->type->nameobj), " from ", Tcl_GetString(elemv[i]), NULL);
		Tcl_Free((void *) map);
		Tcl_SetObjResult(ip, result);
		return TCL_ERROR;
	    }
	}

	inst += type->size;
    }

    Tcl_Free((void *) map);
	return TCL_OK;
}

Tcl_Obj *ARecGetStruct(Tcl_Interp *ip, ARecType *type, void *recs, int m, int objc, Tcl_Obj *const*objv, int flags)
{

    	int i, j;

	Tcl_Obj    *result = Tcl_NewObj();
        ARecField **map;
	int	   nmap;

    if ( !(map = ARecFieldMap(result, objc, objv, type, &nmap)) ) {
	Tcl_SetObjResult(ip, result);
	return NULL;
    }

    for ( j = 0; j < m; j++ ) {
	Tcl_Obj *item;

	item = Tcl_NewObj();

	for ( i = 0 ; i < nmap; i++ ) {

	    Tcl_Obj *value = map[i]->type->get(ip, map[i]->type, ((char *)recs) + map[i]->offset, map[i]->nrecs, 0, NULL, (map[i]->nrecs > 1 ? AREC_ISLIST : 0) | (flags & AREC_ASDICT));

	    if ( value == NULL ) {
		Tcl_Free((void *) map);
		return NULL;
	    }

	    if ( nmap > 1 || flags & AREC_ISLIST || flags & AREC_ASDICT ) {
		if ( flags & AREC_ASDICT ) {
		    if ( Tcl_ListObjAppendElement(ip, item, map[i]->nameobj) == TCL_ERROR ) {
			Tcl_Free((void *) map);
			return NULL;
		    }
		}

		if ( Tcl_ListObjAppendElement(ip, item, value) == TCL_ERROR ) {
		    Tcl_Free((void *) map);
		    return NULL;
		}
	    } else {
		Tcl_DecrRefCount(item);
		item = value;
	    }
	}
	if ( flags & AREC_ISLIST ) {
	    if ( Tcl_ListObjAppendElement(ip, result, item) == TCL_ERROR ) {
		Tcl_Free((void *) map);
		return NULL;
	    }
	} else {
	    Tcl_DecrRefCount(result);
	    result = item;
	}

	recs = (void *) ((char *)recs) + type->size;
    }

    Tcl_Free((void *) map);

    return result;
}

ARecType *ARecTypeAddType(ARecField *types, Tcl_Obj *nameobj, int size, int align, int stype, ARecSetFunc set, ARecGetFunc get)
{
    ARecType *type;

    if ( types ) {
	ARecRealloc(types, ++types->nrecs, 10);

	type = &((ARecType *)(types->recs))[types->nrecs-1];
	type->shadow = (ARecType *) Tcl_Alloc(sizeof(ARecType));
    } else {
	type = (ARecType *) Tcl_Alloc(sizeof(ARecType));
	type->shadow = type;
    }

    type->nameobj = nameobj;
    Tcl_IncrRefCount(type->nameobj);

    type->size   =  size;
    type->align  =  align;
    type->stype  =  stype;
    type->nfield =  0;
    type->afield = 10;
    type->field  = (ARecField *) Tcl_Alloc(sizeof(ARecField) * type->afield);
    memset(type->field, 0, sizeof(ARecField) * type->afield);
    type->instances = NULL;

    type->set = set;
    type->get = get;

    if ( types ) {
	memcpy(type->shadow, type, sizeof(ARecType));
    }

    return type;
}

void ARecInit(Tcl_Interp *ip) {
    ARecType *type;
    int i;

    ARecTypedefInfo.deleteProc = NULL;
    
    TclDictType   = Tcl_GetObjType("dict");
    TclListType   = Tcl_GetObjType("list");

    Tcl_Obj *tclobjLong   = Tcl_NewStringObj("long", -1);
    Tcl_Obj *tclobjTclObj = Tcl_NewStringObj("Tcl_Obj*",-1);

    int dalign = sizeof(ARecDblAlign) - sizeof(double);
    int palign = sizeof(ARecPtrAlign) - sizeof(void*);
    int lalign = sizeof(ARecLngAlign) - sizeof(long);

    ARecTypeType = ARecTypeCreate(ip, Tcl_NewStringObj("::arec::type", -1), AREC_STRUCT);
    ARecTypeType->size = sizeof(ARecType);


    ARecTypeInst = ARecInstCreate(ip, Tcl_NewStringObj("::arec::types", -1), ARecTypeType, 0);

    ARecTypeAddType(ARecTypeInst, Tcl_NewStringObj("char",   -1), sizeof(char)	  	, 1,      0, ARecSetChar,	ARecGetChar  );
    ARecTypeAddType(ARecTypeInst, Tcl_NewStringObj("uchar",  -1), sizeof(unsigned char) , 1,      0, ARecSetUChar,	ARecGetUChar );
    ARecTypeAddType(ARecTypeInst, Tcl_NewStringObj("short",  -1), sizeof(short)	  	, 2,      0, ARecSetShort,	ARecGetShort );
    ARecTypeAddType(ARecTypeInst, Tcl_NewStringObj("ushort", -1), sizeof(unsigned short), 2,      0, ARecSetUShort, 	ARecGetUShort);
    ARecTypeAddType(ARecTypeInst, Tcl_NewStringObj("int",    -1), sizeof(int)		, 4,      0, ARecSetInt,	ARecGetInt   );
    ARecTypeAddType(ARecTypeInst, Tcl_NewStringObj("uint",   -1), sizeof(unsigned int)	, 4,      0, ARecSetUInt,	ARecGetUInt  );
    ARecTypeAddType(ARecTypeInst, tclobjLong,	                  sizeof(long)	  	, lalign, 0, ARecSetLong,	ARecGetLong  );
    ARecTypeAddType(ARecTypeInst, Tcl_NewStringObj("ulong",  -1), sizeof(long)	  	, lalign, 0, ARecSetULong,	ARecGetULong );
    ARecTypeAddType(ARecTypeInst, Tcl_NewStringObj("float",  -1), sizeof(float)	  	, 4,      0, ARecSetFloat,	ARecGetFloat );
    ARecTypeAddType(ARecTypeInst, Tcl_NewStringObj("double", -1), sizeof(double)	, dalign, 0, ARecSetDouble, 	ARecGetDouble);
    ARecTypeAddType(ARecTypeInst, Tcl_NewStringObj("string", -1), sizeof(char *)	, palign, 0, ARecSetString, 	ARecGetString);
    ARecTypeAddType(ARecTypeInst, Tcl_NewStringObj("char*",  -1), sizeof(char *)	, palign, 0, ARecSetString, 	ARecGetString);
    ARecTypeAddType(ARecTypeInst, tclobjTclObj,		          sizeof(Tcl_Obj *)	, palign, 0, ARecSetTclObj, 	ARecGetTclObj);

    ARecTypeType->size = 0;

    ARecTypeAddField1(ARecTypeType, Tcl_NewStringObj("name",  -1), 1, 0, ARecLookupType(tclobjTclObj));
    ARecTypeAddField1(ARecTypeType, Tcl_NewStringObj("size",  -1), 1, 0, ARecLookupType(tclobjLong));
    ARecTypeAddField1(ARecTypeType, Tcl_NewStringObj("align", -1), 1, 0, ARecLookupType(tclobjLong));
    ARecTypeAddField1(ARecTypeType, Tcl_NewStringObj("stype", -1), 1, 0, ARecLookupType(tclobjLong));
    ARecTypeAddField1(ARecTypeType, Tcl_NewStringObj("nfield",-1), 1, 0, ARecLookupType(tclobjLong));
    ARecTypeAddField1(ARecTypeType, Tcl_NewStringObj("afield",-1), 1, 0, ARecLookupType(tclobjLong));
    ARecTypeAddField1(ARecTypeType, Tcl_NewStringObj("fields",-1), 1, 0, ARecLookupType(tclobjLong));
    ARecTypeAddField1(ARecTypeType, Tcl_NewStringObj("set",   -1), 1, 0, ARecLookupType(tclobjLong));
    ARecTypeAddField1(ARecTypeType, Tcl_NewStringObj("get",   -1), 1, 0, ARecLookupType(tclobjLong));
    ARecTypeAddField1(ARecTypeType, Tcl_NewStringObj("shadow",-1), 1, 0, ARecLookupType(tclobjLong));
    ARecTypeAddField1(ARecTypeType, Tcl_NewStringObj("inst",  -1), 1, 0, ARecLookupType(tclobjLong));
}

