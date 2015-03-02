/* amitk_data_set_variable_type.c - used to generate the different amitk_data_set_*.c files
 *
 * Part of amide - Amide's a Medical Image Data Examiner
 * Copyright (C) 2001-2003 Andy Loening
 *
 * Author: Andy Loening <loening@alum.mit.edu>
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

#include "amide_config.h"
#include <glib.h>
#ifdef AMIDE_DEBUG
#include <stdlib.h>
#endif
#include "amitk_data_set_`'m4_Variable_Type`'_`'m4_Scale_Dim`'.h"
#include "amitk_data_set_FLOAT_0D_SCALING.h"

#define DIM_TYPE_`'m4_Scale_Dim`'
#define DATA_TYPE_`'m4_Variable_Type`'


/* function to recalcule the max and min values of a data set */
void amitk_data_set_`'m4_Variable_Type`'_`'m4_Scale_Dim`'_calc_frame_max_min(AmitkDataSet * data_set,
									     gboolean (*update_func)(),
									     gpointer update_data) {

  AmitkVoxel i;
  amide_data_t max, min, temp;
  AmitkVoxel initial_voxel;
  div_t x;
  gint divider;
  gint t_times_z;
  gchar * temp_string;
  AmitkVoxel dim;
  
  dim = AMITK_DATA_SET_DIM(data_set);

  /* note, we can't cancel this */
  if (update_func != NULL) {
    temp_string = g_strdup_printf(_("Calculating Max/Min Values for:\n   %s"), AMITK_OBJECT_NAME(data_set));
    (*update_func)(update_data, temp_string, (gdouble) 0.0);
    g_free(temp_string);
  }
  t_times_z = dim.z*dim.t;
  divider = ((t_times_z/AMIDE_UPDATE_DIVIDER) < 1) ? 1 : (t_times_z/AMIDE_UPDATE_DIVIDER);

  initial_voxel = zero_voxel;
  for (i.t = 0; i.t < dim.t; i.t++) {
    initial_voxel.t = i.t;
    temp = AMITK_DATA_SET_`'m4_Variable_Type`'_`'m4_Scale_Dim`'_CONTENT(data_set, initial_voxel);
    if (finite(temp)) max = min = temp;   
    else max = min = 0.0; /* just throw in zero */

    for (i.z = 0; i.z < dim.z; i.z++) {
      if (update_func != NULL) {
	x = div(i.t*dim.z+i.z,divider);
	if (x.rem == 0)
	  (*update_func)(update_data, NULL, (gdouble) (i.z+i.t*dim.z)/t_times_z);
      }
      for (i.y = 0; i.y < dim.y; i.y++) 
	for (i.x = 0; i.x < dim.x; i.x++) {
	  temp = AMITK_DATA_SET_`'m4_Variable_Type`'_`'m4_Scale_Dim`'_CONTENT(data_set, i);
	  if (finite(temp)) {
	    if (temp > max) max = temp;
	    else if (temp < min) min = temp;
	  }
	}
    }
    
    data_set->frame_max[i.t] = max;
    data_set->frame_min[i.t] = min;
    
#ifdef AMIDE_DEBUG
    if (dim.z > 1) /* don't print for slices */
      g_print("\tframe %d max %5.3f frame min %5.3f\n",i.t, data_set->frame_max[i.t],data_set->frame_min[i.t]);
#endif
  }

  if (update_func != NULL)
    (*update_func)(update_data, NULL, (gdouble) 2.0); /* remove progress bar */
  
  return;
}

/* generate the distribution array for a data_set */
void amitk_data_set_`'m4_Variable_Type`'_`'m4_Scale_Dim`'_calc_distribution(AmitkDataSet * data_set,
									    gboolean (*update_func)(),
									    gpointer update_data) {

  AmitkVoxel i,j;
  amide_data_t scale, diff;
  AmitkVoxel distribution_dim;
  AmitkVoxel data_set_dim;
  gchar * temp_string;
  div_t x;
  gint divider;
  gint t_times_z;
  gboolean continue_work=TRUE;
  AmitkRawData * distribution;

  if (data_set->distribution != NULL)
    return;

  data_set_dim = AMITK_DATA_SET_DIM(data_set);
  diff = amitk_data_set_get_global_max(data_set) - amitk_data_set_get_global_min(data_set);
  if (diff == 0.0)
    scale = 0.0;
  else
    scale = (AMITK_DATA_SET_DISTRIBUTION_SIZE-1)/diff;
  
  distribution_dim.x = AMITK_DATA_SET_DISTRIBUTION_SIZE;
  distribution_dim.y = distribution_dim.z = distribution_dim.t = 1;
  distribution = amitk_raw_data_new_with_data(AMITK_FORMAT_DOUBLE, distribution_dim);
  if (distribution == NULL) {
    g_warning(_("couldn't allocate space for the data set structure to hold distribution data"));
    return;
  }

  /* initialize the distribution array */
  amitk_raw_data_DOUBLE_initialize_data(distribution, 0.0);
  
  if (update_func != NULL) {
    temp_string = g_strdup_printf(_("Generating distribution data for:\n   %s"), AMITK_OBJECT_NAME(data_set));
    continue_work = (*update_func)(update_data, temp_string, (gdouble) 0.0);
    g_free(temp_string);
  }
  t_times_z = data_set_dim.z*data_set_dim.t;
  divider = ((t_times_z/AMIDE_UPDATE_DIVIDER) < 1) ? 1 : (t_times_z/AMIDE_UPDATE_DIVIDER);

  /* now "bin" the data */
  j.t = j.z = j.y = 0;
  for (i.t = 0; (i.t < data_set_dim.t) && continue_work; i.t++) {
    for ( i.z = 0; (i.z < data_set_dim.z) && continue_work; i.z++) {
      if (update_func != NULL) {
	x = div(i.t*data_set_dim.z+i.z,divider);
	if (x.rem == 0)
	  continue_work = (*update_func)(update_data, NULL, (gdouble) (i.z+i.t*data_set_dim.z)/t_times_z);
      }

      for (i.y = 0; i.y < data_set_dim.y; i.y++) 
	for (i.x = 0; i.x < data_set_dim.x; i.x++) {
	  j.x = scale*(AMITK_DATA_SET_`'m4_Variable_Type`'_`'m4_Scale_Dim`'_CONTENT(data_set,i)-amitk_data_set_get_global_min(data_set));
	  AMITK_RAW_DATA_DOUBLE_SET_CONTENT(distribution,j) += 1.0;
	}
    }
  }

  if (update_func != NULL) /* remove progress bar */
    continue_work = (*update_func)(update_data, NULL, (gdouble) 2.0); 

  if (!continue_work) {   /* if we quit, get out of here */
    g_object_unref(distribution);
    return;
  }
  
  /* do some log scaling so the distribution is more meaningful, and doesn't get
     swamped by outlyers */
  for (j.x = 0; j.x < distribution_dim.x ; j.x++) 
    AMITK_RAW_DATA_DOUBLE_SET_CONTENT(distribution,j) = 
      log10(AMITK_RAW_DATA_DOUBLE_CONTENT(distribution,j)+1.0);

  /* and store the distribution with the data set */
  data_set->distribution = distribution;
  return;
}




/* returns a slice  with the appropriate data from the data_set */
AmitkDataSet * amitk_data_set_`'m4_Variable_Type`'_`'m4_Scale_Dim`'_get_slice(AmitkDataSet * data_set,
									      const amide_time_t start_time,
									      const amide_time_t duration,
									      const amide_real_t pixel_dim,
									      const AmitkVolume * slice_volume) {

  /* zp_start, where on the zp axis to start the slice, zp (z_prime) corresponds
     to the rotated axises, if negative, choose the midpoint */

  AmitkDataSet * slice;
  AmitkVoxel i_voxel;
  amide_intpoint_t z;
  amide_real_t max_diff, voxel_length, z_steps;
  AmitkPoint alt;
  AmitkPoint stride[AMITK_AXIS_NUM], last[AMITK_AXIS_NUM];
  AmitkAxis i_axis;
  guint l;
  amide_data_t weight;
  amide_data_t time_weight;
  amide_intpoint_t start_frame, end_frame, i_frame;
  amide_time_t end_time;
  AmitkPoint box_point[8];
  AmitkVoxel box_voxel[8];
  AmitkVoxel start, end;
  amide_data_t box_value[8];
  AmitkPoint slice_point, ds_point,start_point,diff, nearest_point;
  AmitkSpace * slice_space;
  AmitkSpace * data_set_space;
#if AMIDE_DEBUG
  gchar * temp_string;
  AmitkPoint center_point;
#endif
  AmitkVoxel ds_voxel;
  amide_data_t weight1, weight2;
  amide_data_t empty;
  AmitkCorners intersection_corners;
  AmitkVoxel dim;

  /* ----- figure out what frames of this data set to include ----*/
  end_time = start_time+duration;
  start_frame = amitk_data_set_get_frame(data_set, start_time+EPSILON);
  end_frame = amitk_data_set_get_frame(data_set, end_time-EPSILON);

  /* ------------------------- */

  dim.x = ceil(fabs(AMITK_VOLUME_X_CORNER(slice_volume))/pixel_dim);
  dim.y = ceil(fabs(AMITK_VOLUME_Y_CORNER(slice_volume))/pixel_dim);
  dim.z = dim.t = 1;


  /* get the return slice */
  slice = amitk_data_set_new_with_data(NULL, AMITK_DATA_SET_MODALITY(data_set), 
				       AMITK_FORMAT_DOUBLE, dim, AMITK_SCALING_TYPE_0D);
  if (slice == NULL) {
    g_warning(_("couldn't allocate space for the slice, wanted %dx%dx%dx%d elements"), 
	      dim.x, dim.y, dim.z, dim.t);
    return NULL;
  }


  slice->slice_parent = data_set;
  g_object_add_weak_pointer(G_OBJECT(data_set), 
			    (gpointer *) &(slice->slice_parent));
  slice->voxel_size.x = pixel_dim;
  slice->voxel_size.y = pixel_dim;
  slice->voxel_size.z = AMITK_VOLUME_Z_CORNER(slice_volume);
  amitk_space_copy_in_place(AMITK_SPACE(slice), AMITK_SPACE(slice_volume));
  slice->scan_start = start_time;
  slice->thresholding = data_set->thresholding;
  slice->interpolation = AMITK_DATA_SET_INTERPOLATION(data_set);

  amitk_data_set_calc_far_corner(slice);
  amitk_data_set_set_frame_duration(slice, 0, duration);


#if AMIDE_DEBUG
  center_point = amitk_volume_get_center(slice_volume);
  temp_string =  
    g_strdup_printf("slice from data_set %s: @ x %5.3f y %5.3f z %5.3f", AMITK_OBJECT_NAME(data_set), 
		    center_point.x, center_point.y, center_point.z);
  amitk_object_set_name(AMITK_OBJECT(slice),temp_string);
  g_free(temp_string);
#endif
#ifdef AMIDE_DEBUG_COMMENT_OUT
  {
    AmitkCorners real_corner;
    /* convert to real space */
    real_corner[0] = rs_offset(slice->space);
    real_corner[1] = amitk_space_s2b(slice->space, slice->corner);
    g_print("new slice from data_set %s\t---------------------\n",AMITK_OBJECT_NAME(data_set));
    g_print("\tdim\t\tx %d\t\ty %d\t\tz %d\n",
    	    dim.x, dim.y, dim.z);
    g_print("\treal corner[0]\tx %5.4f\ty %5.4f\tz %5.4f\n",
    	    real_corner[0].x,real_corner[0].y,real_corner[0].z);
    g_print("\treal corner[1]\tx %5.4f\ty %5.4f\tz %5.4f\n",
    	    real_corner[1].x,real_corner[1].y,real_corner[1].z);
    g_print("\tdata set\t\tstart\t%5.4f\tend\t%5.3f\tframes %d to %d\n",
    	    start_time, end_time,start_frame,start_frame+num_frames-1);
  }
#endif


  /* get direct pointers to the slice's and data set's spaces for efficiency */
  slice_space = AMITK_SPACE(slice);
  data_set_space = AMITK_SPACE(data_set);

  /* voxel_length is the length of a voxel given the coordinate frame of the slice.
     this is used to figure out how many iterations in the z direction we need to do */
  alt.x = alt.y = 0.0;
  alt.z = 1.0;
  alt = amitk_space_s2s_dim(slice_space, data_set_space, alt);
  alt = point_mult(alt, data_set->voxel_size);
  voxel_length = POINT_MAGNITUDE(alt);
  z_steps = slice->voxel_size.z/voxel_length;

  /* figure out the intersection bounds between the data set and the requested slice volume */
  if (amitk_volume_volume_intersection_corners(slice_volume, 
					       AMITK_VOLUME(data_set), 
					       intersection_corners)) {
    /* translate the intersection into voxel space */
    POINT_TO_VOXEL(intersection_corners[0], slice->voxel_size, 0, start);
    POINT_TO_VOXEL(intersection_corners[1], slice->voxel_size, 0, end);
  } else { /* no intersection */
    start = zero_voxel;
    end = zero_voxel;
  }

  /* make sure we only iterate over the slice we've already mallocs */
  if (start.x < 0) start.x = 0;
  if (start.y < 0) start.y = 0;
  if (end.x >= dim.x) end.x = dim.x-1;
  if (end.y >= dim.y) end.y = dim.y-1;

  /* empty is what we fill voxels with that aren't in the data set*/
  if (amitk_data_set_get_global_max(data_set) < 0)
    empty = amitk_data_set_get_global_min(data_set);
  else
    empty = 0;

  /* iterate over those voxels that we didn't cover, and set them to empty */
  i_voxel.t = i_voxel.z = 0;
  for (i_voxel.y = 0; i_voxel.y < start.y; i_voxel.y++) 
    for (i_voxel.x = 0; i_voxel.x < dim.x; i_voxel.x++) 
      AMITK_RAW_DATA_DOUBLE_SET_CONTENT(slice->raw_data,i_voxel) = empty;
  for (i_voxel.y = end.y+1; i_voxel.y < dim.y; i_voxel.y++) 
    for (i_voxel.x = 0; i_voxel.x < dim.x; i_voxel.x++) 
      AMITK_RAW_DATA_DOUBLE_SET_CONTENT(slice->raw_data,i_voxel) = empty;
  for (i_voxel.x = 0; i_voxel.x < start.x; i_voxel.x++) 
    for (i_voxel.y = 0; i_voxel.y < dim.y; i_voxel.y++) 
      AMITK_RAW_DATA_DOUBLE_SET_CONTENT(slice->raw_data,i_voxel) = empty;
  for (i_voxel.x = end.x+1; i_voxel.x < dim.x; i_voxel.x++) 
    for (i_voxel.y = 0; i_voxel.y < dim.y; i_voxel.y++) 
      AMITK_RAW_DATA_DOUBLE_SET_CONTENT(slice->raw_data,i_voxel) = empty;

  /* and initialize the rest of the voxels */
  for (i_voxel.y = start.y; i_voxel.y <= end.y; i_voxel.y++)
    for (i_voxel.x = start.x; i_voxel.x <= end.x; i_voxel.x++)
      AMITK_RAW_DATA_DOUBLE_SET_CONTENT(slice->raw_data,i_voxel) = 0;



  switch(data_set->interpolation) {

  case AMITK_INTERPOLATION_TRILINEAR:
    /* iterate over the frames we'll be incorporating into this slice */
    for (i_frame = start_frame; i_frame <= end_frame; i_frame++) {
      
      /* averaging over more then one frame */
      if (end_frame-start_frame > 0) {
	if (i_frame == start_frame)
	  time_weight = (amitk_data_set_get_end_time(data_set, start_frame)-start_time)/duration;
	else if (i_frame == end_frame)
	  time_weight = (end_time-amitk_data_set_get_start_time(data_set, end_frame))/duration;
	else
	  time_weight = amitk_data_set_get_frame_duration(data_set, i_frame)/duration;
      } else
	time_weight = 1.0;

      /* iterate over the number of planes we'll be compressing into this slice */
      for (z = 0; z < ceil(z_steps); z++) {
	
	/* the slices z_coordinate for this iteration's slice voxel */
	slice_point.z = (((amide_real_t) z)+0.5)*voxel_length;

	/* weight is between 0 and 1, this is used to weight the last voxel  in the slice's z direction */
	if (floor(z_steps) > z)
	  weight = time_weight/z_steps;
	else
	  weight = time_weight*(z_steps-floor(z_steps)) / z_steps;

	/* iterate over the y dimension */
	for (i_voxel.y = start.y; i_voxel.y <= end.y; i_voxel.y++) {

	  /* the slice y_coordinate of the center of this iteration's slice voxel */
	  slice_point.y = (((amide_real_t) i_voxel.y)+0.5)*slice->voxel_size.y;
	  
	  /* the slice x coord of the center of the first slice voxel in this loop */
	  slice_point.x = (((amide_real_t) start.x)+0.5)*slice->voxel_size.x;

	  /* iterate over the x dimension */
	  for (i_voxel.x = start.x; i_voxel.x <= end.x; i_voxel.x++) {

	    /* translate the current point in slice space into the data set's coordinate frame */
	    ds_point = amitk_space_s2s(slice_space, data_set_space, slice_point);

	    /* get the nearest neighbor in the data set to this slice voxel */
	    POINT_TO_VOXEL(ds_point, data_set->voxel_size, i_frame, ds_voxel);
	    VOXEL_TO_POINT(ds_voxel, data_set->voxel_size, nearest_point);

	    /* figure out which way to go to get the nearest voxels to our slice voxel*/
	    POINT_SUB(ds_point, nearest_point, diff);

	    /* figure out which voxels to look at */
	    for (l=0; l<8; l=l+1) {
	      if (diff.x < 0)
		box_voxel[l].x = (l & 0x1) ? ds_voxel.x-1 : ds_voxel.x;
	      else /* diff.x >= 0 */
		box_voxel[l].x = (l & 0x1) ? ds_voxel.x : ds_voxel.x+1;
	      if (diff.y < 0)
		box_voxel[l].y = (l & 0x2) ? ds_voxel.y-1 : ds_voxel.y;
	      else /* diff.y >= 0 */
		box_voxel[l].y = (l & 0x2) ? ds_voxel.y : ds_voxel.y+1;
	      if (diff.z < 0)
		box_voxel[l].z = (l & 0x4) ? ds_voxel.z-1 : ds_voxel.z;
	      else /* diff.z >= 0 */
		box_voxel[l].z = (l & 0x4) ? ds_voxel.z : ds_voxel.z+1;
	      box_voxel[l].t = ds_voxel.t;

	      VOXEL_TO_POINT(box_voxel[l], data_set->voxel_size, box_point[l]);

	      /* get the value of the point on the box */
	      if (amitk_raw_data_includes_voxel(data_set->raw_data, box_voxel[l]))
		box_value[l] = AMITK_DATA_SET_`'m4_Variable_Type`'_`'m4_Scale_Dim`'_CONTENT(data_set, box_voxel[l]);
	      else
		box_value[l] = empty;
	    }
	    
	    /* do the x direction linear interpolation of the sets of two points */
	    for (l=0;l<8;l=l+2) {
	      max_diff = box_point[l+1].x-box_point[l].x;
	      weight1 = ((max_diff - (ds_point.x - box_point[l].x))/max_diff);
	      weight2 = ((max_diff - (box_point[l+1].x - ds_point.x))/max_diff);
	      box_value[l] = (box_value[l] * weight1) + (box_value[l+1] * weight2);
	    }

	    /* do the y direction linear interpolation of the sets of two points */
	    for (l=0;l<8;l=l+4) {
	      max_diff = box_point[l+2].y-box_point[l].y;
	      weight1 = ((max_diff - (ds_point.y - box_point[l].y))/max_diff);
	      weight2 = ((max_diff - (box_point[l+2].y - ds_point.y))/max_diff);
	      box_value[l] = (box_value[l] * weight1) + (box_value[l+2] * weight2);
	    }

	    /* do the z direction linear interpolation of the sets of two points */
	    for (l=0;l<8;l=l+8) {
	      max_diff = box_point[l+4].z-box_point[l].z;
	      weight1 = ((max_diff - (ds_point.z - box_point[l].z))/max_diff);
	      weight2 = ((max_diff - (box_point[l+4].z - ds_point.z))/max_diff);
	      box_value[l] = (box_value[l] * weight1) + (box_value[l+4] * weight2);
	    }

	    AMITK_RAW_DATA_DOUBLE_SET_CONTENT(slice->raw_data,i_voxel)+=weight*box_value[0];

	    slice_point.x += slice->voxel_size.x; 
	  }
	}
      }
    }
    break;
  case AMITK_INTERPOLATION_NEAREST_NEIGHBOR:
  default:  
    /* figure out what point in the data set we're going to start at */
    start_point.x = ((amide_real_t) start.x+0.5) * slice->voxel_size.x;
    start_point.y = ((amide_real_t) start.y+0.5) * slice->voxel_size.y;
    start_point.z = voxel_length/2.0;
    start_point = amitk_space_s2s(slice_space, data_set_space, start_point);

    /* figure out what stepping one voxel in a given direction in our slice cooresponds to in our data set */
    for (i_axis = 0; i_axis < AMITK_AXIS_NUM; i_axis++) {
      alt.x = (i_axis == AMITK_AXIS_X) ? slice->voxel_size.x : 0.0;
      alt.y = (i_axis == AMITK_AXIS_Y) ? slice->voxel_size.y : 0.0;
      alt.z = (i_axis == AMITK_AXIS_Z) ? voxel_length : 0.0;
      alt = point_add(point_sub(amitk_space_s2b(slice_space, alt),
			  AMITK_SPACE_OFFSET(slice_space)),
		      AMITK_SPACE_OFFSET(data_set_space));
      stride[i_axis] = amitk_space_b2s(data_set_space, alt);
    }

    /* iterate over the number of frames we'll be incorporating into this slice */
    for (i_frame = start_frame; i_frame <= end_frame; i_frame++) {

      /* averaging over more then one frame */
      if (end_frame-start_frame > 0) {
	if (i_frame == start_frame)
	  time_weight = (amitk_data_set_get_end_time(data_set, start_frame)-start_time)/duration;
	else if (i_frame == end_frame)
	  time_weight = (end_time-amitk_data_set_get_start_time(data_set, end_frame))/duration;
	else
	  time_weight = amitk_data_set_get_frame_duration(data_set, i_frame)/duration;
      } else
	time_weight = 1.0;

      ds_point = start_point;
      /* iterate over the number of planes we'll be compressing into this slice */
      for (z = 0; z < ceil(z_steps); z++) { 
	last[AMITK_AXIS_Z] = ds_point;

	/* weight is between 0 and 1, this is used to weight the last voxel  in the slice's z direction */
	if (floor(z_steps) > z)
	  weight = time_weight/z_steps;
	else
	  weight = time_weight*(z_steps-floor(z_steps)) / z_steps;
	
	/* iterate over the y dimension */
	for (i_voxel.y = start.y; i_voxel.y <= end.y; i_voxel.y++) {
	  last[AMITK_AXIS_Y] = ds_point;

	  /* and iteratate over x */
	  for (i_voxel.x = start.x; i_voxel.x <= end.x; i_voxel.x++) {
	    POINT_TO_VOXEL(ds_point, data_set->voxel_size, i_frame, ds_voxel);
	    if (!amitk_raw_data_includes_voxel(data_set->raw_data,ds_voxel))
	      AMITK_RAW_DATA_DOUBLE_SET_CONTENT(slice->raw_data,i_voxel) += weight*empty;
	    else
	      AMITK_RAW_DATA_DOUBLE_SET_CONTENT(slice->raw_data,i_voxel) += 
		weight*AMITK_DATA_SET_`'m4_Variable_Type`'_`'m4_Scale_Dim`'_CONTENT(data_set,ds_voxel);
	    POINT_ADD(ds_point, stride[AMITK_AXIS_X], ds_point); 
	  }
	  POINT_ADD(last[AMITK_AXIS_Y], stride[AMITK_AXIS_Y], ds_point);
	}
	POINT_ADD(last[AMITK_AXIS_Z], stride[AMITK_AXIS_Z], ds_point); 
      }
    }
    break;
  }

  return slice;
}

