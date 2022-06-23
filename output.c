
#include "output.h"
#include "window.h"
#include <string.h>



static GString *line;
static guint lenght, width, height;
static guint watchid;




static gboolean Is(gchar *str)
{
	return strncmp(line->str, str, strlen(str)) == 0;
}



static gint Get_Value_As_Integer() // pensar en usar en float por el final correcto!
{
	gchar **pair = g_strsplit(line->str, "=", 2);
	int value = (gint)g_ascii_strtod(pair[1], NULL);
	g_strfreev(pair);
	return value;
}



static gboolean Get_Value_As_Boolean()
{
	gchar **pair = g_strsplit(line->str, "=", 2);
	gboolean value = strcmp(pair[1], "''\n") ? TRUE: FALSE;
	g_strfreev(pair);
	return value;
}



static gboolean Check_Output(GIOChannel *output, GIOCondition condition, gpointer data)
{
	g_io_channel_read_line_string(output, line, NULL, NULL);

	if (Is("ANS_LENGTH"))
	{
		lenght = Get_Value_As_Integer();
		Window_Set_Lenght(lenght);
	}

	if (Is("ANS_TIME_POSITION"))
	{
		gint seconds = Get_Value_As_Integer();
		if (! Window_Position_In_Use()) Window_Set_Position(seconds);
		gint minutes = seconds / 60;
		gint hours = minutes / 60;
		seconds = seconds % 60;
		minutes = minutes % 60;
		Window_Set_Time(hours, minutes, seconds);
	}
	
	if (Is("ANS_width"))
	{
		width = Get_Value_As_Integer();
	}
	
	if (Is("ANS_height"))
	{
		height = Get_Value_As_Integer();
	}
	
	if (Is("ANS_VIDEO_CODEC"))
	{
		if (Get_Value_As_Boolean() == TRUE) // Hay video
		{
			Window_Change_To_MPlayer_Window();
		}
	}
	
	return TRUE;
}



void Output_Start_Checking(GIOChannel *output)
{
	watchid = g_io_add_watch(output, G_IO_IN, Check_Output, NULL);
	line = g_string_new(NULL);
}



void Output_Stop_Checking()
{
	g_source_remove(watchid);
	g_string_free(line, TRUE);
}


