/*
 * Copyright (C) 2001,2002 Red Hat, Inc.
 *
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef vte_h_included
#define vte_h_included

#ident "$Id$"

#include <sys/types.h>
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <X11/Xlib.h>
#include <glib.h>
#include <pango/pango.h>
#include <gtk/gtk.h>
#include "termcap.h"
#include "trie.h"

G_BEGIN_DECLS

/* The terminal widget itself. */
typedef struct _VteTerminal {
	/*< public >*/

	/* Widget implementation stuffs. */
	GtkWidget widget;
	GtkAdjustment *adjustment;	/* Scrolling adjustment. */

	/* Metric and sizing data. */
	long char_width, char_height;	/* dimensions of character cells */
	long char_ascent, char_descent; /* important font metrics */
	long row_count, column_count;	/* dimensions of the window */

	/* Titles. */
	char *window_title, *icon_title;

	/*< private >*/
	struct _VteTerminalPrivate *pvt;
} VteTerminal;

/* The widget's class structure. */
typedef struct _VteTerminalClass {
	/*< public > */
	/* Inherited parent class. */
	GtkWidgetClass parent_class;

	/*< private > */
	/* Signals we might emit. */
	guint eof_signal;
	guint char_size_changed_signal;
	guint window_title_changed_signal;
	guint icon_title_changed_signal;
	guint selection_changed_signal;
	guint contents_changed_signal;
	guint cursor_moved_signal;

	guint deiconify_window_signal;
	guint iconify_window_signal;
	guint raise_window_signal;
	guint lower_window_signal;
	guint refresh_window_signal;
	guint restore_window_signal;
	guint maximize_window_signal;
	guint resize_window_signal;
	guint move_window_signal;
} VteTerminalClass;

/* A snapshot of the screen contents. */
typedef struct _VteTerminalSnapshot {
	struct {
		int x, y;			/* Location of the cursor. */
	} cursor;
	int rows, columns;			/* Size of the screen[shot]. */
	gboolean cursor_visible;
	struct VteTerminalSnapshotCell {
		gunichar c;			/* The character itself. */
		struct {
			/* Colors of this character. */
			GdkColor foreground, background;
			/* Is it underlined? */
			gboolean underline;
			/* Is it a graphic character? */
			gboolean alternate;
		} attributes;
	} **contents;
} VteTerminalSnapshot;

/* Values for "what should happen when the user hits backspace/delete".  Use
 * AUTO unless the user can cause them to be overridden. */
typedef enum {
	VTE_ERASE_AUTO,
	VTE_ERASE_ASCII_BACKSPACE,
	VTE_ERASE_ASCII_DELETE,
	VTE_ERASE_DELETE_SEQUENCE,
} VteTerminalEraseBinding;

/* The structure we return as the supplemental attributes for strings. */
struct vte_char_attributes {
	long row, column;
	GdkColor fore, back;
	gboolean underline, alternate;
};

/* The widget's type. */
GtkType vte_terminal_get_type(void);

#define VTE_TYPE_TERMINAL		(vte_terminal_get_type())
#define VTE_TERMINAL(obj)		(GTK_CHECK_CAST((obj),\
							VTE_TYPE_TERMINAL,\
							VteTerminal))
#define VTE_TERMINAL_CLASS(klass)	GTK_CHECK_CLASS_CAST((klass),\
							     VTE_TYPE_TERMINAL,\
							     VteTerminalClass)
#define VTE_IS_TERMINAL(obj)		GTK_CHECK_TYPE((obj),\
						       VTE_TYPE_TERMINAL)
#define VTE_IS_TERMINAL_CLASS(klass)	GTK_CHECK_CLASS_TYPE((klass),\
							     VTE_TYPE_TERMINAL)
#define VTE_TERMINAL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), VTE_TYPE_TERMINAL, VteTerminalClass))

GtkWidget *vte_terminal_new(void);
pid_t vte_terminal_fork_command(VteTerminal *terminal,
			        const char *command,
			        const char **argv);

/* Send data to the terminal to display, or to the terminal's forked command
 * to handle in some way.  If it's 'cat', they should be the same. */
void vte_terminal_feed(VteTerminal *terminal, const char *data, size_t length);
void vte_terminal_feed_child(VteTerminal *terminal,
			     const char *data, size_t length);

/* Copy currently-selected text to the clipboard, or from the clipboard to
 * the terminal. */
void vte_terminal_copy_clipboard(VteTerminal *terminal);
void vte_terminal_paste_clipboard(VteTerminal *terminal);

void vte_terminal_set_size(VteTerminal *terminal, long columns, long rows);

/* Set various one-off settings. */
void vte_terminal_set_audible_bell(VteTerminal *terminal, gboolean is_audible);
void vte_terminal_set_scroll_on_output(VteTerminal *terminal, gboolean scroll);
void vte_terminal_set_scroll_on_keystroke(VteTerminal *terminal,
					  gboolean scroll);

/* Set the color scheme. */
void vte_terminal_set_colors(VteTerminal *terminal,
			     const GdkColor *foreground,
			     const GdkColor *background,
			     const GdkColor *palette,
			     size_t palette_size);
void vte_terminal_set_default_colors(VteTerminal *terminal);

/* Background effects. */
void vte_terminal_set_background_image(VteTerminal *terminal, GdkPixbuf *image);
void vte_terminal_set_background_image_file(VteTerminal *terminal,
					    const char *path);
void vte_terminal_set_background_saturation(VteTerminal *terminal,
					    double saturation);
void vte_terminal_set_background_transparent(VteTerminal *terminal,
					     gboolean transparent);

/* Set whether or not the cursor blinks. */
void vte_terminal_set_cursor_blinks(VteTerminal *terminal, gboolean blink);

/* Set the number of scrollback lines, above or at an internal minimum. */
void vte_terminal_set_scrollback_lines(VteTerminal *terminal, long lines);

/* Append the input method menu items to a given shell. */
void vte_terminal_im_append_menuitems(VteTerminal *terminal,
				      GtkMenuShell *menushell);

/* Set or retrieve the current font. */
void vte_terminal_set_font(VteTerminal *terminal,
                           const PangoFontDescription *font_desc);
void vte_terminal_set_font_from_string(VteTerminal *terminal, const char *name);
const PangoFontDescription *vte_terminal_get_font(VteTerminal *terminal);
gboolean vte_terminal_get_using_xft(VteTerminal *terminal);

/* Check if the terminal is the current selection owner. */
gboolean vte_terminal_get_has_selection(VteTerminal *terminal);

/* Set the list of word chars, optionally using hyphens to specify ranges
 * (to get a hyphen, place it first), and check if a character is in the
 * range. */
void vte_terminal_set_word_chars(VteTerminal *terminal, const char *spec);
gboolean vte_terminal_is_word_char(VteTerminal *terminal, gunichar c);

/* Set what happens when the user strikes backspace or delete. */
void vte_terminal_set_backspace_binding(VteTerminal *terminal,
					VteTerminalEraseBinding binding);
void vte_terminal_set_delete_binding(VteTerminal *terminal,
				     VteTerminalEraseBinding binding);

/* Manipulate the autohide setting. */
void vte_terminal_set_mouse_autohide(VteTerminal *terminal, gboolean setting);
gboolean vte_terminal_get_mouse_autohide(VteTerminal *terminal);

/* Reset the terminal, optionally clearing the tab stops and line history. */
void vte_terminal_reset(VteTerminal *terminal, gboolean full,
			gboolean clear_history);

/* Read the contents of the terminal, using a callback function to determine
 * if a particular location on the screen (0-based) is interesting enough to
 * include.  Each byte in the returned string will have a corresponding
 * struct vte_char_attributes in the passed GArray, if the array was not NULL.
 * Note that it will have one entry per byte, not per character, so indexes
 * should match up exactly. */
char *vte_terminal_get_text(VteTerminal *terminal,
			    gboolean(*is_selected)(VteTerminal * terminal,
						    long column, long row),
			    GArray *attributes);

/* Display string matching:  clear all matching expressions. */
void vte_terminal_match_clear_all(VteTerminal *terminal);

/* Add a matching expression, returning the tag the widget assigns to that
 * expression. */
int vte_terminal_match_add(VteTerminal *terminal, const char *match);

/* Check if a given cell on the screen contains part of a matched string.  If
 * it does, return the string, and store the match tag in the optional tag
 * argument. */
char *vte_terminal_match_check(VteTerminal *terminal, long column, long row,
			       int *tag);

/* Set the emulation type.  Most of the time you won't need this. */
void vte_terminal_set_emulation(VteTerminal *terminal, const char *emulation);

G_END_DECLS

#endif
