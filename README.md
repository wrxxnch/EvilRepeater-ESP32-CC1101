# Repetidor RF com ESP32 e CC1101

Este projeto permite criar um **repetidor de sinais RF** utilizando um **ESP32** e o módulo **CC1101**, permitindo capturar sinais de dispositivos como **keyfobs** e retransmiti-los. O dispositivo cria um **ponto de acesso Wi-Fi** com interface web para ativar ou desativar o repetidor.

---

## Funcionalidades

- Captura sinais RF em uma frequência configurável (ex.: 433.92 MHz).
- Repetição do sinal capturado via CC1101.
- Interface web simples para controle:
  - Ativar repetidor
  - Desativar repetidor
- Status do repetidor exibido em tempo real.
- Buffer de captura de até 512 pulsos.

---

## Hardware Necessário

- ESP32 (qualquer versão compatível com Wi-Fi)
- Módulo **CC1101**
- Cabos de conexão

### Conexões sugeridas do CC1101 com ESP32

| CC1101 | ESP32 |
|--------|-------|
| MISO   | 19    |
| MOSI   | 23    |
| SCK    | 18    |
| CS     | 5     |
| GDO0   | 4     |
| VCC    | 3.3V  |
| GND    | GND   |

> Ajuste os pinos no código conforme sua montagem.

---

## Software Necessário

- [Arduino IDE](https://www.arduino.cc/en/software)
- Biblioteca **ELECHOUSE_CC1101_SRC_DRV**
- Biblioteca **ESPAsyncWebServer**
- Biblioteca **AsyncTCP**

Instale as bibliotecas pelo Gerenciador de Bibliotecas do Arduino IDE.

---

## Configurações

No código, você pode alterar:

```cpp
// Configurações Wi-Fi AP
IPAddress apIP(192, 168, 4, 1);
const char* ssid = "ESP32-Repetidor";
const char* password = "12345678";

// Configurações CC1101
#define RXPin0 4
#define TXPin0 5
float frequency = 433.92; // Ajuste conforme seu keyfob
```
