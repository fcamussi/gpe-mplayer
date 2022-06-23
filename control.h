#ifndef CONTROL_H_
#define CONTROL_H_

#include <glib.h>


#define REQUEST_TIME 1000



enum State
{
	PLAYING,
	PAUSED,
	STOPPED
};

typedef guint State;



void Control_Initialize();
void Control_Terminate();
void Control_Load_File(gchar *pathname);
void Control_Play();
void Control_Pause(gboolean pause);
void Control_Stop();
State Control_Get_State();
void Control_Set_Volume_Level(gint level);
void Control_Set_Position(gint position);
void Control_Set_Full_Screen();



#endif /*CONTROL_H_*/
