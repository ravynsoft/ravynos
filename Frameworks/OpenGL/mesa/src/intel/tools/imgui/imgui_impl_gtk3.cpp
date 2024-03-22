// ImGui GLFW binding with OpenGL3 + shaders
// In this binding, ImTextureID is used to store an OpenGL 'GLuint' texture identifier. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#include <stdio.h>

#include "imgui/imgui.h"
#include "imgui_impl_gtk3.h"

#include <gtk/gtk.h>

#define ARRAY_SIZE(arg) (sizeof(arg) / sizeof((arg)[0]))

#define EVENT_MASK \
    ((GdkEventMask)                  \
    (GDK_STRUCTURE_MASK |            \
     GDK_FOCUS_CHANGE_MASK |	     \
     GDK_EXPOSURE_MASK |             \
     GDK_PROPERTY_CHANGE_MASK |	     \
     GDK_ENTER_NOTIFY_MASK |	     \
     GDK_LEAVE_NOTIFY_MASK |	     \
     GDK_KEY_PRESS_MASK |            \
     GDK_KEY_RELEASE_MASK |	     \
     GDK_BUTTON_PRESS_MASK |	     \
     GDK_BUTTON_RELEASE_MASK |	     \
     GDK_POINTER_MOTION_MASK |       \
     GDK_SMOOTH_SCROLL_MASK |        \
     GDK_SCROLL_MASK))

// Data
static GtkWidget*                      g_GtkGlArea = NULL;
static guint64                         g_Time = 0;
static bool                            g_MousePressed[5] = { false, false, false, false, false };
static ImVec2                          g_MousePosition = ImVec2(-1, -1);
static float                           g_MouseWheel = 0.0f;
static int                             g_NumRedraws = 0;
static guint                           g_RedrawTimeout = 0;

static const char* ImGui_ImplGtk3_GetClipboardText(void* user_data)
{
    static char *last_clipboard = NULL;

    g_clear_pointer(&last_clipboard, g_free);
    last_clipboard = gtk_clipboard_wait_for_text(GTK_CLIPBOARD(user_data));
    return last_clipboard;
}

static void ImGui_ImplGtk3_SetClipboardText(void* user_data, const char* text)
{
    gtk_clipboard_set_text(GTK_CLIPBOARD(user_data), text, -1);
}

void   ImGui_ImplGtk3_HandleEvent(GdkEvent *event)
{
    ImGuiIO& io = ImGui::GetIO();

    GdkEventType type = gdk_event_get_event_type(event);
    switch (type)
    {
    case GDK_MOTION_NOTIFY:
    {
        gdouble x = 0.0f, y = 0.0f;
        if (gdk_event_get_coords(event, &x, &y))
            g_MousePosition = ImVec2(x, y);
        break;
    }
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
    {
        guint button = 0;
        if (gdk_event_get_button(event, &button) && button > 0 && button <= 5)
        {
            if (type == GDK_BUTTON_PRESS)
                g_MousePressed[button - 1] = true;
        }
        break;
    }
    case GDK_SCROLL:
    {
        gdouble x, y;
        if (gdk_event_get_scroll_deltas(event, &x, &y))
            g_MouseWheel = -y;
        break;
    }
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
    {
        GdkEventKey *e = (GdkEventKey *) event;

        static const struct
        {
            enum ImGuiKey_ imgui;
            guint gdk;
        } gdk_key_to_imgui_key[] =
              {
                  { ImGuiKey_Tab, GDK_KEY_Tab },
                  { ImGuiKey_Tab, GDK_KEY_ISO_Left_Tab },
                  { ImGuiKey_LeftArrow, GDK_KEY_Left },
                  { ImGuiKey_RightArrow, GDK_KEY_Right },
                  { ImGuiKey_UpArrow, GDK_KEY_Up },
                  { ImGuiKey_DownArrow, GDK_KEY_Down },
                  { ImGuiKey_PageUp, GDK_KEY_Page_Up },
                  { ImGuiKey_PageDown, GDK_KEY_Page_Down },
                  { ImGuiKey_Home, GDK_KEY_Home },
                  { ImGuiKey_End, GDK_KEY_End },
                  { ImGuiKey_Delete, GDK_KEY_Delete },
                  { ImGuiKey_Backspace, GDK_KEY_BackSpace },
                  { ImGuiKey_Enter, GDK_KEY_Return },
                  { ImGuiKey_Escape, GDK_KEY_Escape },
                  { ImGuiKey_A, GDK_KEY_a },
                  { ImGuiKey_C, GDK_KEY_c },
                  { ImGuiKey_V, GDK_KEY_v },
                  { ImGuiKey_X, GDK_KEY_x },
                  { ImGuiKey_Y, GDK_KEY_y },
                  { ImGuiKey_Z, GDK_KEY_z },
              };
        for (unsigned i = 0; i < ARRAY_SIZE(gdk_key_to_imgui_key); i++)
        {
            if (e->keyval == gdk_key_to_imgui_key[i].gdk)
                io.KeysDown[gdk_key_to_imgui_key[i].imgui] = type == GDK_KEY_PRESS;
        }
        gunichar c = gdk_keyval_to_unicode(e->keyval);
        if (g_unichar_isprint(c) && ImGuiKey_COUNT + c < ARRAY_SIZE(io.KeysDown))
            io.KeysDown[ImGuiKey_COUNT + c] = type == GDK_KEY_PRESS;

        if (type == GDK_KEY_PRESS && e->string)
            io.AddInputCharactersUTF8(e->string);

        struct {
            bool *var;
            GdkModifierType modifier;
            guint keyvals[3];
        } mods[] = {
            { &io.KeyCtrl, GDK_CONTROL_MASK,
              { GDK_KEY_Control_L, GDK_KEY_Control_R, 0 }, },
            { &io.KeyShift, GDK_SHIFT_MASK,
              { GDK_KEY_Shift_L, GDK_KEY_Shift_R, 0 }, },
            { &io.KeyAlt, GDK_MOD1_MASK,
              { GDK_KEY_Alt_L, GDK_KEY_Alt_R, 0 }, },
            { &io.KeySuper, GDK_SUPER_MASK,
              { GDK_KEY_Super_L, GDK_KEY_Super_R, 0 }, }
        };
        for (unsigned i = 0; i < ARRAY_SIZE(mods); i++)
        {
            *mods[i].var = (mods[i].modifier & e->state);

            bool match = false;
            for (int j = 0; mods[i].keyvals[j] != 0; j++)
                if (e->keyval == mods[i].keyvals[j])
                    match = true;

            if (match)
                *mods[i].var = type == GDK_KEY_PRESS;
        }
        break;
    }
    default:
        break;
    }

    // We trigger 2 subsequent redraws for each event because of the
    // way some ImGui widgets work. For example a Popup menu will only
    // appear a frame after a click happened.
    g_NumRedraws = 2;

    gtk_widget_queue_draw(g_GtkGlArea);
}

static gboolean handle_gdk_event(GtkWidget *widget, GdkEvent *event, void *data)
{
    ImGui_ImplGtk3_HandleEvent(event);
    return true;
}

bool ImGui_ImplGtk3_Init(GtkWidget* gl_area, bool install_callbacks)
{
    g_clear_pointer(&g_GtkGlArea, g_object_unref);

    g_GtkGlArea = GTK_WIDGET(g_object_ref(gl_area));
    gtk_widget_realize(g_GtkGlArea);
    gtk_widget_set_can_focus(g_GtkGlArea, true);
    gtk_widget_grab_focus(g_GtkGlArea);

    if (install_callbacks) {
        gtk_widget_add_events(g_GtkGlArea, EVENT_MASK);
        g_signal_connect(g_GtkGlArea, "event", G_CALLBACK(handle_gdk_event), NULL);
    }

    ImGuiIO& io = ImGui::GetIO();
    for (int i = 0; i < ImGuiKey_COUNT; i++)
    {
        io.KeyMap[i] = i;
    }

    io.SetClipboardTextFn = ImGui_ImplGtk3_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplGtk3_GetClipboardText;
    io.ClipboardUserData = gtk_widget_get_clipboard(g_GtkGlArea,
                                                    GDK_SELECTION_CLIPBOARD);

    return true;
}

void ImGui_ImplGtk3_Shutdown()
{
    g_clear_pointer(&g_GtkGlArea, g_object_unref);
    if (g_RedrawTimeout)
        g_source_remove(g_RedrawTimeout);
}

static gboolean timeout_callback(gpointer data)
{
    gtk_widget_queue_draw(g_GtkGlArea);
    g_RedrawTimeout = 0;
    return false;
}

static void kick_timeout_redraw(float timeout)
{
    if (g_RedrawTimeout)
        return;
    g_RedrawTimeout = g_timeout_add(timeout * 1000, timeout_callback, NULL);
}

void ImGui_ImplGtk3_NewFrame()
{
    bool next_redraw = false;
    if (g_NumRedraws > 0)
    {
        gtk_widget_queue_draw(g_GtkGlArea);
        g_NumRedraws--;
        next_redraw = true;
    }

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    io.DisplaySize = ImVec2((float)gtk_widget_get_allocated_width(g_GtkGlArea),
                            (float)gtk_widget_get_allocated_height(g_GtkGlArea));
    int scale_factor = gtk_widget_get_scale_factor(g_GtkGlArea);
    io.DisplayFramebufferScale = ImVec2(scale_factor, scale_factor);

    // Setup time step
    guint64 current_time =  g_get_monotonic_time();
    io.DeltaTime = g_Time > 0 ? ((float)(current_time - g_Time) / 1000000) : (float)(1.0f/60.0f);
    g_Time = current_time;

    // Setup inputs
    if (gtk_widget_has_focus(g_GtkGlArea))
    {
        io.MousePos = g_MousePosition;   // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
    }
    else
    {
        io.MousePos = ImVec2(-1,-1);
    }

    GdkWindow *window = gtk_widget_get_window(g_GtkGlArea);
    GdkDevice *pointer = gdk_device_manager_get_client_pointer(gdk_display_get_device_manager(gdk_display_get_default()));
    GdkModifierType modifiers;
    gdk_device_get_state(pointer, window, NULL, &modifiers);

    for (int i = 0; i < 3; i++)
    {
        io.MouseDown[i] = g_MousePressed[i] || (modifiers & (GDK_BUTTON1_MASK << i)) != 0;
        g_MousePressed[i] = false;
    }

    io.MouseWheel = g_MouseWheel;
    g_MouseWheel = 0.0f;

    // Hide OS mouse cursor if ImGui is drawing it
    GdkDisplay *display = gdk_window_get_display(window);
    GdkCursor *cursor =
      gdk_cursor_new_from_name(display, io.MouseDrawCursor ? "none" : "default");
    gdk_window_set_cursor(window, cursor);
    g_object_unref(cursor);

    if (!next_redraw && io.WantTextInput)
        kick_timeout_redraw(0.2);
}
