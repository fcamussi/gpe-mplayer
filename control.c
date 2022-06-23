
#include "control.h"
#include "output.h"
#include "window.h"
#include <glib.h>
#include <string.h>
#include <signal.h>



static GString *filename = NULL;
static GPid pid;
static guint timerid;
static GIOChannel *input, *output, *error;
static gint volumelevel;
static State status = STOPPED;




static void Write_To_Input(char *string)
{
	gsize written;
	g_io_channel_write_chars(input, string, strlen(string), &written, NULL);
	g_io_channel_flush(input, NULL);
}



static void Request_Time()
{
	Write_To_Input("get_time_pos\n");
}



static void Request_Lenght()
{
	Write_To_Input("get_time_length\n");
}



static void Request_Video_Codec()
{
	Write_To_Input("get_video_codec\n");
}



static void Request_Size()
{
	Write_To_Input("get_property width\n");	
	Write_To_Input("get_property height\n");
}



static gboolean Timer(gpointer data)
{
	Request_Time();
	return TRUE;
}



static void Create_Channels(gint inputfd, gint outputfd, gint errorfd)
{
	input = g_io_channel_unix_new(inputfd);
	output = g_io_channel_unix_new(outputfd);
	error = g_io_channel_unix_new(errorfd);
	g_io_channel_set_flags(input, G_IO_FLAG_NONBLOCK, NULL);
	g_io_channel_set_encoding(output, NULL, NULL);
}



static void Shutdown_Channels()
{
	g_io_channel_shutdown(input, TRUE, NULL);
	g_io_channel_shutdown(output, TRUE, NULL);
	g_io_channel_shutdown(error, TRUE, NULL);
}



static void Ended_Playing(GPid pid, gint ret, gpointer data)
{
	if (status == PLAYING && ret == 0) // El fichero se reprodujo hasta el final
	{
		Output_Stop_Checking();
		g_source_remove(timerid);
		Shutdown_Channels();
		status = STOPPED;
		Window_Play_Next();
	}
	g_spawn_close_pid(pid);
	Window_Change_To_List();
}



static void Refresh_Volume_Level()
{ 
	GString *tmp = g_string_new(NULL);
	g_string_printf(tmp, "volume %d 1\n", volumelevel * 10);
	Write_To_Input(tmp->str);
	g_string_free(tmp, TRUE);
}



void Control_Initialize()
{
	filename = g_string_new(NULL);
}



void Control_Terminate()
{
	g_string_free(filename, TRUE);
}



void Control_Load_File(gchar *pathname)
{
	g_string_assign(filename, pathname);
}



void Control_Play()
{
	gint inputfd, outputfd, errorfd;
	GString *line = g_string_new(NULL); 

	g_string_printf(line, "/usr/bin/mplayer -slave -quiet -mixer-channel vol -ac mad "
					"-osdlevel 0 -wid %d -vo x11 -zoom", Window_Get_MPlayer_Window_ID());
	gchar **argv = g_strsplit(line->str, " ", 0);
	gint n = g_strv_length(argv);
	gchar **tmp = g_memdup(argv, n * sizeof(gchar *)); 
	tmp = g_realloc(tmp, (n + 2) * sizeof(gchar *));
	tmp[n] = filename->str;
	tmp[n + 1] = NULL;

	g_spawn_async_with_pipes(NULL, tmp, NULL, G_SPAWN_LEAVE_DESCRIPTORS_OPEN |
							 G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &pid, &inputfd,
							 &outputfd, &errorfd, NULL);
	Create_Channels(inputfd, outputfd, errorfd);
	Refresh_Volume_Level();
	Output_Start_Checking(output);
	Request_Size();
	Request_Video_Codec();
	Request_Lenght();
	Request_Time();
	timerid = g_timeout_add(REQUEST_TIME, Timer, NULL);
	g_child_watch_add(pid, Ended_Playing, NULL);
	status = PLAYING;
	
	g_free(tmp);
	g_strfreev(argv);
	g_string_free(line, TRUE);
}



void Control_Pause(gboolean pause)
{
	if (pause)
	{
		g_source_remove(timerid);
		Write_To_Input("pause\n");
		status = PAUSED;
	}
	else
	{
		Write_To_Input("pause\n");
		Refresh_Volume_Level();
		timerid = g_timeout_add(REQUEST_TIME, Timer, NULL);
		status = PLAYING;
	}
}



void Control_Stop()
{
	Output_Stop_Checking();
	g_source_remove(timerid);
	kill(pid, SIGTERM);
	Shutdown_Channels();
	status = STOPPED;
}



State Control_Get_State()
{
	return status;
}



void Control_Set_Volume_Level(gint level)
{
	volumelevel = level;
 	if (status == PLAYING)
	{
		Refresh_Volume_Level();
	}
}



void Control_Set_Position(gint position)
{
	GString *tmp = g_string_new(NULL);
	g_string_printf(tmp, "seek %d 2\n", position); 

	Write_To_Input(tmp->str);
	if (status == PAUSED)
	{
		Refresh_Volume_Level();
		timerid = g_timeout_add(REQUEST_TIME, Timer, NULL);
	}
	Request_Time();
	status = PLAYING;

	g_string_free(tmp, TRUE);
}



void Control_Set_Full_Screen()
{
	Write_To_Input("vo_fullscreen 1\n");
}

