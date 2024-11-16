# Custom Zephyr function that imports the button/ subdirectory if the Kconfig
# option MCP9808 is defined
add_subdirectory_ifdef(CONFIG_CUSTOM_BUTTON button)