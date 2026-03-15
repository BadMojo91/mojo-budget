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

static ImVec4 HexColor(unsigned int rgb, float alpha)
{
  return (ImVec4){
    ((float)((rgb >> 16) & 0xFFu)) / 255.0f,
    ((float)((rgb >> 8) & 0xFFu)) / 255.0f,
    ((float)(rgb & 0xFFu)) / 255.0f,
    alpha,
  };
}

typedef struct
{
  unsigned int text;
  unsigned int overlay0;
  unsigned int surface2;
  unsigned int surface1;
  unsigned int surface0;
  unsigned int base;
  unsigned int mantle;
  unsigned int crust;
  unsigned int mauve;
  unsigned int blue;
  unsigned int sapphire;
  unsigned int teal;
  unsigned int yellow;
  unsigned int peach;
} CatppuccinPalette;

static void ApplyCatppuccinPalette(const CatppuccinPalette *p)
{
  ImGuiStyle *s = igGetStyle();
  ImVec4     *c = s->Colors;

  c[ImGuiCol_Text]                      = HexColor(p->text, 1.000f);
  c[ImGuiCol_TextDisabled]              = HexColor(p->overlay0, 1.000f);
  c[ImGuiCol_WindowBg]                  = HexColor(p->base, 1.000f);
  c[ImGuiCol_ChildBg]                   = HexColor(p->base, 1.000f);
  c[ImGuiCol_PopupBg]                   = HexColor(p->mantle, 0.960f);
  c[ImGuiCol_Border]                    = HexColor(p->surface0, 1.000f);
  c[ImGuiCol_BorderShadow]              = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_FrameBg]                   = HexColor(p->surface0, 1.000f);
  c[ImGuiCol_FrameBgHovered]            = HexColor(p->surface1, 1.000f);
  c[ImGuiCol_FrameBgActive]             = HexColor(p->surface2, 1.000f);
  c[ImGuiCol_TitleBg]                   = HexColor(p->mantle, 1.000f);
  c[ImGuiCol_TitleBgActive]             = HexColor(p->surface0, 1.000f);
  c[ImGuiCol_TitleBgCollapsed]          = HexColor(p->base, 0.750f);
  c[ImGuiCol_MenuBarBg]                 = HexColor(p->mantle, 1.000f);
  c[ImGuiCol_ScrollbarBg]               = HexColor(p->mantle, 1.000f);
  c[ImGuiCol_ScrollbarGrab]             = HexColor(p->surface0, 1.000f);
  c[ImGuiCol_ScrollbarGrabHovered]      = HexColor(p->surface1, 1.000f);
  c[ImGuiCol_ScrollbarGrabActive]       = HexColor(p->surface2, 1.000f);
  c[ImGuiCol_CheckMark]                 = HexColor(p->mauve, 1.000f);
  c[ImGuiCol_SliderGrab]                = HexColor(p->blue, 1.000f);
  c[ImGuiCol_SliderGrabActive]          = HexColor(p->sapphire, 1.000f);
  c[ImGuiCol_Button]                    = HexColor(p->surface0, 1.000f);
  c[ImGuiCol_ButtonHovered]             = HexColor(p->surface1, 1.000f);
  c[ImGuiCol_ButtonActive]              = HexColor(p->surface2, 1.000f);
  c[ImGuiCol_Header]                    = HexColor(p->surface1, 1.000f);
  c[ImGuiCol_HeaderHovered]             = HexColor(p->surface2, 1.000f);
  c[ImGuiCol_HeaderActive]              = HexColor(p->overlay0, 1.000f);
  c[ImGuiCol_Separator]                 = HexColor(p->surface0, 1.000f);
  c[ImGuiCol_SeparatorHovered]          = HexColor(p->surface2, 1.000f);
  c[ImGuiCol_SeparatorActive]           = HexColor(p->blue, 1.000f);
  c[ImGuiCol_ResizeGrip]                = HexColor(p->surface0, 0.500f);
  c[ImGuiCol_ResizeGripHovered]         = HexColor(p->blue, 0.750f);
  c[ImGuiCol_ResizeGripActive]          = HexColor(p->sapphire, 1.000f);
  c[ImGuiCol_InputTextCursor]           = HexColor(p->text, 1.000f);
  c[ImGuiCol_TabHovered]                = HexColor(p->surface2, 1.000f);
  c[ImGuiCol_Tab]                       = HexColor(p->surface0, 1.000f);
  c[ImGuiCol_TabSelected]               = HexColor(p->surface1, 1.000f);
  c[ImGuiCol_TabSelectedOverline]       = HexColor(p->blue, 1.000f);
  c[ImGuiCol_TabDimmed]                 = HexColor(p->base, 1.000f);
  c[ImGuiCol_TabDimmedSelected]         = HexColor(p->surface0, 1.000f);
  c[ImGuiCol_TabDimmedSelectedOverline] = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_PlotLines]                 = HexColor(p->teal, 1.000f);
  c[ImGuiCol_PlotLinesHovered]          = HexColor(p->sapphire, 1.000f);
  c[ImGuiCol_PlotHistogram]             = HexColor(p->blue, 1.000f);
  c[ImGuiCol_PlotHistogramHovered]      = HexColor(p->peach, 1.000f);
  c[ImGuiCol_TableHeaderBg]             = HexColor(p->surface0, 1.000f);
  c[ImGuiCol_TableBorderStrong]         = HexColor(p->surface2, 1.000f);
  c[ImGuiCol_TableBorderLight]          = HexColor(p->surface0, 1.000f);
  c[ImGuiCol_TableRowBg]                = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_TableRowBgAlt]             = HexColor(p->text, 0.030f);
  c[ImGuiCol_TextLink]                  = HexColor(p->blue, 1.000f);
  c[ImGuiCol_TextSelectedBg]            = HexColor(p->surface1, 0.750f);
  c[ImGuiCol_TreeLines]                 = HexColor(p->surface0, 1.000f);
  c[ImGuiCol_DragDropTarget]            = HexColor(p->yellow, 1.000f);
  c[ImGuiCol_DragDropTargetBg]          = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_UnsavedMarker]             = HexColor(p->yellow, 1.000f);
  c[ImGuiCol_NavCursor]                 = HexColor(p->blue, 1.000f);
  c[ImGuiCol_NavWindowingHighlight]     = (ImVec4){1.000f, 1.000f, 1.000f, 0.700f};
  c[ImGuiCol_NavWindowingDimBg]         = (ImVec4){0.800f, 0.800f, 0.800f, 0.200f};
  c[ImGuiCol_ModalWindowDimBg]          = HexColor(p->base, 0.700f);
}

static void ApplyHighContrastTheme(unsigned int text, unsigned int textDisabled,
                                   unsigned int accent, unsigned int accentAlt,
                                   unsigned int accentWarm)
{
  ImGuiStyle *s = igGetStyle();
  ImVec4     *c = s->Colors;

  c[ImGuiCol_Text]                      = HexColor(text, 1.000f);
  c[ImGuiCol_TextDisabled]              = HexColor(textDisabled, 1.000f);
  c[ImGuiCol_WindowBg]                  = HexColor(0x000000u, 1.000f);
  c[ImGuiCol_ChildBg]                   = HexColor(0x000000u, 1.000f);
  c[ImGuiCol_PopupBg]                   = HexColor(0x040404u, 0.980f);
  c[ImGuiCol_Border]                    = HexColor(accent, 1.000f);
  c[ImGuiCol_BorderShadow]              = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_FrameBg]                   = HexColor(0x080808u, 1.000f);
  c[ImGuiCol_FrameBgHovered]            = HexColor(accent, 0.220f);
  c[ImGuiCol_FrameBgActive]             = HexColor(accent, 0.350f);
  c[ImGuiCol_TitleBg]                   = HexColor(0x000000u, 1.000f);
  c[ImGuiCol_TitleBgActive]             = HexColor(accent, 0.180f);
  c[ImGuiCol_TitleBgCollapsed]          = HexColor(0x000000u, 0.850f);
  c[ImGuiCol_MenuBarBg]                 = HexColor(0x050505u, 1.000f);
  c[ImGuiCol_ScrollbarBg]               = HexColor(0x020202u, 1.000f);
  c[ImGuiCol_ScrollbarGrab]             = HexColor(accent, 0.450f);
  c[ImGuiCol_ScrollbarGrabHovered]      = HexColor(accent, 0.700f);
  c[ImGuiCol_ScrollbarGrabActive]       = HexColor(accentAlt, 1.000f);
  c[ImGuiCol_CheckMark]                 = HexColor(accentAlt, 1.000f);
  c[ImGuiCol_SliderGrab]                = HexColor(accent, 1.000f);
  c[ImGuiCol_SliderGrabActive]          = HexColor(accentAlt, 1.000f);
  c[ImGuiCol_Button]                    = HexColor(0x080808u, 1.000f);
  c[ImGuiCol_ButtonHovered]             = HexColor(accent, 0.260f);
  c[ImGuiCol_ButtonActive]              = HexColor(accent, 0.420f);
  c[ImGuiCol_Header]                    = HexColor(accent, 0.240f);
  c[ImGuiCol_HeaderHovered]             = HexColor(accent, 0.380f);
  c[ImGuiCol_HeaderActive]              = HexColor(accentAlt, 0.520f);
  c[ImGuiCol_Separator]                 = HexColor(accent, 0.900f);
  c[ImGuiCol_SeparatorHovered]          = HexColor(accentAlt, 1.000f);
  c[ImGuiCol_SeparatorActive]           = HexColor(accentAlt, 1.000f);
  c[ImGuiCol_ResizeGrip]                = HexColor(accent, 0.600f);
  c[ImGuiCol_ResizeGripHovered]         = HexColor(accentAlt, 0.850f);
  c[ImGuiCol_ResizeGripActive]          = HexColor(accentAlt, 1.000f);
  c[ImGuiCol_InputTextCursor]           = HexColor(text, 1.000f);
  c[ImGuiCol_TabHovered]                = HexColor(accent, 0.320f);
  c[ImGuiCol_Tab]                       = HexColor(0x050505u, 1.000f);
  c[ImGuiCol_TabSelected]               = HexColor(accent, 0.240f);
  c[ImGuiCol_TabSelectedOverline]       = HexColor(accentAlt, 1.000f);
  c[ImGuiCol_TabDimmed]                 = HexColor(0x000000u, 1.000f);
  c[ImGuiCol_TabDimmedSelected]         = HexColor(accent, 0.160f);
  c[ImGuiCol_TabDimmedSelectedOverline] = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_PlotLines]                 = HexColor(accentAlt, 1.000f);
  c[ImGuiCol_PlotLinesHovered]          = HexColor(accentWarm, 1.000f);
  c[ImGuiCol_PlotHistogram]             = HexColor(accentWarm, 1.000f);
  c[ImGuiCol_PlotHistogramHovered]      = HexColor(accentAlt, 1.000f);
  c[ImGuiCol_TableHeaderBg]             = HexColor(0x050505u, 1.000f);
  c[ImGuiCol_TableBorderStrong]         = HexColor(accent, 0.800f);
  c[ImGuiCol_TableBorderLight]          = HexColor(accent, 0.350f);
  c[ImGuiCol_TableRowBg]                = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_TableRowBgAlt]             = HexColor(accent, 0.080f);
  c[ImGuiCol_TextLink]                  = HexColor(accentAlt, 1.000f);
  c[ImGuiCol_TextSelectedBg]            = HexColor(accent, 0.300f);
  c[ImGuiCol_TreeLines]                 = HexColor(accent, 0.600f);
  c[ImGuiCol_DragDropTarget]            = HexColor(accentWarm, 1.000f);
  c[ImGuiCol_DragDropTargetBg]          = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_UnsavedMarker]             = HexColor(accentWarm, 1.000f);
  c[ImGuiCol_NavCursor]                 = HexColor(accentAlt, 1.000f);
  c[ImGuiCol_NavWindowingHighlight]     = HexColor(accentAlt, 0.850f);
  c[ImGuiCol_NavWindowingDimBg]         = HexColor(0x000000u, 0.550f);
  c[ImGuiCol_ModalWindowDimBg]          = HexColor(0x000000u, 0.850f);
}

/* -------------------------------------------------------------------------
 * Premade theme implementations
 * ---------------------------------------------------------------------- */

static void ApplyDark(void)    { igStyleColorsDark(igGetStyle()); }
static void ApplyLight(void)   { igStyleColorsLight(igGetStyle()); }
static void ApplyClassic(void) { igStyleColorsClassic(igGetStyle()); }

static void ApplyCatppuccinLatte(void)
{
  static const CatppuccinPalette palette = {
    0x4C4F69u, 0x9CA0B0u, 0xACB0BEu, 0xBCC0CCu, 0xCCD0DAu,
    0xEFF1F5u, 0xE6E9EFu, 0xDCE0E8u, 0x8839EFu, 0x1E66F5u,
    0x209FB5u, 0x179299u, 0xDF8E1Du, 0xFE640Bu,
  };
  ApplyCatppuccinPalette(&palette);
}

static void ApplyCatppuccinFrappe(void)
{
  static const CatppuccinPalette palette = {
    0xC6D0F5u, 0x737994u, 0x626880u, 0x51576Du, 0x414559u,
    0x303446u, 0x292C3Cu, 0x232634u, 0xCA9EE6u, 0x8CAAEEu,
    0x85C1DCu, 0x81C8BEu, 0xE5C890u, 0xEF9F76u,
  };
  ApplyCatppuccinPalette(&palette);
}

static void ApplyCatppuccinMacchiato(void)
{
  static const CatppuccinPalette palette = {
    0xCAD3F5u, 0x6E738Du, 0x5B6078u, 0x494D64u, 0x363A4Fu,
    0x24273Au, 0x1E2030u, 0x181926u, 0xC6A0F6u, 0x8AADF4u,
    0x7DC4E4u, 0x8BD5CAu, 0xEED49Fu, 0xF5A97Fu,
  };
  ApplyCatppuccinPalette(&palette);
}

static void ApplyCatppuccinMocha(void)
{
  static const CatppuccinPalette palette = {
    0xCDD6F4u, 0x6C7086u, 0x585B70u, 0x45475Au, 0x313244u,
    0x1E1E2Eu, 0x181825u, 0x11111Bu, 0xCBA6F7u, 0x89B4FAu,
    0x74C7ECu, 0x94E2D5u, 0xF9E2AFu, 0xFAB387u,
  };
  ApplyCatppuccinPalette(&palette);
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

static void ApplyHighContrastAmber(void)
{
  ApplyHighContrastTheme(0xFFE082u, 0xA67C00u, 0xFFB000u, 0xFFD54Fu, 0xFF7043u);
}

static void ApplyHighContrastCyan(void)
{
  ApplyHighContrastTheme(0xB2F5FFu, 0x2F7E8Au, 0x00E5FFu, 0x7AF7FFu, 0xFF3CF7u);
}

static void ApplyHighContrastLime(void)
{
  ApplyHighContrastTheme(0xE6FF9Au, 0x6E8F1Fu, 0x9DFF00u, 0xCFFF5Eu, 0xFFB300u);
}

static void ApplyPsychedelicNeon(void)
{
  ImGuiStyle *s = igGetStyle();
  ImVec4     *c = s->Colors;

  c[ImGuiCol_Text]                      = HexColor(0xF7F4FFu, 1.000f);
  c[ImGuiCol_TextDisabled]              = HexColor(0xB993FFu, 1.000f);
  c[ImGuiCol_WindowBg]                  = HexColor(0x140021u, 1.000f);
  c[ImGuiCol_ChildBg]                   = HexColor(0x140021u, 1.000f);
  c[ImGuiCol_PopupBg]                   = HexColor(0x1F0033u, 0.960f);
  c[ImGuiCol_Border]                    = HexColor(0x2CF8FFu, 0.950f);
  c[ImGuiCol_BorderShadow]              = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_FrameBg]                   = HexColor(0x26003Fu, 1.000f);
  c[ImGuiCol_FrameBgHovered]            = HexColor(0xFF4FD8u, 0.500f);
  c[ImGuiCol_FrameBgActive]             = HexColor(0x7A5CFFu, 0.700f);
  c[ImGuiCol_TitleBg]                   = HexColor(0x1A002Bu, 1.000f);
  c[ImGuiCol_TitleBgActive]             = HexColor(0x7A5CFFu, 0.600f);
  c[ImGuiCol_TitleBgCollapsed]          = HexColor(0x140021u, 0.800f);
  c[ImGuiCol_MenuBarBg]                 = HexColor(0x1A002Bu, 1.000f);
  c[ImGuiCol_ScrollbarBg]               = HexColor(0x140021u, 1.000f);
  c[ImGuiCol_ScrollbarGrab]             = HexColor(0x2CF8FFu, 0.500f);
  c[ImGuiCol_ScrollbarGrabHovered]      = HexColor(0x7CFF6Bu, 0.700f);
  c[ImGuiCol_ScrollbarGrabActive]       = HexColor(0xFFF35Cu, 0.950f);
  c[ImGuiCol_CheckMark]                 = HexColor(0x7CFF6Bu, 1.000f);
  c[ImGuiCol_SliderGrab]                = HexColor(0x2CF8FFu, 1.000f);
  c[ImGuiCol_SliderGrabActive]          = HexColor(0xFF4FD8u, 1.000f);
  c[ImGuiCol_Button]                    = HexColor(0x26003Fu, 1.000f);
  c[ImGuiCol_ButtonHovered]             = HexColor(0x2CF8FFu, 0.350f);
  c[ImGuiCol_ButtonActive]              = HexColor(0xFF4FD8u, 0.550f);
  c[ImGuiCol_Header]                    = HexColor(0x7A5CFFu, 0.350f);
  c[ImGuiCol_HeaderHovered]             = HexColor(0x2CF8FFu, 0.400f);
  c[ImGuiCol_HeaderActive]              = HexColor(0xFF4FD8u, 0.550f);
  c[ImGuiCol_Separator]                 = HexColor(0xFFF35Cu, 0.850f);
  c[ImGuiCol_SeparatorHovered]          = HexColor(0x2CF8FFu, 1.000f);
  c[ImGuiCol_SeparatorActive]           = HexColor(0xFF4FD8u, 1.000f);
  c[ImGuiCol_ResizeGrip]                = HexColor(0x2CF8FFu, 0.550f);
  c[ImGuiCol_ResizeGripHovered]         = HexColor(0x7CFF6Bu, 0.800f);
  c[ImGuiCol_ResizeGripActive]          = HexColor(0xFFF35Cu, 1.000f);
  c[ImGuiCol_InputTextCursor]           = HexColor(0xF7F4FFu, 1.000f);
  c[ImGuiCol_TabHovered]                = HexColor(0x2CF8FFu, 0.280f);
  c[ImGuiCol_Tab]                       = HexColor(0x220038u, 1.000f);
  c[ImGuiCol_TabSelected]               = HexColor(0x7A5CFFu, 0.350f);
  c[ImGuiCol_TabSelectedOverline]       = HexColor(0x7CFF6Bu, 1.000f);
  c[ImGuiCol_TabDimmed]                 = HexColor(0x140021u, 1.000f);
  c[ImGuiCol_TabDimmedSelected]         = HexColor(0x4A1382u, 0.550f);
  c[ImGuiCol_TabDimmedSelectedOverline] = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_PlotLines]                 = HexColor(0x2CF8FFu, 1.000f);
  c[ImGuiCol_PlotLinesHovered]          = HexColor(0x7CFF6Bu, 1.000f);
  c[ImGuiCol_PlotHistogram]             = HexColor(0xFFF35Cu, 1.000f);
  c[ImGuiCol_PlotHistogramHovered]      = HexColor(0xFF4FD8u, 1.000f);
  c[ImGuiCol_TableHeaderBg]             = HexColor(0x220038u, 1.000f);
  c[ImGuiCol_TableBorderStrong]         = HexColor(0x2CF8FFu, 0.850f);
  c[ImGuiCol_TableBorderLight]          = HexColor(0x7A5CFFu, 0.450f);
  c[ImGuiCol_TableRowBg]                = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_TableRowBgAlt]             = HexColor(0x2CF8FFu, 0.070f);
  c[ImGuiCol_TextLink]                  = HexColor(0x2CF8FFu, 1.000f);
  c[ImGuiCol_TextSelectedBg]            = HexColor(0xFF4FD8u, 0.350f);
  c[ImGuiCol_TreeLines]                 = HexColor(0x7A5CFFu, 0.600f);
  c[ImGuiCol_DragDropTarget]            = HexColor(0xFFF35Cu, 1.000f);
  c[ImGuiCol_DragDropTargetBg]          = (ImVec4){0.000f, 0.000f, 0.000f, 0.000f};
  c[ImGuiCol_UnsavedMarker]             = HexColor(0xFFF35Cu, 1.000f);
  c[ImGuiCol_NavCursor]                 = HexColor(0x7CFF6Bu, 1.000f);
  c[ImGuiCol_NavWindowingHighlight]     = HexColor(0x2CF8FFu, 0.850f);
  c[ImGuiCol_NavWindowingDimBg]         = HexColor(0x000000u, 0.350f);
  c[ImGuiCol_ModalWindowDimBg]          = HexColor(0x0A0011u, 0.820f);
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

int IsPremadeTheme(const char *name)
{
  return strcmp(name, THEME_DARK)       == 0
      || strcmp(name, THEME_LIGHT)      == 0
      || strcmp(name, THEME_CLASSIC)    == 0
      || strcmp(name, THEME_CATPPUCCIN_LATTE) == 0
      || strcmp(name, THEME_CATPPUCCIN_FRAPPE) == 0
      || strcmp(name, THEME_CATPPUCCIN_MACCHIATO) == 0
      || strcmp(name, THEME_CATPPUCCIN_MOCHA) == 0
      || strcmp(name, THEME_NORD)       == 0
      || strcmp(name, THEME_DRACULA)    == 0
      || strcmp(name, THEME_HIGH_CONTRAST_AMBER) == 0
      || strcmp(name, THEME_HIGH_CONTRAST_CYAN) == 0
      || strcmp(name, THEME_HIGH_CONTRAST_LIME) == 0
      || strcmp(name, THEME_PSYCHEDELIC_NEON) == 0;
}

void ApplyPremadeTheme(const char *name)
{
  if      (strcmp(name, THEME_DARK)       == 0) ApplyDark();
  else if (strcmp(name, THEME_LIGHT)      == 0) ApplyLight();
  else if (strcmp(name, THEME_CLASSIC)    == 0) ApplyClassic();
  else if (strcmp(name, THEME_CATPPUCCIN_LATTE) == 0) ApplyCatppuccinLatte();
  else if (strcmp(name, THEME_CATPPUCCIN_FRAPPE) == 0) ApplyCatppuccinFrappe();
  else if (strcmp(name, THEME_CATPPUCCIN_MACCHIATO) == 0) ApplyCatppuccinMacchiato();
  else if (strcmp(name, THEME_CATPPUCCIN_MOCHA) == 0) ApplyCatppuccinMocha();
  else if (strcmp(name, THEME_NORD)       == 0) ApplyNord();
  else if (strcmp(name, THEME_DRACULA)    == 0) ApplyDracula();
  else if (strcmp(name, THEME_HIGH_CONTRAST_AMBER) == 0) ApplyHighContrastAmber();
  else if (strcmp(name, THEME_HIGH_CONTRAST_CYAN) == 0) ApplyHighContrastCyan();
  else if (strcmp(name, THEME_HIGH_CONTRAST_LIME) == 0) ApplyHighContrastLime();
  else if (strcmp(name, THEME_PSYCHEDELIC_NEON) == 0) ApplyPsychedelicNeon();
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
