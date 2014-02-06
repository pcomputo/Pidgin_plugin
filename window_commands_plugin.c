/*
 * Window commands - Helps you carry out various commands on the conversation window.
 * Copyright (C) 2013 Pooja Ahuja <ahuja.pooja22@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Include glib.h for various types */
#include <glib.h>

#ifndef G_GNUC_NULL_TERMINATED
# if __GNUC__ >= 4
#  define G_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
# else
#  define G_GNUC_NULL_TERMINATED
# endif
#endif

#ifndef PURPLE_PLUGINS
# define PURPLE_PLUGINS
#endif

#include "internal.h"
#include "version.h"
#include "plugin.h"

#include "cmds.h"
#include "debug.h"

#include "gtkconv.h"
#include "gtkwebview.h"

#include<stdlib.h>
#include<string.h>
#include<gtk/gtk.h>

#define PLUGIN_ID "core-windowcommand"

#define PLUGIN_AUTHOR "Pooja Ahuja <ahuja.pooja22@gmail.com>"


static PurplePlugin *command_example = NULL;

/**
 * Used to hold a handle to the commands we register. Holding this handle
 * allows us to unregister the commands at a later time. 
 */
static PurpleCmdId size_command_id, alloc_command_id, fullscreen_command_id, exit_fullscreen_command_id, backgroundcolor_command_id;

/**
 * cmd: /size
 * The callback function for the /size window command. This command pops up
 * a notification with the current height and width of the Pidgin conversation window. 
 */

static PurpleCmdRet 
size_cb(PurpleConversation *conv, const gchar *cmd, gchar **args, gchar **error, void *data)
{
    char *text;
    PidginConversation *gtkconv;
    GtkAllocation* alloc;

    gtkconv = PIDGIN_CONVERSATION(conv);
    
    alloc = g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(gtkconv->win->window, alloc);

    purple_debug_misc(PLUGIN_ID, "size_cb called\n");

    text = g_strdup_printf("Width is: %d and height is: %d", alloc->width, alloc->height);
    purple_notify_info(command_example, "Window size", text, NULL, NULL);
    g_free(text);

    return PURPLE_CMD_RET_OK;

}

/**
 * cmd: /allocate width:height
 * This command resizes the conversation window
 * as per user input.
 * eg: /allocate 600:500 
 */

static PurpleCmdRet 
alloc_cb(PurpleConversation *conv, const gchar *cmd, gchar **args, gchar **error, void *data)
{
    PidginConversation *gtkconv;
    
    gchar **section = g_strsplit(args[0], ":", 2);
    gchar *width = section[0];
    gchar *height = section[1];

    gtkconv = PIDGIN_CONVERSATION(conv);
    
    purple_debug_misc(PLUGIN_ID, "alloc_cb called\n");
    purple_debug_misc(PLUGIN_ID,"the args: %s and %s", width, height);
    
    if (gdk_window_get_state(gtk_widget_get_window(gtkconv->win->window)) &
        GDK_WINDOW_STATE_MAXIMIZED)
		      purple_notify_warning(command_example, "Window size", 
                                  "The widow has reached it's max limit!", NULL, NULL); 
	
    if (atoi(width) > 0 && atoi(height) > 0)
            gtk_window_resize(GTK_WINDOW(gtkconv->win->window), atoi(width), atoi(height));
    else if(atoi(width) == 0)
            purple_notify_error(command_example, "Window size", 
                                "The width cannot be 0!", NULL, NULL);
    else if(atoi(height) == 0)
            purple_notify_error(command_example, "Window size", 
                                "The height cannot be 0!", NULL, NULL);
    else if(atoi(width) < 0)
            purple_notify_error(command_example, "Window size", 
                                "The width cannot be negative!", NULL, NULL);
    else if(atoi(height) < 0)
            purple_notify_error(command_example, "Window size", 
                                "The height cannot be negative!", NULL, NULL);
    
    g_strfreev (section);

  return PURPLE_CMD_RET_OK;

}

/**
 * cmd: /fullscreen
 * This command asks to place the window in fullscreen state. 
 */

static PurpleCmdRet 
fullscreen_cb(PurpleConversation *conv, const gchar *cmd, gchar **args, gchar **error, void *data)
{
    PidginConversation *gtkconv;
    gtkconv = PIDGIN_CONVERSATION(conv);
    
    gtk_window_fullscreen(GTK_WINDOW(gtkconv->win->window));

   return PURPLE_CMD_RET_OK;
}

/**
 * cmd: /exit fullscreen
 * Asks to toggle off the fullscreen state for conversation window. 
 */

static PurpleCmdRet 
exit_fullscreen_cb(PurpleConversation *conv, const gchar *cmd, gchar **args, gchar **error, void *data)
{
    PidginConversation *gtkconv;
    gtkconv = PIDGIN_CONVERSATION(conv);
    
    gtk_window_unfullscreen(GTK_WINDOW(gtkconv->win->window));

   return PURPLE_CMD_RET_OK;
}

/**
 * cmd: /backgroundcolor or /backgroundcolor <hexadecimal notation>
 * eg: /backgroundcolor #565656
 * Asks to toggle off the fullscreen state for conversation window. 
 */

static PurpleCmdRet 
backgroundcolor_cb(PurpleConversation *conv, const gchar *cmd, gchar **args, gchar **error, void *data)
{
    PidginConversation *gtkconv;
    GtkColorSelection *colorsel;
    GtkResponseType result;
    gchar *color_string;

	 gtkconv = PIDGIN_CONVERSATION(conv);

    if(!args[0]){
     GtkWidget *dialog = gtk_color_selection_dialog_new("Pick a background color.");
     result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    
     if (result == GTK_RESPONSE_OK)
     {
        GdkColor color;
 
        colorsel = gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(dialog));
        gtk_color_selection_get_current_color(colorsel,&color);
        color_string = g_strdup_printf("document.body.style.background = '#%02X%02X%02X'", 
			 										color.red / 256, color.green / 256, color.blue /256);
        purple_debug_info("window_commands_plugin", "executing script: %s\n", color_string);
        gtk_webview_safe_execute_script(GTK_WEBVIEW(gtkconv->webview), color_string);
        g_free(color_string);
      } 
     gtk_widget_destroy(dialog);
     }
     else{
        gchar *color_input; 
        color_input = args[0];

        color_string = g_strdup_printf("document.body.style.background = '%s'", 
			 										 color_input);
        purple_debug_info("window_commands_plugin", "executing script: %s\n", color_string);
        gtk_webview_safe_execute_script(GTK_WEBVIEW(gtkconv->webview), color_string);
     }
   return PURPLE_CMD_RET_OK;
}

static gboolean
plugin_load(PurplePlugin *plugin)
{
    gchar *size_help = NULL;
    gchar *alloc_help = NULL;
    gchar *fullscreen_help = NULL;
    gchar *exit_fullscreen_help = NULL;
    gchar *backgroundcolor_help = NULL;  

  	command_example = plugin;

  	size_help = "notify &lt;notify word here&gt;: Notifies the conversation window size";
  	// Registers a command to allow a user to enter /size some word and have
  	// This command runs with high priority,
  	// and can be used in both group and standard chat messages 
  	size_command_id = purple_cmd_register
    ("size",                      /* command name */ 
     "w",                         /* command argument format */
     PURPLE_CMD_P_HIGH,           /* command priority flags */  
     PURPLE_CMD_FLAG_IM | 
     PURPLE_CMD_FLAG_CHAT |
     PURPLE_CMD_FLAG_ALLOW_WRONG_ARGS,        /* command usage flags */
     PLUGIN_ID,                   /* Plugin ID */
     size_cb,                     /* Callback function */
     size_help,                   /* Help message */
     NULL                         /* Any special user-defined data */
     );

  	alloc_help = "notify &lt;notify word here&gt;:  Let's you change the size of the conversation window.";
  	// Registers a command to allow a user to enter /allocate width:height and have
  	// that word notify the dimensions of the Pidgin window to them.  
  	alloc_command_id = purple_cmd_register
    ("allocate",                  /* command name */ 
     "w",                         /* command argument format */
     PURPLE_CMD_P_HIGH,           /* command priority flags */  
     PURPLE_CMD_FLAG_IM | 
     PURPLE_CMD_FLAG_CHAT,        /* command usage flags */
     PLUGIN_ID,                   /* Plugin ID */
     alloc_cb,                    /* Callback function */
     alloc_help,                  /* Help message */
     NULL                         /* Any special user-defined data */
     );

  	fullscreen_help = "notify &lt;notify word here&gt;:  Let's you change the size of the conversation window to fullscreen.";
  	// Registers a command to allow a user to enter /fullscreen  
  	fullscreen_command_id = purple_cmd_register
    ("fullscreen",                  /* command name */ 
     "w",                         /* command argument format */
     PURPLE_CMD_P_HIGH,           /* command priority flags */  
     PURPLE_CMD_FLAG_IM | 
     PURPLE_CMD_FLAG_CHAT |
     PURPLE_CMD_FLAG_ALLOW_WRONG_ARGS,        /* command usage flags */
     PLUGIN_ID,                   /* Plugin ID */
     fullscreen_cb,                    /* Callback function */
     fullscreen_help,                  /* Help message */
     NULL                         /* Any special user-defined data */
     );
  
  	exit_fullscreen_help = "notify &lt;notify word here&gt;:  Let's you exit fullscreen.";
  	// Registers a command to allow a user to enter /exit fullscreen 
  	exit_fullscreen_command_id = purple_cmd_register
    ("exit",                  /* command name */ 
     "w",                         /* command argument format */
     PURPLE_CMD_P_HIGH,           /* command priority flags */  
     PURPLE_CMD_FLAG_IM | 
     PURPLE_CMD_FLAG_CHAT,        /* command usage flags */
     PLUGIN_ID,                   /* Plugin ID */
     exit_fullscreen_cb,                    /* Callback function */
     exit_fullscreen_help,                  /* Help message */
     NULL                         /* Any special user-defined data */
     );

  	backgroundcolor_help = "notify &lt;notify word here&gt;:  Let's you change the conversation window background color.";
  	// Registers a command to allow a user to enter /backgroundcolor. This command runs with high priority,
  	// and can be used in both group and standard chat messages 
  	backgroundcolor_command_id = purple_cmd_register
    ("backgroundcolor",                  /* command name */ 
     "w",                         /* command argument format */
     PURPLE_CMD_P_HIGH,           /* command priority flags */  
     PURPLE_CMD_FLAG_IM | 
     PURPLE_CMD_FLAG_CHAT |
     PURPLE_CMD_FLAG_ALLOW_WRONG_ARGS,        /* command usage flags */
     PLUGIN_ID,                   /* Plugin ID */
     backgroundcolor_cb,                    /* Callback function */
     backgroundcolor_help,                  /* Help message */
     NULL                         /* Any special user-defined data */
     );
 

  /* Just return true to tell libpurple to finish loading. */
  return TRUE;
}

static gboolean
plugin_unload(PurplePlugin *plugin)
{
  	purple_cmd_unregister(size_command_id);
  	purple_cmd_unregister(alloc_command_id);
  	/* Just return true to tell libpurple to finish unloading. */
  return TRUE;
}

static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,        /* magic number */
    PURPLE_MAJOR_VERSION,       /* purple major */
    PURPLE_MINOR_VERSION,       /* purple minor */
    PURPLE_PLUGIN_STANDARD,     /* plugin type */
    NULL,                       /* UI requirement */
    0,                          /* flags */
    NULL,                       /* dependencies */
    PURPLE_PRIORITY_DEFAULT,    /* priority */
    PLUGIN_ID,                  /* id */
    "Window Commands",      /* name */
    "1.0",                      /* version */
    "Helps you carry out a number of commands on the conversation window.",      /* summary */
    "The commands should be entered in the entry area.",                           /* description */
    PLUGIN_AUTHOR,             /* author */
    "ahujapooja.com",           /* homepage */

    plugin_load,                /* load */
    plugin_unload,              /* unload */
    NULL,                       /* destroy */

    NULL,                       /* ui info */
    NULL,                       /* extra info */
    NULL,                       /* prefs info */
    NULL,                       /* actions */
    NULL,                       /* reserved */
    NULL,                       /* reserved */
    NULL,                       /* reserved */
    NULL                        /* reserved */
};

static void
init_plugin(PurplePlugin *plugin)
{
}

PURPLE_INIT_PLUGIN(commandexample, init_plugin, info)
