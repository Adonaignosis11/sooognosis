/* roi_variable_type.h - used to generate the different roi_*.h files
 *
 * Part of amide - Amide's a Medical Image Data Examiner
 * Copyright (C) 2001 Andy Loening
 *
 * Author: Andy Loening <loening@ucla.edu>
 */

/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
  02111-1307, USA.
*/


#ifndef __ROI_`'m4_Internal_Data_Format`'__
#define __ROI_`'m4_Variable_Type`'__

/* header files that are always needed with this file */
#include "roi.h"


/* function declarations */
#define ROI_`'m4_Variable_Type`'_TYPE

#if defined(ROI_ELLIPSOID_TYPE) || defined(ROI_CYLINDER_TYPE) || defined(ROI_BOX_TYPE)
GSList * roi_`'m4_Variable_Type`'_get_slice_intersection(const roi_t * roi, const volume_t * view_slice);
#endif

#if defined(ROI_ISOCONTOUR_2D_TYPE) || defined(ROI_ISOCONTOUR_3D_TYPE)
roi_t * roi_`'m4_Variable_Type`'_get_slice_intersection(const roi_t * roi, const volume_t * view_slice);
#endif

#if defined(ROI_ISOCONTOUR_2D_TYPE) || defined(ROI_ISOCONTOUR_3D_TYPE)
void roi_`'m4_Variable_Type`'_set_isocontour(roi_t * roi, volume_t * vol, voxelpoint_t iso_vp);
#endif

#undef ROI_`'m4_Variable_Type`'_TYPE

#endif /* __ROI_`'m4_Variable_Type`'__ */




