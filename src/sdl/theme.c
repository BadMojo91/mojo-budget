#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "theme.h"
#include "../core/config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

#ifdef _WIN32
  #include <windows.h>
  #include <direct.h>
  #define MKDIR(p) _mkdir(p)
  #define PATH_SEP "\\"
#else
  #include <dirent.h>
  #define MKDIR(p) mkdir(p, 0755)
  #define PATH_SEP "/"
#endif

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */

static int FindColorIndex(const char *name)
{
  for (int i = 0; i < ImGuiCol_COUNT; i++)
  {
    if (strcmp(igGetStyleColorName(i), name) == 0)
      return i;
  }
  return -1;
}

/* Sanitizes a theme display name into a safe filename component.
   Replaces any character that isn't alphanumeric, space, '-', or '_'
   with '_'. The result is written into buf (max sz bytes). */
static void SanitizeFilename(const char *name, char *buf, size_t sz)
{
  size_t i;
  for (i = 0; i < sz - 1 && name[i] != '\0'; i++)
  {
    char ch = name[i];
    if (isalnum((unsigned char)ch) || ch == ' ' || ch == '-' || ch == '_')
      buf[i] = ch;
    else
      buf[i] = '_';
  }
  buf[i] = '\0';
}

/* -------------------------------------------------------------------------
 * Premade theme implementations
 * ---------------------------------------------------------------------- */

static void ApplyDark(void)    { igStyleColorsDark(igGetStyle()); }
static void ApplyLight(void)   { igStyleColorsLight(igGetStyle()); }
static void ApplyClassic(void) { igStyleColorsClassic(igGetStyle()); }

static void ApplyCatppuccinMocha(void)
{
  ImGuiStyle *s = igGetStyle();
  ImVec4     *c = s->Colors;

  /* Catppuccin Mocha: https://github.com/catppuccin/catppuccin
     Base #1e1e2e  Mantle #181825  Crust #11111b
     Surface0 #313244  Surface1 #45475a  Surface2 #585b70
     Overlay0 #6c7086  Overlay1 #7f849c
     Text #cdd6f4  Subtext0 #a6adc8  Subtext1 #bac2de
     Blue #89b4fa  Sapphire #74c7ec  Teal #94e2d5
     Green #a6e3a1  Yellow #f9e2af  Mauve #cba6f7
     Red #f38ba8  Peach #fab387 */

  c[ImGuiCol_Text]                      = (ImVec4){0.804f, 0.839f, 0.957f, 1.000f};
  c[ImGuiCol_TextDisabled]              = (ImVec4){0.424f, 0.439f, 0.525f, 1.000f};
  c[ImGuiCol_WindowBg]                  = (ImVec4){0.118f, 0.118f, 0.180f, 1.000f};
  c[ImGuiCol_ChildBg]                   = (ImVec4){0.118f, 0.118f, 0.180f, 1.000f};
  c[ImGuiCol_PopupBg]                   = (ImVec4){0.094f, 0.094f, 0.145f, 0.960f};
  c[ImGuiCol_Border]                    = (ImVec4){0.192f, 0.196f, 0.267f, 1.000f};
  c[ImGuiCol_BorderShadow]              = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_FrameBg]                   = (ImVec4){0.192f, 0.196f, 0.267f, 1.000f};
  c[ImGuiCol_FrameBgHovered]            = (ImVec4){0.271f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_FrameBgActive]             = (ImVec4){0.345f, 0.357f, 0.439f, 1.000f};
  c[ImGuiCol_TitleBg]                   = (ImVec4){0.094f, 0.094f, 0.145f, 1.000f};
  c[ImGuiCol_TitleBgActive]             = (ImVec4){0.192f, 0.196f, 0.267f, 1.000f};
  c[ImGuiCol_TitleBgCollapsed]          = (ImVec4){0.118f, 0.118f, 0.180f, 0.750f};
  c[ImGuiCol_MenuBarBg]                 = (ImVec4){0.094f, 0.094f, 0.145f, 1.000f};
  c[ImGuiCol_ScrollbarBg]               = (ImVec4){0.094f, 0.094f, 0.145f, 1.000f};
  c[ImGuiCol_ScrollbarGrab]             = (ImVec4){0.192f, 0.196f, 0.267f, 1.000f};
  c[ImGuiCol_ScrollbarGrabHovered]      = (ImVec4){0.271f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_ScrollbarGrabActive]       = (ImVec4){0.345f, 0.357f, 0.439f, 1.000f};
  c[ImGuiCol_CheckMark]                 = (ImVec4){0.796f, 0.651f, 0.969f, 1.000f}; /* Mauve */
  c[ImGuiCol_SliderGrab]                = (ImVec4){0.537f, 0.706f, 0.980f, 1.000f}; /* Blue */
  c[ImGuiCol_SliderGrabActive]          = (ImVec4){0.455f, 0.780f, 0.925f, 1.000f}; /* Sapphire */
  c[ImGuiCol_Button]                    = (ImVec4){0.192f, 0.196f, 0.267f, 1.000f};
  c[ImGuiCol_ButtonHovered]             = (ImVec4){0.271f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_ButtonActive]              = (ImVec4){0.345f, 0.357f, 0.439f, 1.000f};
  c[ImGuiCol_Header]                    = (ImVec4){0.271f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_HeaderHovered]             = (ImVec4){0.345f, 0.357f, 0.439f, 1.000f};
  c[ImGuiCol_HeaderActive]              = (ImVec4){0.424f, 0.439f, 0.525f, 1.000f};
  c[ImGuiCol_Separator]                 = (ImVec4){0.192f, 0.196f, 0.267f, 1.000f};
  c[ImGuiCol_SeparatorHovered]          = (ImVec4){0.345f, 0.357f, 0.439f, 1.000f};
  c[ImGuiCol_SeparatorActive]           = (ImVec4){0.537f, 0.706f, 0.980f, 1.000f};
  c[ImGuiCol_ResizeGrip]                = (ImVec4){0.192f, 0.196f, 0.267f, 0.500f};
  c[ImGuiCol_ResizeGripHovered]         = (ImVec4){0.537f, 0.706f, 0.980f, 0.750f};
  c[ImGuiCol_ResizeGripActive]          = (ImVec4){0.455f, 0.780f, 0.925f, 1.000f};
  c[ImGuiCol_InputTextCursor]           = (ImVec4){0.804f, 0.839f, 0.957f, 1.000f};
  c[ImGuiCol_TabHovered]                = (ImVec4){0.345f, 0.357f, 0.439f, 1.000f};
  c[ImGuiCol_Tab]                       = (ImVec4){0.192f, 0.196f, 0.267f, 1.000f};
  c[ImGuiCol_TabSelected]               = (ImVec4){0.271f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_TabSelectedOverline]       = (ImVec4){0.537f, 0.706f, 0.980f, 1.000f};
  c[ImGuiCol_TabDimmed]                 = (ImVec4){0.118f, 0.118f, 0.180f, 1.000f};
  c[ImGuiCol_TabDimmedSelected]         = (ImVec4){0.192f, 0.196f, 0.267f, 1.000f};
  c[ImGuiCol_TabDimmedSelectedOverline] = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_PlotLines]                 = (ImVec4){0.580f, 0.886f, 0.835f, 1.000f}; /* Teal */
  c[ImGuiCol_PlotLinesHovered]          = (ImVec4){0.455f, 0.780f, 0.925f, 1.000f};
  c[ImGuiCol_PlotHistogram]             = (ImVec4){0.537f, 0.706f, 0.980f, 1.000f};
  c[ImGuiCol_PlotHistogramHovered]      = (ImVec4){0.455f, 0.780f, 0.925f, 1.000f};
  c[ImGuiCol_TableHeaderBg]             = (ImVec4){0.192f, 0.196f, 0.267f, 1.000f};
  c[ImGuiCol_TableBorderStrong]         = (ImVec4){0.345f, 0.357f, 0.439f, 1.000f};
  c[ImGuiCol_TableBorderLight]          = (ImVec4){0.192f, 0.196f, 0.267f, 1.000f};
  c[ImGuiCol_TableRowBg]                = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_TableRowBgAlt]             = (ImVec4){1.000f, 1.000f, 1.000f, 0.030f};
  c[ImGuiCol_TextLink]                  = (ImVec4){0.537f, 0.706f, 0.980f, 1.000f};
  c[ImGuiCol_TextSelectedBg]            = (ImVec4){0.271f, 0.278f, 0.353f, 0.750f};
  c[ImGuiCol_TreeLines]                 = (ImVec4){0.192f, 0.196f, 0.267f, 1.000f};
  c[ImGuiCol_DragDropTarget]            = (ImVec4){0.976f, 0.886f, 0.686f, 1.000f}; /* Yellow */
  c[ImGuiCol_DragDropTargetBg]          = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_UnsavedMarker]             = (ImVec4){0.976f, 0.886f, 0.686f, 1.000f};
  c[ImGuiCol_NavCursor]                 = (ImVec4){0.537f, 0.706f, 0.980f, 1.000f};
  c[ImGuiCol_NavWindowingHighlight]     = (ImVec4){1.000f, 1.000f, 1.000f, 0.700f};
  c[ImGuiCol_NavWindowingDimBg]         = (ImVec4){0.800f, 0.800f, 0.800f, 0.200f};
  c[ImGuiCol_ModalWindowDimBg]          = (ImVec4){0.118f, 0.118f, 0.180f, 0.700f};
}

static void ApplyNord(void)
{
  ImGuiStyle *s = igGetStyle();
  ImVec4     *c = s->Colors;

  /* Nord palette: https://www.nordtheme.com/
     Polar Night: #2E3440 #3B4252 #434C5E #4C566A
     Snow Storm:  #D8DEE9 #E5E9F0 #ECEFF4
     Frost:       #8FBCBB #88C0D0 #81A1C1 #5E81AC
     Aurora:      #BF616A #D08770 #EBCB8B #A3BE8C #B48EAD */

  c[ImGuiCol_Text]                      = (ImVec4){0.929f, 0.937f, 0.957f, 1.000f};
  c[ImGuiCol_TextDisabled]              = (ImVec4){0.298f, 0.337f, 0.416f, 1.000f};
  c[ImGuiCol_WindowBg]                  = (ImVec4){0.180f, 0.204f, 0.251f, 1.000f};
  c[ImGuiCol_ChildBg]                   = (ImVec4){0.180f, 0.204f, 0.251f, 1.000f};
  c[ImGuiCol_PopupBg]                   = (ImVec4){0.231f, 0.259f, 0.322f, 0.960f};
  c[ImGuiCol_Border]                    = (ImVec4){0.263f, 0.298f, 0.369f, 1.000f};
  c[ImGuiCol_BorderShadow]              = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_FrameBg]                   = (ImVec4){0.231f, 0.259f, 0.322f, 1.000f};
  c[ImGuiCol_FrameBgHovered]            = (ImVec4){0.263f, 0.298f, 0.369f, 1.000f};
  c[ImGuiCol_FrameBgActive]             = (ImVec4){0.298f, 0.337f, 0.416f, 1.000f};
  c[ImGuiCol_TitleBg]                   = (ImVec4){0.180f, 0.204f, 0.251f, 1.000f};
  c[ImGuiCol_TitleBgActive]             = (ImVec4){0.231f, 0.259f, 0.322f, 1.000f};
  c[ImGuiCol_TitleBgCollapsed]          = (ImVec4){0.180f, 0.204f, 0.251f, 0.750f};
  c[ImGuiCol_MenuBarBg]                 = (ImVec4){0.231f, 0.259f, 0.322f, 1.000f};
  c[ImGuiCol_ScrollbarBg]               = (ImVec4){0.180f, 0.204f, 0.251f, 1.000f};
  c[ImGuiCol_ScrollbarGrab]             = (ImVec4){0.263f, 0.298f, 0.369f, 1.000f};
  c[ImGuiCol_ScrollbarGrabHovered]      = (ImVec4){0.298f, 0.337f, 0.416f, 1.000f};
  c[ImGuiCol_ScrollbarGrabActive]       = (ImVec4){0.369f, 0.506f, 0.675f, 1.000f};
  c[ImGuiCol_CheckMark]                 = (ImVec4){0.533f, 0.753f, 0.816f, 1.000f};
  c[ImGuiCol_SliderGrab]                = (ImVec4){0.506f, 0.631f, 0.757f, 1.000f};
  c[ImGuiCol_SliderGrabActive]          = (ImVec4){0.369f, 0.506f, 0.675f, 1.000f};
  c[ImGuiCol_Button]                    = (ImVec4){0.231f, 0.259f, 0.322f, 1.000f};
  c[ImGuiCol_ButtonHovered]             = (ImVec4){0.263f, 0.298f, 0.369f, 1.000f};
  c[ImGuiCol_ButtonActive]              = (ImVec4){0.298f, 0.337f, 0.416f, 1.000f};
  c[ImGuiCol_Header]                    = (ImVec4){0.263f, 0.298f, 0.369f, 1.000f};
  c[ImGuiCol_HeaderHovered]             = (ImVec4){0.298f, 0.337f, 0.416f, 1.000f};
  c[ImGuiCol_HeaderActive]              = (ImVec4){0.369f, 0.506f, 0.675f, 1.000f};
  c[ImGuiCol_Separator]                 = (ImVec4){0.231f, 0.259f, 0.322f, 1.000f};
  c[ImGuiCol_SeparatorHovered]          = (ImVec4){0.298f, 0.337f, 0.416f, 1.000f};
  c[ImGuiCol_SeparatorActive]           = (ImVec4){0.369f, 0.506f, 0.675f, 1.000f};
  c[ImGuiCol_ResizeGrip]                = (ImVec4){0.231f, 0.259f, 0.322f, 0.500f};
  c[ImGuiCol_ResizeGripHovered]         = (ImVec4){0.533f, 0.753f, 0.816f, 0.750f};
  c[ImGuiCol_ResizeGripActive]          = (ImVec4){0.506f, 0.631f, 0.757f, 1.000f};
  c[ImGuiCol_InputTextCursor]           = (ImVec4){0.929f, 0.937f, 0.957f, 1.000f};
  c[ImGuiCol_TabHovered]                = (ImVec4){0.263f, 0.298f, 0.369f, 1.000f};
  c[ImGuiCol_Tab]                       = (ImVec4){0.231f, 0.259f, 0.322f, 1.000f};
  c[ImGuiCol_TabSelected]               = (ImVec4){0.263f, 0.298f, 0.369f, 1.000f};
  c[ImGuiCol_TabSelectedOverline]       = (ImVec4){0.533f, 0.753f, 0.816f, 1.000f};
  c[ImGuiCol_TabDimmed]                 = (ImVec4){0.180f, 0.204f, 0.251f, 1.000f};
  c[ImGuiCol_TabDimmedSelected]         = (ImVec4){0.231f, 0.259f, 0.322f, 1.000f};
  c[ImGuiCol_TabDimmedSelectedOverline] = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_PlotLines]                 = (ImVec4){0.561f, 0.737f, 0.733f, 1.000f};
  c[ImGuiCol_PlotLinesHovered]          = (ImVec4){0.533f, 0.753f, 0.816f, 1.000f};
  c[ImGuiCol_PlotHistogram]             = (ImVec4){0.922f, 0.796f, 0.545f, 1.000f};
  c[ImGuiCol_PlotHistogramHovered]      = (ImVec4){0.816f, 0.529f, 0.439f, 1.000f};
  c[ImGuiCol_TableHeaderBg]             = (ImVec4){0.231f, 0.259f, 0.322f, 1.000f};
  c[ImGuiCol_TableBorderStrong]         = (ImVec4){0.263f, 0.298f, 0.369f, 1.000f};
  c[ImGuiCol_TableBorderLight]          = (ImVec4){0.231f, 0.259f, 0.322f, 1.000f};
  c[ImGuiCol_TableRowBg]                = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_TableRowBgAlt]             = (ImVec4){1.000f, 1.000f, 1.000f, 0.030f};
  c[ImGuiCol_TextLink]                  = (ImVec4){0.533f, 0.753f, 0.816f, 1.000f};
  c[ImGuiCol_TextSelectedBg]            = (ImVec4){0.263f, 0.298f, 0.369f, 0.750f};
  c[ImGuiCol_TreeLines]                 = (ImVec4){0.231f, 0.259f, 0.322f, 1.000f};
  c[ImGuiCol_DragDropTarget]            = (ImVec4){0.922f, 0.796f, 0.545f, 1.000f};
  c[ImGuiCol_DragDropTargetBg]          = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_UnsavedMarker]             = (ImVec4){0.922f, 0.796f, 0.545f, 1.000f};
  c[ImGuiCol_NavCursor]                 = (ImVec4){0.506f, 0.631f, 0.757f, 1.000f};
  c[ImGuiCol_NavWindowingHighlight]     = (ImVec4){1.000f, 1.000f, 1.000f, 0.700f};
  c[ImGuiCol_NavWindowingDimBg]         = (ImVec4){0.800f, 0.800f, 0.800f, 0.200f};
  c[ImGuiCol_ModalWindowDimBg]          = (ImVec4){0.180f, 0.204f, 0.251f, 0.700f};
}

static void ApplyDracula(void)
{
  ImGuiStyle *s = igGetStyle();
  ImVec4     *c = s->Colors;

  /* Dracula palette: https://draculatheme.com/
     Background #282A36  Current Line #44475A  Foreground #F8F8F2
     Comment #6272A4  Cyan #8BE9FD  Green #50FA7B  Orange #FFB86C
     Pink #FF79C6  Purple #BD93F9  Red #FF5555  Yellow #F1FA8C */

  c[ImGuiCol_Text]                      = (ImVec4){0.973f, 0.973f, 0.949f, 1.000f};
  c[ImGuiCol_TextDisabled]              = (ImVec4){0.384f, 0.447f, 0.643f, 1.000f};
  c[ImGuiCol_WindowBg]                  = (ImVec4){0.157f, 0.165f, 0.212f, 1.000f};
  c[ImGuiCol_ChildBg]                   = (ImVec4){0.157f, 0.165f, 0.212f, 1.000f};
  c[ImGuiCol_PopupBg]                   = (ImVec4){0.157f, 0.165f, 0.212f, 0.960f};
  c[ImGuiCol_Border]                    = (ImVec4){0.267f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_BorderShadow]              = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_FrameBg]                   = (ImVec4){0.267f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_FrameBgHovered]            = (ImVec4){0.384f, 0.447f, 0.643f, 0.600f};
  c[ImGuiCol_FrameBgActive]             = (ImVec4){0.384f, 0.447f, 0.643f, 1.000f};
  c[ImGuiCol_TitleBg]                   = (ImVec4){0.157f, 0.165f, 0.212f, 1.000f};
  c[ImGuiCol_TitleBgActive]             = (ImVec4){0.267f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_TitleBgCollapsed]          = (ImVec4){0.157f, 0.165f, 0.212f, 0.750f};
  c[ImGuiCol_MenuBarBg]                 = (ImVec4){0.157f, 0.165f, 0.212f, 1.000f};
  c[ImGuiCol_ScrollbarBg]               = (ImVec4){0.157f, 0.165f, 0.212f, 1.000f};
  c[ImGuiCol_ScrollbarGrab]             = (ImVec4){0.267f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_ScrollbarGrabHovered]      = (ImVec4){0.384f, 0.447f, 0.643f, 1.000f};
  c[ImGuiCol_ScrollbarGrabActive]       = (ImVec4){0.741f, 0.576f, 0.976f, 1.000f};
  c[ImGuiCol_CheckMark]                 = (ImVec4){1.000f, 0.475f, 0.776f, 1.000f}; /* Pink */
  c[ImGuiCol_SliderGrab]                = (ImVec4){0.741f, 0.576f, 0.976f, 1.000f}; /* Purple */
  c[ImGuiCol_SliderGrabActive]          = (ImVec4){1.000f, 0.475f, 0.776f, 1.000f};
  c[ImGuiCol_Button]                    = (ImVec4){0.267f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_ButtonHovered]             = (ImVec4){0.384f, 0.447f, 0.643f, 1.000f};
  c[ImGuiCol_ButtonActive]              = (ImVec4){0.741f, 0.576f, 0.976f, 1.000f};
  c[ImGuiCol_Header]                    = (ImVec4){0.267f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_HeaderHovered]             = (ImVec4){0.384f, 0.447f, 0.643f, 1.000f};
  c[ImGuiCol_HeaderActive]              = (ImVec4){0.741f, 0.576f, 0.976f, 1.000f};
  c[ImGuiCol_Separator]                 = (ImVec4){0.267f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_SeparatorHovered]          = (ImVec4){0.384f, 0.447f, 0.643f, 1.000f};
  c[ImGuiCol_SeparatorActive]           = (ImVec4){0.741f, 0.576f, 0.976f, 1.000f};
  c[ImGuiCol_ResizeGrip]                = (ImVec4){0.267f, 0.278f, 0.353f, 0.500f};
  c[ImGuiCol_ResizeGripHovered]         = (ImVec4){0.741f, 0.576f, 0.976f, 0.750f};
  c[ImGuiCol_ResizeGripActive]          = (ImVec4){1.000f, 0.475f, 0.776f, 1.000f};
  c[ImGuiCol_InputTextCursor]           = (ImVec4){0.973f, 0.973f, 0.949f, 1.000f};
  c[ImGuiCol_TabHovered]                = (ImVec4){0.384f, 0.447f, 0.643f, 1.000f};
  c[ImGuiCol_Tab]                       = (ImVec4){0.267f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_TabSelected]               = (ImVec4){0.384f, 0.447f, 0.643f, 1.000f};
  c[ImGuiCol_TabSelectedOverline]       = (ImVec4){0.741f, 0.576f, 0.976f, 1.000f};
  c[ImGuiCol_TabDimmed]                 = (ImVec4){0.157f, 0.165f, 0.212f, 1.000f};
  c[ImGuiCol_TabDimmedSelected]         = (ImVec4){0.267f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_TabDimmedSelectedOverline] = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_PlotLines]                 = (ImVec4){0.545f, 0.914f, 0.992f, 1.000f}; /* Cyan */
  c[ImGuiCol_PlotLinesHovered]          = (ImVec4){0.314f, 0.980f, 0.482f, 1.000f}; /* Green */
  c[ImGuiCol_PlotHistogram]             = (ImVec4){1.000f, 0.722f, 0.424f, 1.000f}; /* Orange */
  c[ImGuiCol_PlotHistogramHovered]      = (ImVec4){1.000f, 0.333f, 0.333f, 1.000f}; /* Red */
  c[ImGuiCol_TableHeaderBg]             = (ImVec4){0.267f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_TableBorderStrong]         = (ImVec4){0.384f, 0.447f, 0.643f, 1.000f};
  c[ImGuiCol_TableBorderLight]          = (ImVec4){0.267f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_TableRowBg]                = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_TableRowBgAlt]             = (ImVec4){1.000f, 1.000f, 1.000f, 0.030f};
  c[ImGuiCol_TextLink]                  = (ImVec4){0.545f, 0.914f, 0.992f, 1.000f};
  c[ImGuiCol_TextSelectedBg]            = (ImVec4){0.267f, 0.278f, 0.353f, 0.750f};
  c[ImGuiCol_TreeLines]                 = (ImVec4){0.267f, 0.278f, 0.353f, 1.000f};
  c[ImGuiCol_DragDropTarget]            = (ImVec4){0.945f, 0.980f, 0.549f, 1.000f}; /* Yellow */
  c[ImGuiCol_DragDropTargetBg]          = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_UnsavedMarker]             = (ImVec4){0.945f, 0.980f, 0.549f, 1.000f};
  c[ImGuiCol_NavCursor]                 = (ImVec4){0.741f, 0.576f, 0.976f, 1.000f};
  c[ImGuiCol_NavWindowingHighlight]     = (ImVec4){1.000f, 1.000f, 1.000f, 0.700f};
  c[ImGuiCol_NavWindowingDimBg]         = (ImVec4){0.800f, 0.800f, 0.800f, 0.200f};
  c[ImGuiCol_ModalWindowDimBg]          = (ImVec4){0.157f, 0.165f, 0.212f, 0.700f};
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

int IsPremadeTheme(const char *name)
{
  return strcmp(name, THEME_DARK)       == 0
      || strcmp(name, THEME_LIGHT)      == 0
      || strcmp(name, THEME_CLASSIC)    == 0
      || strcmp(name, THEME_CATPPUCCIN) == 0
      || strcmp(name, THEME_NORD)       == 0
      || strcmp(name, THEME_DRACULA)    == 0;
}

void ApplyPremadeTheme(const char *name)
{
  if      (strcmp(name, THEME_DARK)       == 0) ApplyDark();
  else if (strcmp(name, THEME_LIGHT)      == 0) ApplyLight();
  else if (strcmp(name, THEME_CLASSIC)    == 0) ApplyClassic();
  else if (strcmp(name, THEME_CATPPUCCIN) == 0) ApplyCatppuccinMocha();
  else if (strcmp(name, THEME_NORD)       == 0) ApplyNord();
  else if (strcmp(name, THEME_DRACULA)    == 0) ApplyDracula();
  else                                          ApplyDark();
}

void ApplyCustomTheme(const Theme *t)
{
  ImGuiStyle *s = igGetStyle();
  for (int i = 0; i < ImGuiCol_COUNT; i++)
    s->Colors[i] = t->colors[i];
}

void GetThemesDir(char *buf, size_t sz)
{
  const char *cfg_dir = GetConfigDir();
  snprintf(buf, sz, "%s" PATH_SEP "themes", cfg_dir);

  struct stat st = {0};
  if (stat(buf, &st) == -1)
    MKDIR(buf);
}

int LoadCustomThemes(const char *dir, Theme **out, int *count)
{
  *out   = NULL;
  *count = 0;

  Theme *themes  = NULL;
  int    nthemes = 0;
  int    cap     = 0;

#ifdef _WIN32
  char pattern[1024];
  snprintf(pattern, sizeof(pattern), "%s\\*.theme", dir);
  WIN32_FIND_DATA fd;
  HANDLE hFind = FindFirstFile(pattern, &fd);
  if (hFind == INVALID_HANDLE_VALUE)
    return 0;

  do
  {
    char path[1024];
    snprintf(path, sizeof(path), "%s\\%s", dir, fd.cFileName);
#else
  DIR *d = opendir(dir);
  if (!d)
    return 0;

  struct dirent *entry;
  while ((entry = readdir(d)) != NULL)
  {
    size_t len = strlen(entry->d_name);
    if (len < 7 || strcmp(entry->d_name + len - 6, ".theme") != 0)
      continue;

    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
#endif

    FILE *f = fopen(path, "r");
    if (!f)
      continue;

    if (nthemes >= cap)
    {
      cap = (cap == 0) ? 8 : cap * 2;
      Theme *tmp = realloc(themes, (size_t)cap * sizeof(Theme));
      if (!tmp)
      {
        fclose(f);
        break;
      }
      themes = tmp;
    }

    Theme *t = &themes[nthemes];
    memset(t, 0, sizeof(*t));
    /* Start with Dark defaults so any unspecified colors are reasonable */
    igStyleColorsDark(igGetStyle());
    ImGuiStyle *s = igGetStyle();
    for (int i = 0; i < ImGuiCol_COUNT; i++)
      t->colors[i] = s->Colors[i];

    char line[256];
    while (fgets(line, sizeof(line), f))
    {
      line[strcspn(line, "\r\n")] = '\0';
      char *eq = strchr(line, '=');
      if (!eq) continue;
      *eq = '\0';
      const char *key = line;
      const char *val = eq + 1;

      if (strcmp(key, "name") == 0)
      {
        strncpy(t->name, val, THEME_NAME_MAX - 1);
        t->name[THEME_NAME_MAX - 1] = '\0';
      }
      else
      {
        int idx = FindColorIndex(key);
        if (idx >= 0)
        {
          float r, g, b, a;
          if (sscanf(val, "%f,%f,%f,%f", &r, &g, &b, &a) == 4)
            t->colors[idx] = (ImVec4){r, g, b, a};
        }
      }
    }
    fclose(f);

    /* Skip entries without a name */
    if (t->name[0] != '\0')
      nthemes++;

#ifdef _WIN32
  } while (FindNextFile(hFind, &fd));
  FindClose(hFind);
#else
  }
  closedir(d);
#endif

  *out   = themes;
  *count = nthemes;
  return nthemes;
}

int SaveCustomTheme(const char *dir, const char *name)
{
  char safe[THEME_NAME_MAX];
  SanitizeFilename(name, safe, sizeof(safe));

  char path[1024];
  snprintf(path, sizeof(path), "%s" PATH_SEP "%s.theme", dir, safe);

  FILE *f = fopen(path, "w");
  if (!f)
    return 0;

  ImGuiStyle *s = igGetStyle();
  fprintf(f, "name=%s\n", name);
  for (int i = 0; i < ImGuiCol_COUNT; i++)
  {
    fprintf(f, "%s=%.6f,%.6f,%.6f,%.6f\n",
            igGetStyleColorName(i),
            s->Colors[i].x, s->Colors[i].y,
            s->Colors[i].z, s->Colors[i].w);
  }
  fclose(f);
  return 1;
}

int DeleteCustomTheme(const char *dir, const char *name)
{
  char safe[THEME_NAME_MAX];
  SanitizeFilename(name, safe, sizeof(safe));

  char path[1024];
  snprintf(path, sizeof(path), "%s" PATH_SEP "%s.theme", dir, safe);
  return remove(path) == 0;
}

void FreeCustomThemes(Theme *themes)
{
  free(themes);
}

void ApplyThemeByName(const char *themesDir, const char *name)
{
  if (!name || name[0] == '\0')
  {
    ApplyDark();
    return;
  }

  if (IsPremadeTheme(name))
  {
    ApplyPremadeTheme(name);
    return;
  }

  if (!themesDir)
  {
    ApplyDark();
    return;
  }

  Theme *themes = NULL;
  int    count  = 0;
  LoadCustomThemes(themesDir, &themes, &count);
  for (int i = 0; i < count; i++)
  {
    if (strcmp(themes[i].name, name) == 0)
    {
      ApplyCustomTheme(&themes[i]);
      FreeCustomThemes(themes);
      return;
    }
  }
  FreeCustomThemes(themes);
  ApplyDark(); /* fallback */
}
