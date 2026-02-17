idf_component_register(
    SRCS "main.c"                           # Тільки той файл, що реально є в папці main
    INCLUDE_DIRS "."
    REQUIRES my_joystick my_led led_strip espressif__button  # Додаємо твої компоненти сюди
)