# ESP32Dali

ESP32Dali is project to interface with [DALI](https://en.wikipedia.org/wiki/Digital_Addressable_Lighting_Interface) compatible lighting devices using [ESPHome](https://esphome.io/) project. The board supports isolated communication to DALI bus as a master device and the board can also work as 18V DALI power supply from single 5V supply. ESP32Dali is a single 2-layer matchbox sized PCB to host ESP32-WROOM-32 and required interfacing components. The schematics and board files are in the `hw` folder ([KiCanvas](https://kicanvas.org/?github=https%3A%2F%2Fgithub.com%2Fpetrinm%2FESP32Dali%2Fblob%2Fmain%2Fhw%2FESPDali.kicad_sch), [pdf](https://github.com/petrinm/ESP32Dali/blob/main/hw/ESPDali.pdf)) 3D step files and printed enclosure are also available. The software is based on ESPHome project and its installation is documented in the software README.

![ESP32Dali](https://raw.githubusercontent.com/petrinm/ESP32Dali/refs/heads/main/hw/ESP32Dali.png)
![ESP32Dali front side](https://raw.githubusercontent.com/petrinm/ESP32Dali/refs/heads/main/hw/ESPDali_front.jpg)
![ESP32Dali back side](https://raw.githubusercontent.com/petrinm/ESP32Dali/refs/heads/main/hw/ESPDali_back.jpg)

The project is licenced under MIT license.

# Example configuration

```yaml
esphome:
  name: livingroom
  friendly_name: Livingroom

esp32:
  board: esp32dev
  framework:
    type: arduino

light:
  - platform: monochromatic
    name: "LED strip"
    output: dali_output

dali:
  id: dali_if
  tx_pin:
    number: 17
    inverted: true
  rx_pin:
    number: 16
    inverted: true

output:
  - platform: dali
    id: dali_output
    interface: dali_if
    address: 5
```

Run
```bash
$ esphome run livingroom.yaml
```


## Installing ESP32Dali as external component

https://esphome.io/components/external_components


```yaml
external_components:
  - source:
      type: git
      url: https://github.com/petrinm/ESP32Dali.git
    components: [ dali ]
```


