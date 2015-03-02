/* ui_rendering_movie_dialog.c
 *
 * Part of amide - Amide's a Medical Image Dataset Examiner
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

#include "config.h"

#ifdef AMIDE_LIBVOLPACK_SUPPORT
#ifdef AMIDE_MPEG_ENCODE_SUPPORT

#include <gnome.h>
#include <math.h>
#include <sys/stat.h>
#include "amide.h"
#include "rendering.h"
#include "study.h"
#include "image.h"
#include "ui_rendering.h"
#include "ui_rendering_movie_dialog.h"
#include "ui_rendering_movie_dialog_callbacks.h"


/* destroy a ui_rendering_movie data structure */
ui_rendering_movie_t * ui_rendering_movie_free(ui_rendering_movie_t * ui_rendering_movie) {

  if (ui_rendering_movie == NULL)
    return ui_rendering_movie;

  /* sanity checks */
  g_return_val_if_fail(ui_rendering_movie->reference_count > 0, NULL);

  /* remove a reference count */
  ui_rendering_movie->reference_count--;

  /* things we always do */

  /* things we do if we've removed all references */
  if (ui_rendering_movie->reference_count == 0) {
#ifdef AMIDE_DEBUG
    g_print("freeing ui_rendering_movie\n");
#endif
    g_free(ui_rendering_movie);
    ui_rendering_movie = NULL;
  }
    
  return ui_rendering_movie;

}

/* malloc and initialize a ui_rendering_movie data structure */
ui_rendering_movie_t * ui_rendering_movie_init(void) {

  ui_rendering_movie_t * ui_rendering_movie;


  /* alloc space for the data structure */
  if ((ui_rendering_movie = (ui_rendering_movie_t *) g_malloc(sizeof(ui_rendering_movie_t))) == NULL) {
    g_warning("%s: couldn't allocate space for ui_rendering_movie_t",PACKAGE);
    return NULL;
  }

  ui_rendering_movie->reference_count = 1;
  ui_rendering_movie->dialog = NULL;
  ui_rendering_movie->num_frames = MOVIE_DEFAULT_FRAMES;
  ui_rendering_movie->rotation[XAXIS] = 0;
  ui_rendering_movie->rotation[YAXIS] = 0;
  ui_rendering_movie->rotation[ZAXIS] = 1;
  ui_rendering_movie->ui_rendering = NULL;

 return ui_rendering_movie;
}




/* perform the movie generation */
void ui_rendering_movie_dialog_perform(ui_rendering_movie_t * ui_rendering_movie, gchar * output_file_name) {

  guint i_frame;
  gdouble rotation_step[NUM_AXIS];
  axis_t i_axis;
  gint return_val = 1;
   FILE * param_file;
  GTimeVal current_time;
  guint i;
  gchar * frame_file_name = NULL;
  gchar * param_file_name = NULL;
  gchar * temp_dir = NULL;
  gchar * temp_string;
  struct stat file_info;
  GList * file_list = NULL;
  GList * temp_list;
  GdkImlibImage * ppm_image;



  /* figure out what the temp directory is */
  temp_dir = g_get_tmp_dir();

  /* get the current time, this is so we have a, "hopefully" unique file name */
  g_get_current_time(&current_time);

#ifdef AMIDE_DEBUG
  g_print("Total number of frames to do:\t%d\n",ui_rendering_movie->num_frames);
  g_print("Frame:\n");
#endif

  /* start generating the frames */
  for (i_frame = 0; i_frame < ui_rendering_movie->num_frames; i_frame++) {
    if (return_val == 1) {

      /* figure out the rotation for this frame */
      for (i_axis = 0; i_axis < NUM_AXIS ; i_axis++) {
	rotation_step[i_axis] = 
	  (((double) i_frame)*2.0*M_PI*ui_rendering_movie->rotation[i_axis])
	  / ui_rendering_movie->num_frames;
	rendering_context_set_rotation(ui_rendering_movie->ui_rendering->axis_context, 
				       i_axis, rotation_step[i_axis]);
	rendering_list_set_rotation(ui_rendering_movie->ui_rendering->contexts, 
				    i_axis, rotation_step[i_axis]);
      }
      
#ifdef AMIDE_DEBUG
      g_print("\t%d\tRendering",i_frame);
#endif
      
      /* render the contexts */
      ui_rendering_update_canvases(ui_rendering_movie->ui_rendering); /* render now */
      
#ifdef AMIDE_DEBUG
      g_print("\tWriting\n");
#endif

      i = 0;
      do {
	if (i > 0) g_free(frame_file_name);
	i++;
	frame_file_name = g_strdup_printf("%s/%ld_%d_amide_rendering_%d.ppm",
					  temp_dir, current_time.tv_sec, i, i_frame);
      } while (stat(frame_file_name, &file_info) == 0);
      
      /* yep, we still need to use imlib to export ppm files, as gdk_pixbuf doesn't seem to have
	 this capability */
      ppm_image = 
	gdk_imlib_create_image_from_data(gdk_pixbuf_get_pixels(ui_rendering_movie->ui_rendering->main_image), 
					 NULL,
					 gdk_pixbuf_get_width(ui_rendering_movie->ui_rendering->main_image), 
					 gdk_pixbuf_get_height(ui_rendering_movie->ui_rendering->main_image));

      return_val = gdk_imlib_save_image_to_ppm(ppm_image, frame_file_name);
      gdk_imlib_destroy_image(ppm_image);

      
      if (return_val != 1) 
	g_warning("%s: saving of following ppm file failed: %s\n\tAborting movie generation",
		  PACKAGE, frame_file_name);
      else
	file_list = g_list_append(file_list, frame_file_name);
      
      /* do any events pending, this allows the canvas to get displayed */
      while (gtk_events_pending()) 
	gtk_main_iteration();
      
      /* and unrotate the rendering contexts so that we can properly rerotate
	 for the next frame */
      for (i_axis = NUM_AXIS; i_axis > 0 ; i_axis--) {
	rendering_context_set_rotation(ui_rendering_movie->ui_rendering->axis_context, 
				       i_axis-1, -rotation_step[i_axis-1]);
	rendering_list_set_rotation(ui_rendering_movie->ui_rendering->contexts, 
				    i_axis-1, -rotation_step[i_axis-1]);
      }
    }
  }


  /* and rerender one last time to get the last rotation */
  ui_rendering_update_canvases(ui_rendering_movie->ui_rendering); /* render now */


  if (return_val == 1) { /* do the mpeg stuff */
#ifdef AMIDE_DEBUG
    g_print("Encoding the MPEG\n");
#endif
      i = 0;
      do {
	if (i > 0) g_free(param_file_name); 
	i++;
	param_file_name = g_strdup_printf("%s/%ld_%d_amide_rendering.param",
					  temp_dir, current_time.tv_sec, i);
      } while (stat(param_file_name, &file_info) == 0);


      /* open the parameter file for writing */
      if ((param_file = fopen(param_file_name, "w")) != NULL)

	  fprintf(param_file, "PATTERN\tIBBPBBPBBPBBPBBP\n");
	  fprintf(param_file, "OUTPUT\t%s\n",output_file_name);
	  fprintf(param_file, "INPUT_DIR\t%s\n","");
	  fprintf(param_file, "INPUT\n");

	  temp_list = file_list;
	  while (temp_list != NULL) {
	    frame_file_name = temp_list->data;
	    fprintf(param_file, "%s\n",frame_file_name);
	    temp_list = temp_list->next;
	  }
	  fprintf(param_file, "END_INPUT\n");
	  fprintf(param_file, "BASE_FILE_FORMAT\tPPM\n");
	  fprintf(param_file, "INPUT_CONVERT\t*\n");
	  fprintf(param_file, "GOP_SIZE\t16\n");
	  fprintf(param_file, "SLICES_PER_FRAME\t1\n");
	  fprintf(param_file, "PIXEL\tHALF\n");
	  fprintf(param_file, "RANGE\t10\n");
	  fprintf(param_file, "PSEARCH_ALG\tEXHAUSTIVE\n");
	  fprintf(param_file, "BSEARCH_ALG\tCROSS2\n");
	  fprintf(param_file, "IQSCALE\t8\n");
	  fprintf(param_file, "PQSCALE\t10\n");
	  fprintf(param_file, "BQSCALE\t25\n");
	  fprintf(param_file, "BQSCALE\t25\n");
	  fprintf(param_file, "REFERENCE_FRAME\tDECODED\n");
	  fprintf(param_file, "FRAME_RATE\t30\n");

	  file_list = g_list_append(file_list, param_file_name);
	  fclose(param_file);

	  /* delete the previous .mpg file if it existed */
	  if (stat(output_file_name, &file_info) == 0) {
	    if (S_ISREG(file_info.st_mode)) {
	      if (unlink(output_file_name) != 0) {
		g_warning("%s: Couldn't unlink file: %s",PACKAGE, output_file_name);
		return_val = -1;
	      }
	    } else {
	      g_warning("%s: Unrecognized file type for file: %s, couldn't delete",
			PACKAGE, output_file_name);
	      return_val = -1;
	    }
	  }

	  if (return_val != -1) {
	    /* and run the mpeg encoding process */
	    temp_string = g_strdup_printf("%s %s",AMIDE_MPEG_ENCODE_BIN, param_file_name);
	    return_val = system(temp_string);
	    g_free(temp_string);
	    
	    if ((return_val == -1) || (return_val == 127)) {
	      g_warning("%s: executing of %s for creation of mpeg movie was unsucessful", 
			PACKAGE, AMIDE_MPEG_ENCODE_BIN);
	      return_val = -1;
	    }
	  }
  }


  /* do some cleanups */
  /* note, temp_dir is just a pointer to a string, not a copy, so don't free it */

  while (file_list != NULL) {
    frame_file_name = file_list->data;
    file_list = g_list_remove(file_list, frame_file_name);
    unlink(frame_file_name);
    g_free(frame_file_name);
  }
    

  return;
}

/* function that sets up the rendering options dialog */
ui_rendering_movie_t * ui_rendering_movie_dialog_create(ui_rendering_t * ui_rendering) {
  
  ui_rendering_movie_t * ui_rendering_movie;
  gchar * temp_string = NULL;
  GtkWidget * packing_table;
  GtkWidget * hseparator;
  GtkWidget * label;
  GtkWidget * entry;
  guint table_row = 0;
  axis_t i_axis;
  
  if (ui_rendering->movie != NULL)
    return ui_rendering->movie;

  ui_rendering_movie = ui_rendering_movie_init();
  ui_rendering->movie = ui_rendering_movie;
  ui_rendering_movie->ui_rendering = ui_rendering;
    
  temp_string = g_strdup_printf("%s: Rendering Movie Generation Dialog",PACKAGE);
  ui_rendering_movie->dialog = gnome_property_box_new();
  gtk_window_set_title(GTK_WINDOW(ui_rendering_movie->dialog), temp_string);
  g_free(temp_string);

  /* setup the callbacks for app */
  gtk_signal_connect(GTK_OBJECT(ui_rendering_movie->dialog), "close",
  		     GTK_SIGNAL_FUNC(ui_rendering_movie_dialog_callbacks_close_event),
  		     ui_rendering_movie);
  gtk_signal_connect(GTK_OBJECT(ui_rendering_movie->dialog), "apply",
  		     GTK_SIGNAL_FUNC(ui_rendering_movie_dialog_callbacks_apply), 
		     ui_rendering_movie);
  gtk_signal_connect(GTK_OBJECT(ui_rendering_movie->dialog), "help",
		     GTK_SIGNAL_FUNC(ui_rendering_movie_dialog_callbacks_help),
		     ui_rendering_movie);


  /* ---------------------------
     Basic Parameters page 
     --------------------------- */


  /* start making the widgets for this dialog box */
  packing_table = gtk_table_new(5,2,FALSE);
  label = gtk_label_new("Movie Parameters");
  table_row=0;
  gnome_property_box_append_page (GNOME_PROPERTY_BOX(ui_rendering_movie->dialog), 
				  GTK_WIDGET(packing_table), label);

  /* widgets to specify how many frames */
  label = gtk_label_new("Frames");
  gtk_table_attach(GTK_TABLE(packing_table), GTK_WIDGET(label), 0,1,
		   table_row, table_row+1, 0, 0, X_PADDING, Y_PADDING);
  entry = gtk_entry_new();
  temp_string = g_strdup_printf("%d", ui_rendering_movie->num_frames);
  gtk_entry_set_text(GTK_ENTRY(entry), temp_string);
  g_free(temp_string);
  gtk_editable_set_editable(GTK_EDITABLE(entry), TRUE);
  gtk_signal_connect(GTK_OBJECT(entry), "changed", 
		     GTK_SIGNAL_FUNC(ui_rendering_movie_dialog_callbacks_change_frames), 
		     ui_rendering_movie);
  gtk_table_attach(GTK_TABLE(packing_table), GTK_WIDGET(entry),1,2,
		   table_row, table_row+1, GTK_FILL, 0, X_PADDING, Y_PADDING);
  table_row++;

  /* a separator for clarity */
  hseparator = gtk_hseparator_new();
  gtk_table_attach(GTK_TABLE(packing_table), GTK_WIDGET(hseparator),
		   table_row, table_row+1,0,4, GTK_FILL, 0, X_PADDING, Y_PADDING);
  table_row++;

  /* widgets to specify number of rotations on the axis */
  for (i_axis=0;i_axis<NUM_AXIS;i_axis++) {
    temp_string = g_strdup_printf("Rotations on %s", axis_names[i_axis]);
    label = gtk_label_new(temp_string);
    g_free(temp_string);
    gtk_table_attach(GTK_TABLE(packing_table), GTK_WIDGET(label), 0,1,
		     table_row, table_row+1, 0, 0, X_PADDING, Y_PADDING);

    entry = gtk_entry_new();
    temp_string = g_strdup_printf("%f", ui_rendering_movie->rotation[i_axis]);
    gtk_entry_set_text(GTK_ENTRY(entry), temp_string);
    g_free(temp_string);
    gtk_editable_set_editable(GTK_EDITABLE(entry), TRUE);
    gtk_object_set_data(GTK_OBJECT(entry), "which_entry", GINT_TO_POINTER(i_axis));
    gtk_signal_connect(GTK_OBJECT(entry), "changed", 
		       GTK_SIGNAL_FUNC(ui_rendering_movie_dialog_callbacks_change_rotation), 
		       ui_rendering_movie);
    gtk_table_attach(GTK_TABLE(packing_table), GTK_WIDGET(entry),1,2,
		     table_row, table_row+1, GTK_FILL, 0, X_PADDING, Y_PADDING);
    table_row++;
  }




  /* show all our widgets */
  gtk_widget_show_all(ui_rendering_movie->dialog);

  /* tell the dialog we've changed */
  gnome_property_box_changed(GNOME_PROPERTY_BOX(ui_rendering_movie->dialog));

  return ui_rendering_movie;
}

#endif /* AMIDE_MPEG_ENCODE_SUPPORT */
#endif /* LIBVOLPACK_SUPPORT */









