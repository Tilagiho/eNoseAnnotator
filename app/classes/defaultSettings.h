#ifndef DEFAULTVALUES_H
#define DEFAULTVALUES_H

#include <QColor>

// debug settings
#define CLEAR_SETTINGS false

// directories
#define DATA_DIR_KEY "directories/dataDir"
#define DEFAULT_DATA_DIR "."

#define EXPORT_DIR_KEY "directories/exportDir"
#define DEFAULT_EXPORT_DIR "."

#define CLASSIFIER_DIR_KEY "directories/classifierDir"
#define DEFAULT_CLASSIFIER_DIR "."

#define PRESET_DIR_KEY "directories/presetDir"
#define DEFAULT_PRESET_DIR "./presets/"
#define PRESET_SOURCE "../share/presets/"

// limits
#define USE_LIMITS_KEY "settings/useLimits"
#define DEFAULT_USE_LIMITS false
#define UPPER_LIMIT_KEY "settings/upperLimit"
#define DEFAULT_UPPER_LIMIT 90000.0
#define LOWER_LIMIT_KEY "settings/lowerLimit"
#define DEFAULT_LOWER_LIMIT 300.0

// colors
# define GRAPH_BACKGROUND_COLOR QColor(255,250,240)

// curve fit settings
#define CVWIZ_DEFAULT_MODEL_TYPE LeastSquaresFitter::Type::SUPERPOS
#define CVWIZ_DEFAULT_JUMP_BASE_THRESHOLD 0.2
#define CVWIZ_DEFAULT_JUMP_FACTOR 2.0
#define CVWIZ_DEFAULT_RECOVERY_FACTOR 0.25
#define CVWIZ_DEFAULT_BUFFER_SIZE 15
#define CVWIZ_DEFAULT_DETECT_EXPOSITION_START true
#define CVWIZ_DEFAULT_DETECT_RECOVERY_START false

#endif // DEFAULTVALUES_H
