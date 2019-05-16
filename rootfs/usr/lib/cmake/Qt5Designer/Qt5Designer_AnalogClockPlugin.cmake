
add_library(Qt5::AnalogClockPlugin MODULE IMPORTED)

_populate_Designer_plugin_properties(AnalogClockPlugin RELEASE "designer/libcustomwidgetplugin.so")

list(APPEND Qt5Designer_PLUGINS Qt5::AnalogClockPlugin)
