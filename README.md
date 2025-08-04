"# practica1_com" 
This embedded system project creates a TCP Echo server using FreeRTOS and Lwlp with integrated AES decryption and CRC32 checksum validation.
Built to run on a NXP microcontroller.
| File                                       | Purpose                                                                  |
| ------------------------------------------ | ------------------------------------------------------------------------ |
| `aaj_main.txt` / `lwip_tcpecho_freertos.c` | Main application logic: board/network init, FreeRTOS setup, starts tasks |
| `aaj_layer.c`                              | CRC32 initialization helper function                                     |
| `aaj_layer.h`                              | AES key/IV/output buffer definitions                                     |
| `aaj_layer_config.h`                       | Test message buffers and sample encrypted messages                       |
| `aaj_layer_interface.h`                    | Declares `aescrc_test_task()`                                            |
| `aes.c`                                    | Complete AES (ECB/CBC/CTR) implementation                                |
| `aes.h`                                    | AES context and API definitions                                          |
| `FreeRTOSConfig.h`                         | Configuration for FreeRTOS kernel features (e.g., stack size, timers)    |
| `lwipopts.h`                               | Configuration for LwIP stack (e.g., TCP parameters, memory pools)        |
| `semihost_hardfault.c`                     | **Custom Hard Fault Handler for Semihosting**                         |
