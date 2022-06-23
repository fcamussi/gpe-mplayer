#ifndef WINDOW_H_
#define WINDOW_H_

#include <gtk/gtk.h>


#define WINDOW_HEIGHT 320
#define WINDOW_WIDTH 240
#define WINDOW_TITLE "GPE-MPlayer"



enum Buttons {
	PLAY_BUTTON,
	PAUSE_BUTTON,
	STOP_BUTTON,
	PREVIOUS_BUTTON,
	NEXT_BUTTON,
	ADD_BUTTON,
	REMOVE_BUTTON,
	N_BUTTONS
};




void Window_Create();
void Window_Set_Lenght(gint lenght);
void Window_Set_Position(gint pos);
void Window_Set_Time(gint hours, gint minutes, gint seconds);
gboolean Window_Position_In_Use();
void Window_Play_Next();
guint32 Window_Get_MPlayer_Window_ID();
void Window_Change_To_List();
void Window_Change_To_MPlayer_Window();


#endif /*WINDOW_H_*/
