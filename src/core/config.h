#ifndef CONFIG_H
#define CONFIG_H



#define DEFAULT_WINDOW_WIDTH 1024
#define DEFAULT_WINDOW_HEIGHT 768
#define RECENT_FILES_MAX 10

#ifdef __cplusplus
extern "C" {
#endif


  typedef struct {
    int  window_width;
    int  window_height;
    char activeTheme[256];
    char recentFiles[RECENT_FILES_MAX][4096];
    int  recentFileCount;
  }Config;

  extern Config config;

  Config CreateDefaultConfig(void);

// Config file path: ~/.config/mojo-budget (Linux) or %APPDATA%\mojo-budget (Windows)
// Reads config from file, or creates default config if file doesn't exist
  void ReadConfig(Config* config);

// Saves config to file, returns 1 on success, 0 on failure
  int SaveConfig(Config* config);

// Returns the user config directory: ~/.config/mojo-budget (Linux) or %APPDATA%\mojo-budget (Windows)
  const char* GetConfigDir(void);

// Inserts path at the front of cfg->recentFiles, deduplicating and capping at RECENT_FILES_MAX.
  void AddRecentFile(Config* cfg, const char* path);



#ifdef __cplusplus
}
#endif
#endif /* CONFIG_H */
