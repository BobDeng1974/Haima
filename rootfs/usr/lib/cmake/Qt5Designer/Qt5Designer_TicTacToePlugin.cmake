
add_library(Qt5::TicTacToePlugin MODULE IMPORTED)

_populate_Designer_plugin_properties(TicTacToePlugin RELEASE "designer/libtaskmenuextension.so")

list(APPEND Qt5Designer_PLUGINS Qt5::TicTacToePlugin)
