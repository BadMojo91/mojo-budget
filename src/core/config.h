#ifndef CONFIG_H
#define CONFIG_H



#define DEFAULT_WINDOW_WIDTH 1024
#define DEFAULT_WINDOW_HEIGHT 768

#ifdef __cplusplus
extern "C" {
#endif


  typedef struct {
    int window_width;
    int window_height;
  }Config;

  extern Config config;

  Config CreateDefaultConfig(void);

// Config file path: ~/.config/mojo-budget
// Reads config from file, or creates default config if file doesn't exist
  void ReadConfig(Config* config);

// Saves config to file, returns 1 on success, 0 on failure
  int SaveConfig(Config* config);

// Returns the user config path: /home/<user>/.config/mojo-budget
  const char* GetConfigDir(void);
  


#ifdef __cplusplus
}
#endif
#endif /* CONFIG_H */
