idf_component_register(
    SRCS "main.c" 
         "data-storage.c"
         "wifiConnectivity.c"
         "commondef.c"
         "tcp_server.c"
    INCLUDE_DIRS ""
    EMBED_FILES "home.html" "settings.html" "info.html"
)


# Create a SPIFFS image from the contents of the 'spiffs_image' directory
# that fits the partition named 'storage'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'idf.py -p PORT flash'.
spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)