idf_component_register(SRCS "main.c"
                            "../components/wifi/wifi_tt.c"
                            "../components/soket_tcp/soket_tcp_tt.c"
                            "../components/uart/uart_tt.c"
                            "../components/Led/led_tt.c"
                    INCLUDE_DIRS "."
                            "../components/wifi/"
                            "../components/soket_tcp/"
                            "../components/uart/"
                            "../components/Led/"
                            "../components/."
)


