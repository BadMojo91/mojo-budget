#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_WINDOW_WIDTH 1024
#define DEFAULT_WINDOW_HEIGHT 768
#define DEFAULT_COLLUMN_WIDTH 32
#define DEFAULT_BILL_TABLE_HEIGHT 0.0f

#define NUM_BILL_COLUMNS 9

#ifdef __cplusplus
extern "C" {
#endif


  typedef struct {
    int window_width;
    int window_height;
    float bill_column_widths[NUM_BILL_COLUMNS];
    float bill_table_height;
  }Config;

  void ReadConfig(Config* config);
  int SaveConfig(Config* config);

  const char* GetConfigDir(void);



#ifdef __cplusplus
}
#endif
#endif /* CONFIG_H */
