/* --------------------------------------------------------------------
Copyright (C) 2009 Swedish Meteorological and Hydrological Institute, SMHI,

This file is part of HLHDF.

HLHDF is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HLHDF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with HLHDF.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------*/

/**
 * Functions for working with compound types.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-11
 */
#include "hlhdf.h"
#include "hlhdf_alloc.h"
#include "hlhdf_defines_private.h"
#include "hlhdf_debug.h"
#include <string.h>
#include <stdlib.h>

HL_CompoundTypeAttribute* newHL_CompoundTypeAttribute(char* attrname,
  size_t offset, const char* format, size_t size, int ndims, size_t* dims)
{
  HL_CompoundTypeAttribute* retv = NULL;
  int i;
  HL_SPEWDEBUG0("ENTER: newHL_CompoundTypeAttribute");
  if (!attrname) {
    HL_ERROR0("Impossible to have an attribute without a name in a compound attribute");
    goto fail;
  }
  if (!format) {
    HL_ERROR0("Impossible to have an attribute without a format in a compound type");
    goto fail;
  }
  if (!(retv = (HL_CompoundTypeAttribute*) HLHDF_MALLOC(sizeof(HL_CompoundTypeAttribute)))) {
    HL_ERROR0("Failed to allocate CompoundTypeAttribute description");
    goto fail;
  }

  strcpy(retv->attrname, attrname);
  retv->offset = offset;
  retv->size = size;
  strcpy(retv->format, format);
  retv->ndims = ndims;
  for (i = 0; i < ndims; i++)
    retv->dims[i] = dims[i];

fail:
  HL_SPEWDEBUG0("EXIT: newHL_CompoundTypeAttribute");
  return retv;
}

HL_CompoundTypeDescription* newHL_CompoundTypeDescription()
{
   HL_CompoundTypeDescription* retv=NULL;
   int i;
   HL_DEBUG0("ENTER: newHL_CompoundTypeDescription");
   if(!(retv=(HL_CompoundTypeDescription*)HLHDF_MALLOC(sizeof(HL_CompoundTypeDescription)))) {
      HL_ERROR0("Failed to allocate memory for CompoundTypeDescription");
      return NULL;
   }
   strcpy(retv->hltypename,"");
   retv->size=0;
   if(!(retv->attrs = (HL_CompoundTypeAttribute **)HLHDF_MALLOC(sizeof(HL_CompoundTypeAttribute*)*DEFAULT_SIZE_NODELIST))) {
      HL_ERROR0("Failed to allocate memory for CompoundTypeDescription list");
      HLHDF_FREE(retv);
      return NULL;
   }
   for(i=0;i<DEFAULT_SIZE_NODELIST;i++) {
      retv->attrs[i]=NULL;
   }
   retv->nAttrs=0;
   retv->nAllocAttrs=DEFAULT_SIZE_NODELIST;
   retv->objno[0]=0;
   retv->objno[1]=0;
   return retv;
}

void freeHL_CompoundTypeAttribute(HL_CompoundTypeAttribute* attr)
{
   HL_SPEWDEBUG0("ENTER: freeHL_CompoundTypeAttribute");
   if(attr) {
     HLHDF_FREE(attr);
   }
   HL_SPEWDEBUG0("EXIT: freeHL_CompoundTypeAttribute");
}

void freeHL_CompoundTypeDescription(HL_CompoundTypeDescription* typelist)
{
  int i;
  if (!typelist)
    return;

  HL_SPEWDEBUG0("ENTER: freeHL_CompoundTypeDescription");

  if (typelist->attrs) {
    for (i = 0; i < typelist->nAttrs; i++) {
      if (typelist->attrs[i])
        freeHL_CompoundTypeAttribute(typelist->attrs[i]);
    }
    HLHDF_FREE(typelist->attrs);
  }
  HLHDF_FREE(typelist);

  HL_SPEWDEBUG0("EXIT: freeHL_CompoundTypeDescription");
}

int addHL_CompoundTypeAttribute(HL_CompoundTypeDescription* typelist,
           HL_CompoundTypeAttribute* attribute)
{
   int newallocsize;
   int i;
   HL_DEBUG0("ENTER: addCompoundTypeAttribute");
   if(!attribute) {
      HL_ERROR0("Trying to add compound type attribute which is NULL");
      return 0;
   }

   if(typelist->nAttrs<typelist->nAllocAttrs-1) {
      typelist->attrs[typelist->nAttrs++]=attribute;
      return 1;
   }

   newallocsize = typelist->nAllocAttrs + DEFAULT_SIZE_NODELIST;
   if(!(typelist->attrs = HLHDF_REALLOC(typelist->attrs,
          sizeof(HL_CompoundTypeAttribute*)*newallocsize))) {
      HL_ERROR0("Serious memory error occured when reallocating compound attr list");
      return 0;
   }
   for(i=typelist->nAllocAttrs;i<newallocsize;i++) {
      typelist->attrs[i]=NULL;
   }
   typelist->nAllocAttrs=newallocsize;
   typelist->attrs[typelist->nAttrs++]=attribute;

   return 1;
}

HL_CompoundTypeDescription* copyHL_CompoundTypeDescription(
  HL_CompoundTypeDescription* descr)
{
  HL_CompoundTypeDescription* retv = NULL;
  int i;
  HL_DEBUG0("ENTER: copyCompoundTypeDescription");
  if (descr == NULL)
    return NULL;

  if (!(retv = newHL_CompoundTypeDescription())) {
    return NULL;
  }
  strcpy(retv->hltypename, descr->hltypename);
  memcpy(retv->objno, descr->objno, sizeof(unsigned long) * 2);
  retv->size = descr->size;
  retv->nAttrs = descr->nAttrs;
  retv->nAllocAttrs = descr->nAllocAttrs;

  HLHDF_FREE(retv->attrs);
  if (!(retv->attrs
      = (HL_CompoundTypeAttribute**) HLHDF_MALLOC(sizeof(HL_CompoundTypeAttribute*)*retv->nAllocAttrs))) {
    HL_ERROR0("Failed to allocate list of HL_CompoundTypeAttribute");
    goto fail;
  }

  for (i = 0; i < retv->nAllocAttrs; i++)
    retv->attrs[i] = NULL;
  for (i = 0; i < retv->nAttrs; i++) {
    if (!(retv->attrs[i]
        = newHL_CompoundTypeAttribute(descr->attrs[i]->attrname,
                                      descr->attrs[i]->offset,
                                      descr->attrs[i]->format,
                                      descr->attrs[i]->size,
                                      descr->attrs[i]->ndims,
                                      descr->attrs[i]->dims))) {
      HL_ERROR0("Failed to allocate HL_CompoundTypeAttribute");
      goto fail;
    }
  }

  return retv;
fail:
  freeHL_CompoundTypeDescription(retv);
  return NULL;
}
