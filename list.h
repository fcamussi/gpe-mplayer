#ifndef LIST_H_
#define LIST_H_

#include <gtk/gtk.h>

#define ROW_COLOR "Black"
#define SEL_ROW_COLOR "Blue"



enum Columns
{
	NAME_COLUMN,
	PATH_COLUMN,
	COLOR_COLUMN,
	N_COLUMNS
};




GtkWidget *List_Create();
void List_Add_File(gchar *name, gchar* path, gboolean invert);
gboolean List_Remove_File();
gboolean List_Is_Empty();
void List_Select_First();
void List_Select_This(GtkTreePath *path);
gboolean List_Select_Previous();
gboolean List_Select_Next();
void List_Get_Current(GString *file);



#endif /*LIST_H_*/
