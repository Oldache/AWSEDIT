/*

 Copyright  2011  Mustapha Oldache   <mustapha.oldache@gmail.com>

  This file is part of Awsedit.

Awsedit is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Awsedit is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Awsedit.  If not, see <http://www.gnu.org/licenses/>. */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

/* widget reference */
GtkWidget *param;
gboolean stop;
gboolean werr;
/* statusbar identfificator */
guint converid;
/* buffer for reading block header */
AWSTAPE_BLKHDR   hb;
/* temporary buffer for textview */
guchar buf[65535];
/* buffer for textview */
GtkTextBuffer *buffer;
/* research buffer */
guchar fbuf[65535];
/* extract buffer */
guchar xbuf[65535];
/* temporary vector containing block locations and block lengths  */
BLKDESC (*blk_tmp)[10000];
BLKDESC (*blk_tmp_ext)[10000];
/* table for keeping block positions and lengths per file */
BLKDESC (**blk)[10000];
BLKDESC (**blk_ext)[10000];
/* vector containing block counts */
guint *blk_count;
guint *blk_count_ext;
/* aws tape file descriptor */
int f_awsfile;
/* data file descriptor */
int f_datafile;
/* file index */
guint fi;
/* block index */
guint bi;
/* position within a block */
guint pi;
/* textview find selection */
gint it1,it2;
/* set to TRUE when string is found */
gboolean find;
/* record length */
guint  lrecl=80;
/* index : hexa or decimal */
gboolean dec=TRUE;
gboolean hexadata=FALSE;
gboolean tabindex=FALSE;
gboolean convert=FALSE;
gboolean range_block=FALSE;
gboolean find=FALSE;
/* translation to ebcdic */
gboolean  ebcdic=TRUE;
gboolean  cobol=TRUE;
gboolean  unx=FALSE;
FIELDESC desc [500];
/* field selection */
guint sf;
guint ef;
// number of files 
guint i=0;
// number of descriptions
guint j=0;
guint start;
guint end;
guint block_start;
guint block_end;
guint file_start;
guint file_end;
gchar *awsfile;
GtkWidget *message_dialog;
//Extract File
gchar *datafile;
// EBCDIC to ASCII conversion table


 gchar map[256] = \



//     0000000000000000111111111111111122222222222222223333333333333333
//     0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF 
      "................................................................"
//     4444444444444444555555555555555566666666666666667777777777777777
//     0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF  
      " ...........<(+|&.........!$*).^-/..........%_>?...........#@.=\"" 
//     88888888888888889999999999999999AAAAAAAAAAAAAAAABBBBBBBBBBBBBBBB
//     0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF 
      ".abcdefghi.{.(+..jklmnopqr.}.)....stuvwxyz......................" 
//     CCCCCCCCCCCCCCCCDDDDDDDDDDDDDDDDEEEEEEEEEEEEEEEEFFFFFFFFFFFFFFFF
//     0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF 
      "{ABCDEFGHI......}JKLMNOPQR........STUVWXYZ......0123456789......";  


GtkTreeSelection *sel;

void
on_ouvrir_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
GtkTreePath *path;
guint loc;
gboolean  error = FALSE;
gboolean  tape_end = FALSE;
find=FALSE;
GtkTreeIter iter1;  /* Parent iter */
GtkTreeIter iter2;  /* Child iter  */
GtkTreeStore *store;
GtkWidget *pbar;
guint m;
guint progress;
guint total;
gfloat ftotal;

gchar label[20];
GtkWidget *dialog;
dialog = gtk_file_chooser_dialog_new ("Open File",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);


if(!user_data)
 if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
   awsfile = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
   gtk_widget_destroy (dialog);
  } 
  else 
  {
  gtk_widget_destroy (dialog);
  return;
  }
else
  awsfile=user_data;

buffer=gtk_text_buffer_new(NULL);
gtk_text_view_set_buffer(GTK_TEXT_VIEW(textview),buffer); 

if(f_awsfile==3) close(f_awsfile);
f_awsfile = open (awsfile,O_RDONLY,0);

if (f_awsfile!=-1)
{ 
store = gtk_tree_store_new (N_COLUMNS,       /* Total number of columns */
                                          G_TYPE_STRING,   /* Blocks             */
                                          G_TYPE_STRING);   /* File                  */
converid=gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar),"conversion");
gtk_statusbar_pop(GTK_STATUSBAR(statusbar),converid);
pbar = create_prog_bar();
gtk_window_set_skip_taskbar_hint(GTK_WINDOW(pbar),TRUE);
gtk_widget_show(pbar);
total=lseek(f_awsfile,0,2);
progress = total/1000;
ftotal=total;
lseek(f_awsfile,0,0);
i=0;
m=0;
if (blk_tmp) g_free(blk_tmp);
blk_tmp=g_malloc(10000*sizeof(BLKDESC));
if (blk) g_free(blk);
blk=g_malloc0(1000*sizeof(guint));
if (blk_count) g_free(blk_count);
blk_count=g_malloc(1000*sizeof(guint));
blk_count[i]=0;
gtk_tree_store_append (store, &iter1, NULL);
loc=0;

do
 {
/* progress bar relative code */
  if ( loc >= progress && total > 0) 
    {
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(progressbar),loc/ftotal);
      gdk_window_process_all_updates();
      progress = loc+(total/1000);
    }
if(loc+6<=total)
 {
 read(f_awsfile,&hb,6);
 loc=loc+6;
/* new record */
     if (hb.flags1.newrec && hb.flags1.endrec && !hb.flags1.segmented && !hb.flags1.tapemark && !hb.flags2 && !hb.flags1.unused)
       if(loc+hb.curblkl<=total)
         {
           blk_count[i]++;
/* extend memory for more blocks */ 
           if(fmod(blk_count[i],10000)==0)
            {
             blk_tmp_ext=g_malloc((blk_count[i]+10000)*sizeof(BLKDESC));
             g_memmove(blk_tmp_ext,blk_tmp,blk_count[i]*sizeof(BLKDESC));
             g_free(blk_tmp);
             blk_tmp=blk_tmp_ext;
            }
           (*blk_tmp)[m].loc=loc;
           (*blk_tmp)[m].length=hb.curblkl;
           loc=lseek(f_awsfile,hb.curblkl,1);
           m++;
           sprintf(label,"%6d  %5d",m,hb.curblkl);
           gtk_tree_store_append (store, &iter2, &iter1);  /* Acquire a child iterator */
           gtk_tree_store_set (store, &iter2, BLOCK_COLUMN , label, -1);
          }
       else error=TRUE;
     else
/*new tapemark */
        if (!hb.flags1.newrec && !hb.flags1.endrec && !hb.flags1.segmented && hb.flags1.tapemark && !hb.flags2 && !hb.flags1.unused)
          {
            if (blk_count[i]>0)
             {
              if (blk[i]) g_free(blk[i]);
              blk[i] = g_malloc(blk_count[i]*sizeof(BLKDESC));
              for (m=0 ; m < blk_count[i] ; m++)
                (*blk[i])[m] = (*blk_tmp)[m] ;
              sprintf(label,"File%3d  %5d",i+1,blk_count[i]);
              gtk_tree_store_set (store, &iter1, FILE_COLUMN , label, -1);
              i++;
/* extend memory for more files */
           if(fmod(i,1000)==0)
            {
             blk_count_ext=g_malloc((i+1000)*sizeof(guint));
             g_memmove(blk_count_ext,blk_count,i*sizeof(guint));
             g_free(blk_count);
             blk_count=blk_count_ext;
             blk_ext=g_malloc0((i+1000)*sizeof(guint));
             g_memmove(blk_ext,blk,i*sizeof(BLKDESC));
             g_free(blk);
             blk=blk_ext;
            }
              m=0;
              blk_count[i]=0;
              gtk_tree_store_append (store, &iter1, NULL);            
            }
           } 
        else
            error=TRUE;
}
else
 if(loc==total)
   tape_end=TRUE;
 else
   error=TRUE;
} while (tape_end==FALSE && error==FALSE);


gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (store));

/*tape ending with error or without tapemark*/
if(blk_count[i]>0)
      {
           if (blk[i]) g_free(blk[i]);
           blk[i] = g_malloc(blk_count[i]*sizeof(BLKDESC));
           for (m=0 ; m < blk_count[i] ; m++)
           (*blk[i])[m] = (*blk_tmp)[m] ;
           if(tape_end==TRUE)
           sprintf(label,"File%3d  %5d",i+1,blk_count[i]);
           else
           sprintf(label,"File truncated");
           gtk_tree_store_set (store, &iter1, FILE_COLUMN , label, -1);
           i++;
        }
else
     gtk_tree_store_remove (store, &iter1);

/* on error */
while (gtk_events_pending ())
	  gtk_main_iteration ();
if(error==TRUE)
         {
            message_dialog = gtk_message_dialog_new (GTK_WINDOW(awsedit),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "Erreur dans le format du fichier AWS tape");
            gtk_window_set_title(GTK_WINDOW(message_dialog),"Erreur Lecture");
            run_dialog();
           }

gtk_widget_destroy(pbar);
if (i>0)
   {
/* Setup the selection handler */
    if(!sel)
    {
    sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    gtk_tree_selection_set_mode (sel, GTK_SELECTION_MULTIPLE);
    g_signal_connect (G_OBJECT (sel), "changed",
                  G_CALLBACK (tree_selection_changed),
                  NULL);
    }
    path=gtk_tree_path_new_from_indices(0,-1);
    gtk_tree_selection_unselect_all(sel);
    gtk_tree_selection_select_path(sel,path);
    gtk_tree_path_free(path);
    gtk_widget_set_sensitive(rechercher,TRUE);
   }
else
   {
    gtk_widget_set_sensitive(rechercher,FALSE);
   }
 }
}



void
tree_selection_changed (GtkTreeSelection *selection, gpointer data)
{
GtkTextMark *mark;
GtkTextIter iterbuf, iterline, iterindex;
// GtkTreeIter iter;
GtkTreeModel *model;
// gchar *p,*sep;
gchar *rec, *xrec;
gchar label[20], charseq[5];
guint8  ph,pl;
guint p,l,k,q,n,m;
GList *list,*last,*first;
gint *indice_first,*indice_last;
list=gtk_tree_selection_get_selected_rows(selection,&model);
if(list)
{
first=g_list_first(list);
last=g_list_last(list);
indice_first=gtk_tree_path_get_indices(first->data);
indice_last=gtk_tree_path_get_indices(last->data);
n=indice_last[0]-indice_first[0]+1;
m=indice_last[1]-indice_first[1]+1;
if(gtk_tree_path_get_depth(first->data)==1 && gtk_tree_path_get_depth(last->data)==1 &&
          g_list_length(list)==n)
    {
    bi=0;
    block_start=bi;
    block_end=bi;
    range_block=FALSE;
    if(n>1)
    {
     if(indice_first[0]!=start)
     fi=indice_first[0];
     else
     if(indice_last[0]!=end)
     fi=indice_last[0];
    }
    else
    fi=indice_last[0];    
    start=indice_first[0];
    end=indice_last[0];
    file_start=start;
    file_end=end;
    }
else
   if(gtk_tree_path_get_depth(first->data)==2 && gtk_tree_path_get_depth(last->data)==2 &&
         g_list_length(list)==m && indice_last[0]==indice_first[0])
      {
       fi=indice_last[0];
       file_start=fi;
       file_end=fi;
       if(m>1)
       {
       range_block=TRUE;
       if(indice_first[1]!=start) 
        bi=indice_first[1];
       else
        if(indice_last[1]!=end)
        bi=indice_last[1];
       }
       else
       {
       bi=indice_last[1];
       range_block=FALSE;
       }
       start=indice_first[1];
       end=indice_last[1];       
       block_start=start;
       block_end=end;      
      }
   else
      {
      if((indice_first[1]==start && indice_last[1]!=end) || indice_first[0]==start)
      gtk_tree_selection_unselect_path(selection,last->data);
      else
      gtk_tree_selection_unselect_path(selection,first->data);
      }
                g_list_foreach (list,(void *)gtk_tree_path_free, NULL);
                g_list_free(list);      
                gtk_statusbar_pop(GTK_STATUSBAR(statusbar),converid);
                sprintf(label,"File %3d",fi+1);
                gtk_statusbar_push(GTK_STATUSBAR(statusbar),converid,label);
                lseek(f_awsfile,(*blk[fi])[bi].loc,0);
                read(f_awsfile,buf,(*blk[fi])[bi].length);
                if (!fmod((*blk[fi])[bi].length,lrecl) && !tabindex && !hexadata)
                  gtk_widget_set_sensitive(conversion,TRUE);
                else
                  gtk_widget_set_sensitive(conversion,FALSE);
                  buffer=gtk_text_buffer_new(NULL);
                  gtk_text_buffer_get_iter_at_offset (buffer,&iterbuf,0);
                  gtk_text_buffer_create_tag (buffer, "monospace",
			      "family", "monospace",NULL);
                  gtk_text_buffer_create_tag (buffer, "red_foreground",
			      "foreground", "red", NULL);
                  gtk_text_buffer_create_tag (buffer, "blue_foreground",
			      "foreground", "blue", NULL);
                  gtk_text_buffer_create_tag (buffer, "green_background",
			      "background", "green", NULL);
                  gtk_text_buffer_create_tag (buffer, "red_background",
			      "background", "red", NULL);
                  gtk_text_buffer_create_tag (buffer, "yellow_background",
			      "background", "yellow", NULL);
                  gtk_text_view_set_buffer (GTK_TEXT_VIEW(textview),buffer);
                  rec=g_malloc(lrecl+1);
                  if(hexadata) xrec=g_malloc(2*lrecl+1);
                  k=0;
                  for(l=0; k<(*blk[fi])[bi].length; l++)
                  {
                    if(tabindex)
                     {
                       if (dec)
                         sprintf(charseq,"%4d",k);
                       else
                         sprintf(charseq,"%4x",k);
                     g_strcanon(charseq,"0123456789abcdef",'0');
                     gtk_text_buffer_insert_with_tags_by_name (buffer, &iterbuf,
			  	             charseq, 4, "monospace","red_foreground", NULL);
                     }
                  q=0;                                       
                  for (k=l*lrecl; k<(l+1)*lrecl && k<(*blk[fi])[bi].length; k++)
                  {
                  if(hexadata)
                   {
                    pl = fmod(buf[k],16);
                    ph = buf[k]/16;
                    sprintf(xrec+2*q,"%x%x",ph,pl);
                   }
                  mapcar(rec,buf[k],q);
                  q++;
                  }
/* first loop end*/
                  if(hexadata)
                   {
                   memset(xrec+2*q,' ',2*(lrecl-q));
                   gtk_text_buffer_insert_with_tags_by_name (buffer, &iterbuf,
			  	                    xrec, 2*lrecl, "monospace","blue_foreground", NULL);
                   }
                  rec[q]='\n';
                  gtk_text_buffer_insert_with_tags_by_name (buffer, &iterbuf,
				                   rec, q+1, "monospace", NULL);
                 }
/* second loop end */
         if(hexadata)
         g_free((gpointer)(xrec));
         else
         g_free((gpointer)(rec));
         if (find==TRUE)
             { 
              gint  dep=0;
              gint  mul=1;
              if (tabindex)  dep=4;
              if (hexadata)  mul=2;
              gtk_text_buffer_get_iter_at_line_index(buffer,&iterline,it1/lrecl,fmod(it1,lrecl)*mul+dep);
              gtk_text_buffer_get_iter_at_line_index(buffer,&iterindex,it2/lrecl,(fmod(it2,lrecl)+1)*mul+dep);
              gtk_text_buffer_select_range(buffer,&iterline,&iterindex);
              mark=gtk_text_buffer_get_mark(buffer,"selection_bound");
              gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(textview),mark,0.25,FALSE,0.0,0.0);
              gtk_window_set_focus(GTK_WINDOW(awsedit),textview);
              find=FALSE;
              }
              else
/* pi=0, provoque lecture nouveau block pour la recherche */
              pi=0;
  }
}


void
on_ascii_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
ebcdic=FALSE;
change_selection();
}


void
on_ebcdic_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
ebcdic=TRUE;
change_selection();
}



void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
param=create_aboutdialog();
gtk_widget_show(param);
}


void
on_textview_populate_popup             (GtkTextView     *textview,
                                        GtkMenu         *menu,
                                        gpointer         user_data)
{
GtkTextIter start_iter, end_iter;
guint fl, line_start, line_end;
gchar hexa[201];
guint  k,q;
guint8  tmp;
gtk_text_buffer_get_selection_bounds(buffer,&start_iter,&end_iter);
line_start=gtk_text_iter_get_line(&start_iter);
line_end=gtk_text_iter_get_line(&end_iter);
sf=gtk_text_iter_get_line_offset(&start_iter);
ef=gtk_text_iter_get_line_offset(&end_iter);
fl=ef-sf;
if (convert)
{
 GList *gl=gtk_menu_get_for_attach_widget(GTK_WIDGET(textview));
 gtk_menu_detach(gl->data);
 param=create_convert_menu();
if (j>0) 
    gtk_widget_set_sensitive(annul,TRUE);
    else
    gtk_widget_set_sensitive(annul,FALSE);
 if (line_start==line_end && fl)
    {
     gtk_widget_set_sensitive(packed,TRUE);
     gtk_widget_set_sensitive(zoned,TRUE);
     gtk_widget_set_sensitive(supress,TRUE);
    }
 else
   {
    gtk_widget_set_sensitive(packed,FALSE);
    gtk_widget_set_sensitive(zoned,FALSE);
    gtk_widget_set_sensitive(supress,FALSE);
   }
    gtk_widget_show_now(param);
    gtk_menu_popup(GTK_MENU(param),NULL,NULL,NULL,NULL,0,gtk_get_current_event_time());
  }
if (line_start==line_end && fl>0 && fl<100 && !tabindex && !hexadata)
 {
 k=(line_start*lrecl)+sf;
 for  (q=0 ; q<fl ; q++)
 {
   tmp = fmod(buf[k],16);
   sprintf(hexa+2*q,"%x%x",buf[k]/16,tmp);
   k++;
  }
  hexa[2*fl+1]=0;
  gtk_statusbar_pop(GTK_STATUSBAR(statusbar),converid);
  gtk_statusbar_push(GTK_STATUSBAR(statusbar),converid,hexa);
 }
}
void
on_config_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
param=create_config_dialog ();
gtk_widget_show(param);
gtk_widget_grab_focus(lrecl_spin);
gtk_spin_button_set_value(GTK_SPIN_BUTTON(lrecl_spin),lrecl);
GtkTooltips *button_bar_tips;
button_bar_tips = gtk_tooltips_new ();
gtk_tooltips_set_tip(GTK_TOOLTIPS (button_bar_tips),index_check,"Adjoindre un index aux lignes","");
gtk_tooltips_set_tip(GTK_TOOLTIPS (button_bar_tips),hexa_index_radio,"Index en hexadecimales","");
gtk_tooltips_set_tip(GTK_TOOLTIPS (button_bar_tips),decimal_index_radio,"Index en entiers","");
gtk_tooltips_set_tip(GTK_TOOLTIPS (button_bar_tips),hexa_data_check,"Afficher un dump des lignes en hexadecimales","");
gtk_tooltips_set_tip(GTK_TOOLTIPS (button_bar_tips),lrecl_spin,"la Longueur de l'enregistrement logique.\n Le mode conversion ne s'activera que si la longueur du block courant est un multiple de ce nombre.","");
if (tabindex)
   {
   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(index_check),TRUE);
   gtk_widget_set_sensitive(decimal_index_radio,TRUE);
   gtk_widget_set_sensitive(hexa_index_radio,TRUE);
   }
else
   {
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(index_check),FALSE);
   gtk_widget_set_sensitive(decimal_index_radio,FALSE);
   gtk_widget_set_sensitive(hexa_index_radio,FALSE);
   }
if (dec)
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(decimal_index_radio),TRUE);
else
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hexa_index_radio),TRUE);
if (hexadata)
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hexa_data_check),TRUE);
else
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hexa_data_check),FALSE);

}

/* dialog boxes button */

void
on_extract_dialog_cancelbutton_clicked (GtkButton       *button,
                                        gpointer         user_data)
{
gtk_widget_destroy(param);
}

/* thread extract function */

gpointer extract_file (gpointer  data)
{
GtkWidget *pbar=data;
/* misc indices */
guint f,k,l,m,n,p,q;
guint *type;
/* four high bits */
guint8 ph;
/* four low bits  */
guint8 pl;
guint8 car;
gchar *rec;
gfloat ftotal;
guint progress;
guint blk_start,blk_end;
rec=g_malloc(2*lrecl);
type=g_malloc(sizeof(guint)*lrecl);

/* initialisation */

for (n=0 ; n<lrecl ; n++)
 {
  type[n]=NORMAL;
 }

for (p=0 ; p<j ; p++)
 {
   for(n=desc[p].start ; n<desc[p].end ; n++) 
      {
       type[n]=desc[p].type;
      }
 }

ftotal=0;
if(range_block==FALSE)
{
for(n=file_start; n<=file_end && n<i ;n++)
ftotal=ftotal+blk_count[n];
}
else
{
ftotal=block_end-block_start+1;
}

/* extract data file */ 
p=0;
for(f=file_start; f<=file_end && f<i && stop==FALSE && werr==FALSE; f++)
 {
  if (range_block==TRUE)
  {
  blk_start=block_start;
  blk_end=block_end;  
  }
  else
  {
  blk_start=0;
  blk_end=blk_count[f];
  }
  for (m=blk_start; m<=blk_end && m<blk_count[f] && stop==FALSE && werr==FALSE; m++)
   {
  /* progress bar relative code */
   p++;
   if(fmod(p,10)==0)
    {
    gdk_threads_enter();
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(pbar),p/ftotal);
    gdk_window_process_all_updates();
    gdk_threads_leave();
    }
    lseek(f_awsfile,(*blk[f])[m].loc,0);
    read(f_awsfile,xbuf,(*blk[f])[m].length);
    k=0;
    for (l=0; k<(*blk[f])[m].length; l++)
     {
       q=0;
       for (k=l*lrecl; k<(l+1)*lrecl && k<(*blk[f])[m].length; k++)
        {
         car=xbuf[k];
         ph=car/16;
         pl=fmod(car,16);
         n=fmod(k,lrecl);
         switch (type[n])
         {
          case NORMAL :
            mapcar(rec,car,q);
            q++;
            break;
           case PACKED :
           if (ph>9)
            {
             rec[q]='*';
             q++;
             rec[q]='*';
             q++;
            }
            else
             if (pl<10)
             {
             mapcar(rec,ph+15*16,q);
             q++;
             mapcar(rec,pl+15*16,q);
             q++;
             }
             else
                 if(cobol)
                   {
                    mapcar(rec,pl*16+ph,q);
                    q++;
                   }
                 else
                   {
                    mapcar(rec,15*16+ph,q);
                    q++;                 
                    if(pl==11 || pl==13)
                      {
                       rec[q]='-';
                       q++;
                      }
                     else
                      {          
                       rec[q]='+';
                       q++;
                      }
                     }
             break;
            case ZONED:
                 if (pl>9 || ph<10)
                 {
                   rec[q]='*';
                   q++;
                 }
                  else
                       if(cobol || ph==15)
                          {
                           mapcar(rec,car,q);
                           q++;                           
                          }                        
                         else
                          {
                            mapcar(rec,15*16+pl,q);
                            q++;                                                  
                            if(ph==11 || ph==13)
                             {
                              rec[q]='-';
                              q++;                                                
                             }
                            else                     
                             {
                             rec[q]='+';
                             q++;                                          
                             }
                           }
                 break;
           default :
              continue;
           }
/* end fourth loop */
         }
         if (unx==FALSE) {
	  rec[q]=13;
          q++;
	 } 
       rec[q]=10;
       if(write(f_datafile,rec,q+1)==-1)
        {
        werr=TRUE;
        break;
        }
       }
/* end third loop */
     }
/* end second loop */
  }
/* end first loop */
     close(f_datafile);
     g_free(rec);
     g_free (datafile);
     if(stop==FALSE || werr==TRUE)
     {
     gdk_threads_enter();
     gtk_dialog_response(GTK_DIALOG(param),GTK_RESPONSE_REJECT);
     gdk_threads_leave();
     }  
     g_thread_exit(NULL);

}

void
on_extract_dialog_okbutton_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
GtkWidget *dialog; 
GtkWidget *label;
gchar *name, *generic,*p,*q;
GThread  *ret;
GtkWidget *pbar;
GdkCursor *watch=gdk_cursor_new(GDK_WATCH);
GdkCursor *left_ptr=gdk_cursor_new(GDK_LEFT_PTR);
/* thread function data */
gtk_widget_destroy(param);
dialog = gtk_file_chooser_dialog_new ("Save File",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), g_get_current_dir());
p=strrchr(awsfile,'/');
if(!p) p=awsfile; else p++;
q=strrchr(p,'.');
generic=g_malloc0(strlen(p)+1);
name=g_malloc0(strlen(p)+100);
if(q)
strncpy(generic,p,strlen(p)-strlen(q));
else
strncpy(generic,p,strlen(p));
if(range_block==TRUE)
sprintf(name,"%s-file%d-blocks-%d-%d.txt",generic,file_start+1,block_start+1,block_end+1);
else
sprintf(name,"%s-Files-%d-%d.txt",generic,file_start+1,file_end+1);
g_free((gpointer)(generic));
gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), name);
g_free((gpointer)(name));
if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
 {
 datafile = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
 gtk_widget_destroy (dialog);
 f_datafile = open(datafile,O_CREAT|O_TRUNC|O_WRONLY,S_IREAD|S_IWRITE);
 if(f_datafile>0)
 {  
  param=gtk_dialog_new_with_buttons("Enregistrement",NULL,
                     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                     GTK_STOCK_CANCEL,
                     GTK_RESPONSE_REJECT,
                     NULL);
label = gtk_label_new ("\n\nClicker sur Annuler pour aborter l'Extraction.\n\n");
gtk_container_add (GTK_CONTAINER (GTK_DIALOG(param)->vbox), label);
pbar = gtk_progress_bar_new();
gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbar),"Extraction en cours");
gtk_container_add (GTK_CONTAINER (GTK_DIALOG(param)->vbox),pbar);
gtk_widget_show_all(param);
stop=FALSE;
werr=FALSE;
ret=g_thread_create(extract_file,pbar,TRUE,NULL);
gdk_window_set_cursor(awsedit->window,watch);
gtk_dialog_run (GTK_DIALOG (param));
stop=TRUE;
g_thread_join(ret);
if(werr==TRUE)
         {
            message_dialog = gtk_message_dialog_new (GTK_WINDOW(awsedit),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "Probablement Disque plein");
            gtk_window_set_title(GTK_WINDOW(message_dialog),"Erreur Ecriture");
            run_dialog();
          }     
gtk_widget_destroy(param);
gdk_window_set_cursor(awsedit->window,left_ptr);
  }
 }
}

void
on_config_dialog_cancelbutton_clicked  (GtkButton       *button,
                                        gpointer         user_data)
{
gtk_widget_destroy(param);
}


void
on_config_dialog_okbutton_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
gboolean p;
guint l;

if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(index_check)))
  {
   tabindex=TRUE;
   if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(decimal_index_radio)))
     dec=TRUE;
   else
     dec=FALSE;
   }
else 
    tabindex=FALSE;  
if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hexa_data_check)))
    hexadata=TRUE;
else 
    hexadata=FALSE;
lrecl=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lrecl_spin));
if(lrecl+lrecl*2*hexadata>4750 && hexadata) lrecl=4750/3;
gtk_widget_destroy(param);
change_selection();
}


/* popup menu functions */

void
on_packed_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
process_field(PACKED);
}


void
on_supress_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
process_field(SUPRESS);
}


void
on_zoned_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
process_field(ZONED);
}

void
on_enregistrer_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
 
 param=create_extract_dialog();
 gtk_widget_show(param);

 if(end-start>0)
 {
 gtk_widget_set_sensitive(files_radio,FALSE);
 gtk_widget_set_sensitive(blocks_radio,FALSE);
 }
 if(range_block==TRUE)
 gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (blocks_radio), TRUE);
 else
 gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (files_radio), TRUE);
 GtkTooltips *button_bar_tips;
 button_bar_tips = gtk_tooltips_new ();
 gtk_tooltips_set_tip(GTK_TOOLTIPS (button_bar_tips),unix_radio,"Rajout des carateres 0A (new line)","");
 gtk_tooltips_set_tip(GTK_TOOLTIPS (button_bar_tips),start_spin,"Premier fichier/block","");
 gtk_tooltips_set_tip(GTK_TOOLTIPS (button_bar_tips),end_spin,"Dernier fichier/block","");
 gtk_tooltips_set_tip(GTK_TOOLTIPS (button_bar_tips),dos_radio,"Rajout des carateres 0D 0A (carriage return, new line)","");
}

/* coversion menu and submenu */

void
on_conversion_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
GtkTreePath *path;
j=0;
if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(conversion))==TRUE)
  { 
   convert=TRUE;
   if(rechercher_dialog)
   {
   gtk_widget_destroy(param);
   gtk_widget_destroyed(rechercher_dialog,&rechercher_dialog);
   }
   gtk_widget_set_sensitive(treeview,FALSE);
   gtk_widget_set_sensitive(config,FALSE);
   gtk_widget_set_sensitive(ouvrir,FALSE);   
   gtk_widget_set_sensitive(rechercher,FALSE); 
   gtk_statusbar_pop(GTK_STATUSBAR(statusbar),converid);
   gtk_statusbar_push(GTK_STATUSBAR(statusbar),converid,"conversion");
   message_dialog = gtk_message_dialog_new (GTK_WINDOW(awsedit),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_INFO,
                                  GTK_BUTTONS_CLOSE,
                                  "You are in the conversion mode. To select a field to translate you have to highlight the field with the mouse or the arrow keys then press the right button of the mouse. A popup menu will rise. Choose a type to convert. Before quitting save the description.");
   gtk_window_set_title(GTK_WINDOW(message_dialog),"Aide pour le mode Conversion");
   run_dialog();
  }
  else 
  { 
   change_selection();
   convert=FALSE;
   gtk_widget_set_sensitive(treeview,TRUE);
   gtk_widget_set_sensitive(config,TRUE);
   gtk_widget_set_sensitive(ouvrir,TRUE);
   gtk_widget_set_sensitive(rechercher,TRUE);
   }
}


void
on_charger_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
gchar *ctlfile;
GtkWidget *dialog;
gint p;
FILE *f_ctlfile;
gboolean error=FALSE;
/* gint l; */
dialog = gtk_file_chooser_dialog_new ("Open File",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);

if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
   ctlfile = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
   gtk_widget_destroy (dialog);
  } 
  else 
  {
  gtk_widget_destroy (dialog);
  return;
  }
f_ctlfile = fopen(ctlfile,"r");
if(f_ctlfile)
 {
/*  fread(&l,sizeof(int),1,f_ctlfile); 
  if (feof(f_ctlfile) || ferror(f_ctlfile) || l!=lrecl)
  error=TRUE; */
  fread(&j,sizeof(int),1,f_ctlfile);
  if (feof(f_ctlfile) || ferror(f_ctlfile) || j>500)
  error=TRUE;
  p=0;
  while ( error==FALSE && p<j)
   {
    fread(&desc[p],sizeof(desc[p]),1,f_ctlfile);
    if (feof(f_ctlfile) || ferror(f_ctlfile) || desc[p].type > NORMAL || desc[p].end > lrecl )
    error=TRUE;
    else
    {
     color_field(p);
     p++;
     }
   }
  if(error)
  {
   j=p;
  message_dialog = gtk_message_dialog_new (GTK_WINDOW(awsedit),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "Erreur dans le fichier de description");
  gtk_window_set_accept_focus(GTK_WINDOW(rechercher_dialog),FALSE);
  gtk_window_set_accept_focus(GTK_WINDOW(awsedit),FALSE);
  gtk_window_set_title(GTK_WINDOW(message_dialog),"Erreur Lecture");
  run_dialog();
  }
  fclose(f_ctlfile);
 }
}


void
on_sauver_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
GtkWidget *dialog;
gchar *ctlfile;
FILE *f_ctlfile;
gint p;
dialog = gtk_file_chooser_dialog_new ("Save File",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);


gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), g_get_current_dir());
gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "*.ctl");


if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {

    ctlfile = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
/* write data file */    
    f_ctlfile = fopen(ctlfile,"w");
    if ( j>0 && f_ctlfile)
    {
/*     fwrite(&lrecl,sizeof(int),1,f_ctlfile); */
     fwrite(&j,sizeof(int),1,f_ctlfile);
     for (p=0;p<j;p++)
     fwrite(&desc[p],sizeof(desc[p]),1,f_ctlfile);
     fclose(f_ctlfile);
    }
    gtk_widget_destroy (dialog);
  }  
}

void
on_index_check_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(index_check)))
  {
   gtk_widget_set_sensitive(decimal_index_radio,TRUE);
   gtk_widget_set_sensitive(hexa_index_radio,TRUE);
  }
  else
  {
   gtk_widget_set_sensitive(decimal_index_radio,FALSE);
   gtk_widget_set_sensitive(hexa_index_radio,FALSE);
  }
}

void
color_field(guint p)
{
GtkTextIter startit, endit;
guint n,l;
n=(*blk[fi])[bi].length/lrecl;
for (l=0; l<n; l++)
 {
   gtk_text_buffer_get_iter_at_line_index(buffer,&startit,l,desc[p].start);
   gtk_text_buffer_get_iter_at_line_index(buffer,&endit,l,desc[p].end);
   if (desc[p].type==PACKED)
    gtk_text_buffer_apply_tag_by_name(buffer,"green_background",&startit,&endit);
   else
    if  (desc[p].type==SUPRESS)
       gtk_text_buffer_apply_tag_by_name(buffer,"red_background",&startit,&endit);
    else
       gtk_text_buffer_apply_tag_by_name(buffer,"yellow_background",&startit,&endit);
  }
}

void
process_field(guint type)
{
gboolean valid_field=TRUE;
guint p;
if (j<500)
 {
  for ( p=0 ; p<j; p++)
  if ( desc[p].end <= sf || desc[p].start >= ef)
     continue;
  else 
   {
    valid_field=FALSE;
    break;
   }
  if (valid_field==TRUE)
   {
    desc[j].start=sf;
    desc[j].end=ef;
    desc[j].type=type;
    color_field(j);
    j++;
   }
  else
   {
     gtk_statusbar_pop(GTK_STATUSBAR(statusbar),converid); 
     gtk_statusbar_push(GTK_STATUSBAR(statusbar),converid,"error: this field overlaps another field");
   }
  }
else
 {
   gtk_statusbar_pop(GTK_STATUSBAR(statusbar),converid); 
   gtk_statusbar_push(GTK_STATUSBAR(statusbar),converid,"depassement: le nombre de champs excede 500");
 }
}

void
on_annul_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
gint l,n;
GtkTextIter startit, endit;
if (j>0)
 {
  j--;
  n=(*blk[fi])[bi].length/lrecl;
  for (l=0; l<n; l++)
  {
   gtk_text_buffer_get_iter_at_line_index(buffer,&startit,l,desc[j].start);
   gtk_text_buffer_get_iter_at_line_index(buffer,&endit,l,desc[j].end);
   if(desc[j].type==PACKED)
   gtk_text_buffer_remove_tag_by_name(buffer,"green_background",&startit,&endit);
   else
     if (desc[j].type==SUPRESS)
      gtk_text_buffer_remove_tag_by_name(buffer,"red_background",&startit,&endit);
     else
      gtk_text_buffer_remove_tag_by_name(buffer,"yellow_background",&startit,&endit);
   }
 }
}

void
mapcar(gchar *rec,guint8 car,gint q)
{
 if (ebcdic) rec[q]=map[car];
 else
  if (g_ascii_isgraph(car)) rec[q] = car;
  else
  rec[q]='.';
}
  

void
on_cobol_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
cobol=TRUE;
}


void
on_dbf_menu_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
cobol=FALSE;
}




void
on_aboutdialog_response                (GtkDialog       *dialog,
                                        gint             response_id,
                                        gpointer         user_data)
{
gtk_widget_destroy(param);
}


void
on_description_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
if (j>0)
   {
   gtk_widget_set_sensitive(charger,FALSE);
   gtk_widget_set_sensitive(sauver,TRUE);
   }
   else
   {
   gtk_widget_set_sensitive(charger,TRUE);
   gtk_widget_set_sensitive(sauver,FALSE);
   }
}


void
on_rechercher_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
GtkTooltips *button_bar_tips;
button_bar_tips = gtk_tooltips_new ();
if (!rechercher_dialog)
{
param=create_rechercher_dialog ();
gtk_entry_set_max_length(GTK_ENTRY(rechercher_entry),100);
gtk_window_set_skip_taskbar_hint(GTK_WINDOW(param),TRUE);
gtk_widget_show(param);
enable_signal();
gtk_tooltips_set_tip(GTK_TOOLTIPS (button_bar_tips),casse_check,"La chaine sera rechercher sans conversion des minusclues en majuscules","");
gtk_tooltips_set_tip(GTK_TOOLTIPS (button_bar_tips),pos_check,"La recherche se fera a partir du debut du fichier","");
gtk_tooltips_set_tip(GTK_TOOLTIPS (button_bar_tips),hexa_check,"La chaine doit etre ecrite en hexadecimale","");
}
pi=0;
}


void
on_rechercher_dialog_cancelbutton_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{
gtk_widget_destroy(param);
gtk_widget_destroyed(rechercher_dialog,&rechercher_dialog);
disable_signal();
}


void
on_rechercher_dialog_okbutton_clicked  (GtkButton       *button,
                                        gpointer         user_data)
{
const gchar *chaine;
gboolean chaine_valid;
gboolean casse, debut, hexa;
guint8 car,c,v[101];
guint f,b,p,q,k,l,m,n;
GtkTreePath *path;
chaine = gtk_entry_get_text(GTK_ENTRY(rechercher_entry));
if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(casse_check)))
 casse=TRUE;
else
 casse=FALSE;
if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pos_check))) 
 debut=TRUE;
else
 debut=FALSE;
if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hexa_check))) 
 hexa=TRUE;
else
hexa=FALSE;
chaine_valid=TRUE;
find=FALSE;
n=strlen(chaine);
q=0;
if(n>0)
if(hexa)
 {
  if(!fmod(n,2))
  {
   n=n/2;
   do
    { 
     if(g_ascii_isxdigit(chaine[q*2]) && g_ascii_isxdigit(chaine[q*2+1]))
          {
          v[q]=g_ascii_xdigit_value(chaine[q*2])*16;
          v[q]=v[q]+g_ascii_xdigit_value(chaine[q*2+1]);
          q++;
          }
     else 
          chaine_valid=FALSE;
     } while (q<n && chaine_valid==TRUE);
   }
  else
  chaine_valid=FALSE;
 }
else
 { 
  do
   {
    car=chaine[q];
/* conversion miniscule vers majuscule */
    if(car>='a' && car<='z' && casse==FALSE) car='A'+(car-'a');
    if(ebcdic)
     {
      if (car=='.')
        v[q]=75;
      else
      {
       for(c=0; c<250; c++)
        if(car==map[c])
        {
          v[q]=c;
          break;
        }
       if(c==250) chaine_valid=FALSE;
       }
      }
     else
      if(g_ascii_isgraph(car))
        v[q]=car;
      else   
      chaine_valid=FALSE;
    q++;
   } while (q<n && chaine_valid==TRUE);
 }
else
chaine_valid=FALSE;
/* traitement erreur */
if(chaine_valid==FALSE)
 {
    if(n==0)
    message_dialog = gtk_message_dialog_new (GTK_WINDOW(awsedit),
		         GTK_DIALOG_DESTROY_WITH_PARENT,
			 GTK_MESSAGE_ERROR,
			 GTK_BUTTONS_CLOSE,
			 "Chaine Vide");
    else
     if (hexa)
      message_dialog = gtk_message_dialog_new (GTK_WINDOW(awsedit),
		      GTK_DIALOG_DESTROY_WITH_PARENT,
		      GTK_MESSAGE_ERROR,
		      GTK_BUTTONS_CLOSE,
		      "Chaine Hexadecimale Invalide: \"%s\"",chaine);
     else
      message_dialog = gtk_message_dialog_new (GTK_WINDOW(awsedit),
                       GTK_DIALOG_DESTROY_WITH_PARENT,
                       GTK_MESSAGE_ERROR,
                       GTK_BUTTONS_CLOSE,
                      "Chaine Caracteres Invalide \"%s\"",chaine);
   gtk_window_set_title(GTK_WINDOW(message_dialog),"Erreur Recherche");
   run_dialog();
 }       
else
/* recherche de la chaine */
 {
  if(debut==TRUE)
   {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pos_check),FALSE);
    f=0;
    b=0;
    p=0;
   }
   else
   {
    f=fi;
    b=bi;
    p=pi;
   }
 find=FALSE;
 for(l=f ; l<i && find==FALSE; l++)
 { 
  for(m=b ; m<blk_count[l] && find==FALSE; m++)
  {
/* lecture nouveau block */
   if (p==0)
   {
     lseek(f_awsfile,(*blk[l])[m].loc,0);
     read(f_awsfile,fbuf,(*blk[l])[m].length);
    }
   q=0;
   for(k=p ; k<(*blk[l])[m].length && q<n; k++)
   {
    if (fbuf[k]==v[q])
      q++;
    else
      q=0;
   }
   if (q<n)
    p=0;
   else
    {
     find=TRUE;
     pi=k;
     it1=k-q;
     it2=k-1;
     }
   }
  b=0;
 }
 if (find==TRUE)
/*rencontre chaine*/
  {
    path=gtk_tree_path_new_from_indices(l-1,-1);
    gtk_tree_view_expand_row(GTK_TREE_VIEW(treeview),path,TRUE);
    path=gtk_tree_path_new_from_indices(l-1,m-1,-1);
    gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(treeview),path,NULL,FALSE,0.0,0.0);
    gtk_tree_selection_unselect_all(sel);
    gtk_tree_selection_select_path(sel,path);
    gtk_tree_path_free(path);
   }
 else
  {
/* restore actual block */
    lseek(f_awsfile,(*blk[fi])[bi].loc,0);
    read(f_awsfile,fbuf,(*blk[fi])[bi].length);
/* chaine non existante */ 
    if (hexa)
    message_dialog = gtk_message_dialog_new (GTK_WINDOW(awsedit),
		    GTK_DIALOG_DESTROY_WITH_PARENT,
		    GTK_MESSAGE_WARNING,
		    GTK_BUTTONS_CLOSE,
		    "Chaine Hexadecimale Introuvable: \"%s\"",chaine);
    else
    message_dialog = gtk_message_dialog_new (GTK_WINDOW(awsedit),
                     GTK_DIALOG_DESTROY_WITH_PARENT,
                     GTK_MESSAGE_WARNING,
                     GTK_BUTTONS_CLOSE,
                    "Chaine Caracteres Introuvable: \"%s\"",chaine); 
    gtk_window_set_title(GTK_WINDOW(message_dialog),"Fin Recherche");
    run_dialog();
  }
 }
}

void
on_rechercher_entry_changed            (GtkEditable     *editable,
                                        gpointer         user_data)
{
gtk_widget_grab_default(rechercher_dialog_okbutton);
pi=0;
}


void
on_rechercher_dialog_close             (GtkDialog       *dialog,
                                        gpointer         user_data)
{
gtk_widget_destroy(param);
gtk_widget_destroyed(rechercher_dialog,&rechercher_dialog);
disable_signal();
}


gboolean
on_rechercher_dialog_delete_event      (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
gtk_widget_destroy(param);
gtk_widget_destroyed(rechercher_dialog,&rechercher_dialog);
disable_signal();
  return FALSE;
}


gboolean
on_rechercher_dialog_destroy_event     (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
gtk_widget_destroy(param);
gtk_widget_destroyed(rechercher_dialog,&rechercher_dialog);
disable_signal();
  return FALSE;
}

void
change_selection()
{

if(i>0)
 { 
 GtkTreePath *path;
 gtk_tree_selection_unselect_all(sel);
 path=gtk_tree_path_new_from_indices(fi,-1);
 if (gtk_tree_view_row_expanded(GTK_TREE_VIEW(treeview),path))  path=gtk_tree_path_new_from_indices(fi,bi,-1);
/* faire apparaitre la selection de la recherche */ 
 if(rechercher_dialog && pi>0) find=TRUE;
 gtk_tree_selection_select_path(sel,path);
 gtk_tree_path_free(path);
 }
}

void
run_dialog()
{
if(!rechercher_dialog) enable_signal();
gint result=gtk_dialog_run (GTK_DIALOG (message_dialog));
if(result!=GTK_RESPONSE_NONE)
{
gtk_widget_destroy (message_dialog);
gtk_widget_destroyed(message_dialog,&message_dialog);
if(!rechercher_dialog) disable_signal();
 }
}

void
awsedit_get_focus (GtkWidget *window, gpointer  data)
{
if(message_dialog) gtk_window_present (GTK_WINDOW(message_dialog));
if(rechercher_dialog) gtk_window_set_keep_above(GTK_WINDOW(rechercher_dialog),TRUE);
}

void
awsedit_lost_focus (GtkWidget *window, gpointer  data)
{
if(rechercher_dialog) gtk_window_set_keep_above(GTK_WINDOW(rechercher_dialog),FALSE);
}

void
awsedit_iconified (GtkWidget *window, GdkEventWindowState *etat, gpointer  data)
{
if(etat->new_window_state==GDK_WINDOW_STATE_ICONIFIED)
 {
      if(message_dialog)    gtk_widget_hide(message_dialog);
      if(rechercher_dialog) gtk_widget_hide(rechercher_dialog);
 }
  else
 {
      if(rechercher_dialog) gtk_widget_show(rechercher_dialog);
      if(message_dialog)    run_dialog();  
 }
}

void
enable_signal()
{
g_signal_connect (G_OBJECT (awsedit), "focus_in_event",
                      G_CALLBACK (awsedit_get_focus),
                      NULL); 
g_signal_connect (G_OBJECT (awsedit), "focus_out_event",
                   G_CALLBACK (awsedit_lost_focus),
                    NULL);
g_signal_connect (G_OBJECT (awsedit), "window_state_event",
                   G_CALLBACK (awsedit_iconified),
                    NULL);
}

void
disable_signal()
{
 g_signal_handlers_disconnect_by_func(awsedit,awsedit_get_focus,NULL);
 g_signal_handlers_disconnect_by_func(awsedit,awsedit_lost_focus,NULL);
 g_signal_handlers_disconnect_by_func(awsedit,awsedit_iconified,NULL);
}


gboolean
on_textview_button_release_event       (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
  cursor_position();
  return FALSE;
}


gboolean
on_textview_key_release_event          (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
  cursor_position();
  return FALSE;
}
/*  evaluate cursor position */

void
cursor_position()
{
GtkTextIter curspos_iter;
gint ligne, col;
gchar coord[20];
if (!tabindex && !hexadata)
{
  gtk_text_buffer_get_iter_at_mark(buffer,&curspos_iter, gtk_text_buffer_get_insert(buffer));
  ligne=gtk_text_iter_get_line(&curspos_iter)+1;
  col=gtk_text_iter_get_line_offset(&curspos_iter)+1;
  if (convert)
  sprintf(coord,"conversion %3d:%4d",ligne,col);
  else
  sprintf(coord,"%3d:%4d",ligne,col);
  gtk_statusbar_pop(GTK_STATUSBAR(statusbar),converid);
  gtk_statusbar_push(GTK_STATUSBAR(statusbar),converid,coord);
 }
}

void
on_extraire_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
if(i>0)
  gtk_widget_set_sensitive(enregistrer,TRUE);
else
  gtk_widget_set_sensitive(enregistrer,FALSE);
if (i>0 && !fmod((*blk[fi])[bi].length,lrecl) && !tabindex && !hexadata)
    gtk_widget_set_sensitive(conversion,TRUE);
else
    gtk_widget_set_sensitive(conversion,FALSE);
if(convert)
 {
   gtk_widget_set_sensitive(type,TRUE);
   gtk_widget_set_sensitive(description,TRUE);
 }
 else
 {
  gtk_widget_set_sensitive(type,FALSE);
  gtk_widget_set_sensitive(description,FALSE);
 }
}

void
on_options_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
if(convert)
  {
   gtk_widget_set_sensitive(ebcdic_menu,FALSE);
   gtk_widget_set_sensitive(ascii_menu,FALSE);
  }
  else
  {
   gtk_widget_set_sensitive(ebcdic_menu,TRUE);
   gtk_widget_set_sensitive(ascii_menu,TRUE);
  }
}

void
on_blocks_radio_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
range_block=TRUE;
gtk_spin_button_set_value(GTK_SPIN_BUTTON(start_spin),block_start+1);
gtk_spin_button_set_value(GTK_SPIN_BUTTON(end_spin),block_end+1);
}


void
on_files_radio_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
range_block=FALSE;
gtk_spin_button_set_value(GTK_SPIN_BUTTON(start_spin),file_start+1);
gtk_spin_button_set_value(GTK_SPIN_BUTTON(end_spin),file_end+1);
}

void
on_end_spin_changed                    (GtkEditable     *editable,
                                        gpointer         user_data)
{
if(range_block==TRUE)
block_end=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(end_spin))-1;
else
file_end=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(end_spin))-1;
}


void
on_start_spin_changed                  (GtkEditable     *editable,
                                        gpointer         user_data)
{
if(range_block==TRUE)
block_start=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(start_spin))-1;
else
file_start=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(start_spin))-1;
}

void
on_dos_radio_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
unx=FALSE;
}


void
on_unix_radio_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
unx=TRUE;
}
