#include <gtk/gtk.h>
/*--------------------------------------------------------------------*/
/* Structure definition for AWSTAPE block header                      */
/*--------------------------------------------------------------------*/
#include <sys/fcntl.h>
#include <sys/stat.h>
extern GtkWidget *treeview;
extern GtkWidget *textview;
extern GtkWidget *progressbar;
extern GtkWidget *index_check;
extern GtkWidget *decimal_index_radio;
extern GtkWidget *hexa_index_radio;
extern GtkWidget *hexa_data_check;
extern GtkWidget *lrecl_spin;
extern GtkWidget *end_spin;
extern GtkWidget *start_spin;
extern GtkWidget *files_radio;
extern GtkWidget *blocks_radio;
extern GtkWidget *dos_radio;
extern GtkWidget *unix_radio;
extern GtkWidget *statusbar;
extern GtkWidget *conversion;
extern GtkWidget *enregistrer;
extern GtkWidget *config;
extern GtkWidget *ouvrir;
extern GtkWidget *packed;
extern GtkWidget *zoned;
extern GtkWidget *supress;
extern GtkWidget *annul;
extern GtkWidget *ascii_menu;
extern GtkWidget *ebcdic_menu;
extern GtkWidget *cobol_menu;
extern GtkWidget *dbf_menu;
extern GtkWidget *type;
extern GtkWidget *description;
extern GtkWidget *charger;
extern GtkWidget *sauver;
extern GtkWidget *rechercher;
extern GtkWidget *rechercher_entry;
extern GtkWidget *rechercher_dialog;
extern GtkWidget *casse_check;
extern GtkWidget *pos_check;
extern GtkWidget *hexa_check;
extern GtkWidget *rechercher_dialog_okbutton;
extern GtkWidget *awsedit;
struct flags {
        guint8 unused   : 4,
              segmented : 1,
              endrec    : 1,
              tapemark  : 1,
              newrec    : 1;
             };
             
typedef struct _AWSTAPE_BLKHDR {
        guint16   curblkl;             /* Length of this block          */
        guint16   prvblkl;             /* Length of previous block      */
        struct flags  flags1;              /* Flags byte 1                  */
        guint8 flags2;              /* Flags byte 2                  */
           } AWSTAPE_BLKHDR;
/* Definitions for flags1 */

enum
{
   BLOCK_COLUMN,
   FILE_COLUMN,
   N_COLUMNS
};

enum
{  PACKED,
   ZONED,
   SUPRESS,
   NORMAL
};

typedef struct _BLKDESC {
         guint loc;
         guint length;
         } BLKDESC;

typedef struct _FIELDESC {
        gint  start;
        gint  end;
        gint  type;
        } FIELDESC;
        

void 
on_ouvrir_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);



void
tree_selection_changed (GtkTreeSelection *selection, gpointer data);

                                          
void
on_ascii_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_ebcdic_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);


void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_textview_populate_popup             (GtkTextView     *textview,
                                        GtkMenu         *menu,
                                        gpointer         user_data);


void
on_config_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);




void
on_extract_dialog_cancelbutton_clicked (GtkButton       *button,
                                        gpointer         user_data);

void
on_extract_dialog_okbutton_clicked     (GtkButton       *button,
                                        gpointer         user_data);

void
on_config_dialog_cancelbutton_clicked  (GtkButton       *button,
                                        gpointer         user_data);

void
on_config_dialog_okbutton_clicked      (GtkButton       *button,
                                        gpointer         user_data);




void
on_packed_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_supress_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_zoned_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
                                        
void
on_conversion_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
                                        
void
on_enregistrer_activate                (GtkMenuItem       *button,
                                        gpointer         user_data);

void
on_charger_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_sauver_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);


void
on_index_check_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);


void
on_range_check_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);


void
color_field(guint p);
void
process_field(guint type);

void
on_annul_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);


void
mapcar(gchar *rec,guint8 car,gint q);


void
on_cobol_menu_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_dbf_menu_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);


void
on_aboutdialog_response                (GtkDialog       *dialog,
                                        gint             response_id,
                                        gpointer         user_data);

void
on_description_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rechercher_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_rechercher_dialog_cancelbutton_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_rechercher_dialog_okbutton_clicked  (GtkButton       *button,
                                        gpointer         user_data);



void
on_rechercher_entry_changed            (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_rechercher_dialog_close             (GtkDialog       *dialog,
                                        gpointer         user_data);

gboolean
on_rechercher_dialog_delete_event      (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_rechercher_dialog_destroy_event     (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);
void
change_selection();
void
awsedit_get_focus (GtkWidget *window, gpointer  data);
void
awsedit_lost_focus (GtkWidget *window, gpointer  data);
void
awsedit_iconified (GtkWidget *window, GdkEventWindowState *etat, gpointer  data);
void
run_dialog();
void
enable_signal();
void
disable_signal();


gboolean
on_textview_button_release_event       (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_textview_key_release_event          (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

void
cursor_position();

void
on_extraire_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_options_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
gboolean
on_awsedit_configure_event                  (GtkWidget  *widget,
                                       GdkEventConfigure *event,
                                       gpointer user_data);
void
on_blocks_radio_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_files_radio_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_dos_radio_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_unix_radio_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);
void
on_end_spin_changed                    (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_start_spin_changed                  (GtkEditable     *editable,
                                        gpointer         user_data);
