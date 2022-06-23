
#include "window.h"
#include "control.h"
#include "list.h"
#include <gdk/gdkx.h>



static GtkWidget *window;
static GtkWidget *table;
static GtkWidget *mpwindow;
static GtkWidget *scrolled;
static GtkWidget *list;
static GtkWidget *timepos;
static GtkWidget *position;
static GtkWidget *buttonsbar;
static GtkToolItem *buttons[N_BUTTONS];
static GtkWidget *volume;
static gboolean position_in_use = FALSE;




static void Set_Position_Enabled(gboolean state)
{
	gtk_widget_set_sensitive(position, state);
	if (! state)
	{
		Window_Set_Position(0);
		Window_Set_Time(0, 0, 0);
	}
}



static void Load_Config()
{
	// Simplemente seteo lo que deberia cargarse desde un fichero de configuracion 
	gtk_range_set_value(GTK_RANGE(volume), 10);
}
	


static void Window_Show(GtkWidget *widget, gpointer data)
{
	Control_Initialize();
	Set_Position_Enabled(FALSE);
	Window_Change_To_List();
	Load_Config();
}



static gboolean Window_Delete(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	if (Control_Get_State() != STOPPED)
	{
		Control_Stop();
	}
	Control_Terminate();
	gtk_main_quit();
	return TRUE;
}



static void Load_Current()
{
	GString *file = g_string_new(NULL);
	List_Get_Current(file);
	Control_Load_File(file->str);
	g_string_free(file, TRUE);
}



static void Button_Play_Clicked(GtkToolButton *toolbutton, gpointer data)
{
	if (! List_Is_Empty())
	{
		switch(Control_Get_State())
		{
			case STOPPED:
				Load_Current();
				Set_Position_Enabled(TRUE);
				Control_Play();
				break;

			case PLAYING:
				Control_Stop();
				Control_Play();
				break;

			case PAUSED:
				Control_Pause(FALSE);
				break;
		}
	}
}



static void Button_Pause_Clicked(GtkToolButton *toolbutton, gpointer data)
{
	switch(Control_Get_State())
	{
		case PLAYING:
			Control_Pause(TRUE);
			break;

		case PAUSED:
			Control_Pause(FALSE);
			break;
	}
}



static void Button_Stop_Clicked(GtkToolButton *toolbutton, gpointer data)
{
	if (Control_Get_State() != STOPPED)
	{
		Control_Stop();
		Set_Position_Enabled(FALSE);
	}	
}



static void Button_Previous_Clicked(GtkToolButton *toolbutton, gpointer data)
{
	if (! List_Is_Empty())
	{
		if (List_Select_Previous())
		{
			if (Control_Get_State() != STOPPED)
			{
				Load_Current();
				Control_Stop();
				Control_Play();
			}
		}
	}
}



static void Button_Next_Clicked(GtkToolButton *toolbutton, gpointer data)
{
	if (! List_Is_Empty())
	{
		if (List_Select_Next())
		{
			if (Control_Get_State() != STOPPED)
			{
				Load_Current();
				Control_Stop();
				Control_Play();
			}
		}
	}
}



static void List_Clicked(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data)
{
	List_Select_This(path);
	Load_Current();
	if (Control_Get_State() != STOPPED)
	{
		Control_Stop();
	}
	else
	{
		Set_Position_Enabled(TRUE);
	}
	Control_Play();
}



static void Position_Changed(GtkRange *range, gpointer data)
{
	Control_Set_Position(gtk_range_get_value(range));
}



static void Volume_Changed(GtkRange *range, gpointer data)
{
	Control_Set_Volume_Level(gtk_range_get_value(range));
}



static gboolean Position_Pressed(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	position_in_use = TRUE;
	return FALSE;
}



static gboolean Position_Released(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	position_in_use = FALSE;
	return FALSE;
}



static void Add_File(gchar *file)
{
	if (g_file_test(file, G_FILE_TEST_IS_REGULAR))
	{
		gchar *base = g_path_get_basename(file);
		List_Add_File(base, file, FALSE);
		g_free(base);
	}
	if (g_file_test(file, G_FILE_TEST_IS_DIR))
	{
		// Evita los posibles ciclos infinitos
		if (g_file_test(file, G_FILE_TEST_IS_SYMLINK))
		{
			return;
		}
		GDir *dir = g_dir_open(file, 0, NULL);
		const gchar *name;
		GString *tmp = g_string_new(NULL);
		while ((name = g_dir_read_name(dir)))
		{
			g_string_printf(tmp, "%s/%s", file, name);
			Add_File(tmp->str);
		}
		g_string_free(tmp, TRUE);
		g_dir_close(dir);
	}
}



static void Button_Add_Clicked(GtkToolButton *toolbutton, gpointer data)
{
	GtkWidget *filesel = gtk_file_selection_new("Select files");
	gtk_file_selection_set_select_multiple(GTK_FILE_SELECTION(filesel), TRUE);
	gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(filesel));
	gtk_window_set_modal(GTK_WINDOW(filesel), TRUE);
	if (gtk_dialog_run(GTK_DIALOG(filesel)) == GTK_RESPONSE_OK)
	{
		gchar **files = gtk_file_selection_get_selections(GTK_FILE_SELECTION(filesel));
		gchar **p;
		for (p = files ; *p; p++)
		{
			Add_File(*p);
		}
		g_strfreev(files);
		if (! List_Is_Empty())
		{
			List_Select_First();
		}
	}
	gtk_widget_destroy(filesel);
}



static void Button_Remove_Clicked(GtkToolButton *toolbutton, gpointer data)
{
	if (! List_Is_Empty())
	{
		if (List_Remove_File())
		{
		  	if (Control_Get_State() != STOPPED)
			{
				Control_Stop();
				Set_Position_Enabled(FALSE);
			}
			List_Select_First();
		}
	}
}



static gboolean Window_Key_Press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	//g_print("%d\n", event->hardware_keycode);
	switch(event->hardware_keycode)
	{
		case 75:
			Button_Play_Clicked(NULL, NULL);
		break;
		case 76:
			Button_Pause_Clicked(NULL, NULL);
		break;
		case 77:
			// No anda porque el windows manager caputura la pulsación
		break;
		case 96:
			Button_Stop_Clicked(NULL, NULL);
		break;
		case 111:
		{
			gint vol = gtk_range_get_value(GTK_RANGE(volume));
			if (vol < 10)
			{
				gtk_range_set_value(GTK_RANGE(volume), vol + 1);
			}
		}
		break; // Mirar el libro de C para ver como lo pone
		case 116:
		{
			gint vol = gtk_range_get_value(GTK_RANGE(volume));
			if (vol > 0)
			{
				gtk_range_set_value(GTK_RANGE(volume), vol - 1);
			}
		}
		break;
		case 113:
			Button_Previous_Clicked(NULL, NULL);
		break;
		case 114:
			Button_Next_Clicked(NULL, NULL);
		break;
	}
	return FALSE;
}



static gboolean Abort_Key_Press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	return TRUE;
}

	

void Window_Create()
{
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), WINDOW_TITLE);
	gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_WIDTH, WINDOW_HEIGHT);
	g_signal_connect(G_OBJECT(window), "show", G_CALLBACK(Window_Show), NULL);
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(Window_Delete), NULL);
	g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(Window_Key_Press), NULL);

	table = gtk_table_new(6, 2, FALSE);
	gtk_container_add(GTK_CONTAINER(window), table);
	
	list = List_Create();
	g_signal_connect(G_OBJECT(list), "row-activated", G_CALLBACK(List_Clicked), NULL);
	g_signal_connect(G_OBJECT(list), "key-press-event", G_CALLBACK(Abort_Key_Press), NULL);
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), 
								   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scrolled), list);
	gtk_table_attach(GTK_TABLE(table), scrolled, 0, 2, 0, 1,
					 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);


	// Aqui viene lo bueno joven!
	mpwindow = gtk_socket_new();
	gtk_table_attach(GTK_TABLE(table), mpwindow, 0, 2, 1, 2,
					 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);


	timepos = gtk_label_new(NULL);
	Window_Set_Time(0, 0, 0);
	gtk_table_attach(GTK_TABLE(table), timepos, 0, 1, 2, 3,
					 GTK_EXPAND | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
	
	position = gtk_hscale_new_with_range(0, 100, 10);
	gtk_range_set_increments(GTK_RANGE(position), 10, 10);
	g_signal_connect(G_OBJECT(position), "value-changed",
					 G_CALLBACK(Position_Changed), NULL);
	g_signal_connect(G_OBJECT(position), "button-press-event",
					 G_CALLBACK(Position_Pressed), NULL);
	g_signal_connect(G_OBJECT(position), "button-release-event",
					 G_CALLBACK(Position_Released), NULL);
	g_signal_connect(G_OBJECT(position), "key-press-event", G_CALLBACK(Abort_Key_Press), NULL);
	gtk_scale_set_draw_value(GTK_SCALE(position), FALSE);
	gtk_range_set_update_policy(GTK_RANGE(position), GTK_UPDATE_DISCONTINUOUS);
	gtk_table_attach(GTK_TABLE(table), position, 0, 1, 3, 4,
					 GTK_EXPAND | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);

	buttonsbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(buttonsbar), GTK_TOOLBAR_ICONS);
	gtk_table_attach(GTK_TABLE(table), buttonsbar, 0, 1, 4, 5,
					 GTK_EXPAND | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
	
	buttons[PLAY_BUTTON] = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
	gtk_toolbar_insert(GTK_TOOLBAR(buttonsbar),
					   GTK_TOOL_ITEM(buttons[PLAY_BUTTON]), -1);
	g_signal_connect(G_OBJECT(buttons[PLAY_BUTTON]), "clicked",
					 G_CALLBACK(Button_Play_Clicked), NULL);
					   
	buttons[PAUSE_BUTTON] = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PAUSE);
	gtk_toolbar_insert(GTK_TOOLBAR(buttonsbar),
					   GTK_TOOL_ITEM(buttons[PAUSE_BUTTON]), -1);
	g_signal_connect(G_OBJECT(buttons[PAUSE_BUTTON]), "clicked",
					 G_CALLBACK(Button_Pause_Clicked), NULL);

	buttons[STOP_BUTTON] = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
	gtk_toolbar_insert(GTK_TOOLBAR(buttonsbar),
					   GTK_TOOL_ITEM(buttons[STOP_BUTTON]), -1);
	g_signal_connect(G_OBJECT(buttons[STOP_BUTTON]), "clicked",
					 G_CALLBACK(Button_Stop_Clicked), NULL);

	buttons[PREVIOUS_BUTTON] = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PREVIOUS);
	gtk_toolbar_insert(GTK_TOOLBAR(buttonsbar),
					   GTK_TOOL_ITEM(buttons[PREVIOUS_BUTTON]), -1);
	g_signal_connect(G_OBJECT(buttons[PREVIOUS_BUTTON]), "clicked",
					 G_CALLBACK(Button_Previous_Clicked), NULL);

	buttons[NEXT_BUTTON] = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_NEXT);
	gtk_toolbar_insert(GTK_TOOLBAR(buttonsbar),
					   GTK_TOOL_ITEM(buttons[NEXT_BUTTON]), -1);
	g_signal_connect(G_OBJECT(buttons[NEXT_BUTTON]), "clicked",
					 G_CALLBACK(Button_Next_Clicked), NULL);
					   
	gtk_toolbar_insert(GTK_TOOLBAR(buttonsbar),
					   GTK_TOOL_ITEM(gtk_separator_tool_item_new()), -1);

	buttons[ADD_BUTTON] = gtk_tool_button_new_from_stock(GTK_STOCK_ADD);
	gtk_toolbar_insert(GTK_TOOLBAR(buttonsbar),
					   GTK_TOOL_ITEM(buttons[ADD_BUTTON]), -1);
	g_signal_connect(G_OBJECT(buttons[ADD_BUTTON]), "clicked",
					 G_CALLBACK(Button_Add_Clicked), NULL);

	buttons[REMOVE_BUTTON] = gtk_tool_button_new_from_stock(GTK_STOCK_REMOVE);
	gtk_toolbar_insert(GTK_TOOLBAR(buttonsbar),
					   GTK_TOOL_ITEM(buttons[REMOVE_BUTTON]), -1);
	g_signal_connect(G_OBJECT(buttons[REMOVE_BUTTON]), "clicked",
					 G_CALLBACK(Button_Remove_Clicked), NULL);

	volume = gtk_vscale_new_with_range(0, 10, 1);
	gtk_range_set_inverted(GTK_RANGE(volume), TRUE);
	gtk_range_set_increments(GTK_RANGE(volume), 1, 1);
	g_signal_connect(G_OBJECT(volume), "value-changed",
					 G_CALLBACK(Volume_Changed), NULL);
	g_signal_connect(G_OBJECT(volume), "key-press-event", G_CALLBACK(Abort_Key_Press), NULL);
	gtk_scale_set_draw_value(GTK_SCALE(volume), FALSE);
	gtk_table_attach(GTK_TABLE(table), volume, 1, 2, 2, 5,
					 GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);

	gtk_widget_show_all(window);
}



void Window_Set_Lenght(gint lenght)
{
	g_signal_handlers_block_by_func(G_OBJECT(position), G_CALLBACK(Position_Changed), NULL);
	gtk_range_set_range(GTK_RANGE(position), 0, lenght);
	g_signal_handlers_unblock_by_func(G_OBJECT(position), G_CALLBACK(Position_Changed), NULL);
}



void Window_Set_Position(gint pos)
{
	g_signal_handlers_block_by_func(G_OBJECT(position), G_CALLBACK(Position_Changed), NULL);
	gtk_range_set_value(GTK_RANGE(position), pos);
	g_signal_handlers_unblock_by_func(G_OBJECT(position), G_CALLBACK(Position_Changed), NULL);
}



void Window_Set_Time(gint hours, gint minutes, gint seconds)
{
	char *tmp = g_markup_printf_escaped("<span font_desc='10'>%02d:%02d:%02d</span>", 
										 hours, minutes, seconds);
	gtk_label_set_markup(GTK_LABEL(timepos), tmp);
	g_free(tmp);
}



gboolean Window_Position_In_Use()
{
	return position_in_use;
}



void Window_Play_Next()
{
	if (List_Select_Next())
	{
		Load_Current();
		Control_Play();
	}
	else
	{
		Set_Position_Enabled(FALSE);
	}		
}



guint32 Window_Get_MPlayer_Window_ID()
{
	return GDK_WINDOW_XWINDOW(mpwindow->window);
}



void Window_Change_To_List()
{
	gtk_widget_hide(mpwindow);
	gtk_widget_show(scrolled);
}


		
void Window_Change_To_MPlayer_Window()
{
	gtk_widget_hide(scrolled);
	gtk_widget_show(mpwindow);
}

