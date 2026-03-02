#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
  int window_width;
  int window_height;
}Config;

void ReadConfig(Config* config);
int SaveConfig(Config* config);

const char* GetConfigDir(void);



#ifdef __cplusplus
}
#endif
#endif /* CONFIG_H */
