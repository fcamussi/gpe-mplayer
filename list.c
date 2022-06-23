
#include "list.h"
#include <string.h>



static GtkWidget *treeview;
static GtkListStore *store; // model1
//static GtkTreeModel *model; // model2
static GtkCellRenderer *renderer;
static GtkTreeViewColumn *column;
static GtkTreeSelection *selection;
static GtkTreeIter current;




GtkWidget *List_Create()
{
	store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	//model = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(store));
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("File name", renderer, "text", NAME_COLUMN,
													  "foreground", COLOR_COLUMN, NULL);
	//gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), PATH_COLUMN, GTK_SORT_DESCENDING);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	//gtk_tree_view_set_enable_search(GTK_TREE_VIEW(treeview), FALSE);
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	return treeview;
}



void List_Add_File(gchar *name, gchar* path, gboolean invert)
{
	GtkTreeIter iter;
	if (invert)
	{
		gtk_list_store_prepend(store, &iter);
	}
	else
	{
		gtk_list_store_append(store, &iter);
	}
	gchar *nameutf8 = g_locale_to_utf8(name, strlen(name), NULL, NULL, NULL);
	gtk_list_store_set(store, &iter, NAME_COLUMN, nameutf8, PATH_COLUMN, path,
					   COLOR_COLUMN, ROW_COLOR, -1);
	g_free(nameutf8);
}



gboolean List_Remove_File()
{
	// deberia separar las cosas...
	GtkTreeIter iter;
	gboolean current_deleted = FALSE;
	gtk_tree_selection_get_selected(selection, NULL, &iter);
	GtkTreePath *selectedpath = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
	GtkTreePath *currentpath = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &current);
	if (gtk_tree_path_compare(currentpath, selectedpath) == 0)
	{
		current_deleted = TRUE;
	}
	gtk_list_store_remove(store, &iter);
	gtk_tree_path_free(selectedpath);
	gtk_tree_path_free(currentpath);
	return current_deleted;
}



gboolean List_Is_Empty()
{
	GtkTreeIter tmp = current;
	return ! gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &tmp);
}



void List_Select_First()
{
	// quiza el problema de select first se solucione permitiendo que no halla seleccion
	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &current);
	gtk_list_store_set(store, &current, COLOR_COLUMN, SEL_ROW_COLOR, -1);
	GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &current);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(treeview), path, column, TRUE, 0.5, 0.5);
	gtk_tree_path_free(path);
}



void List_Select_This(GtkTreePath *path)
{
	gtk_list_store_set(store, &current, COLOR_COLUMN, ROW_COLOR, -1);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &current, path);
	gtk_list_store_set(store, &current, COLOR_COLUMN, SEL_ROW_COLOR, -1);
}



gboolean List_Select_Previous()
{
	GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &current);
	gboolean result;
	if ((result = gtk_tree_path_prev(path)))
	{
		gtk_list_store_set(store, &current, COLOR_COLUMN, ROW_COLOR, -1);
		gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &current, path);
		gtk_list_store_set(store, &current, COLOR_COLUMN, SEL_ROW_COLOR, -1);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(treeview), path, column, TRUE, 0.5, 0.5);
	}
	gtk_tree_path_free(path);
	return result;
}



gboolean List_Select_Next()
{
	GtkTreeIter tmp = current;
	if (gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &current))
	{
		gtk_list_store_set(store, &tmp, COLOR_COLUMN, ROW_COLOR, -1);
		gtk_list_store_set(store, &current, COLOR_COLUMN, SEL_ROW_COLOR, -1);
		GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &current);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(treeview), path, column, TRUE, 0.5, 0.5);
		gtk_tree_path_free(path);
		return TRUE;
	}
	else
	{
		current = tmp;
		return FALSE;
	}
}



void List_Get_Current(GString *file)
{
	gchar *tmp;
	gtk_tree_model_get(GTK_TREE_MODEL(store), &current, PATH_COLUMN, &tmp, -1);
	g_string_assign(file, tmp);
	g_free(tmp);
}
	

