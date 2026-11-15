# Ý tưởng dùng board (nghiên cứu 2026-11-14)

Lưu ý: nghiên cứu ban đầu tưởng board là CYD 2.8"; firmware có sẵn phải chọn bản cho 3248S035 / 3.5".

## Firmware có sẵn
- openHASP / ESPHome: bảng điều khiển Home Assistant.
- ESP32 Marauder / Bruce / Ghost ESP: học bảo mật WiFi/BLE, chỉ dùng trên mạng của mình.
- NerdMiner: "vé số" Bitcoin.
- CYD-Klipper: điều khiển máy in 3D (chưa xác minh lại).

## Tự code (LVGL)
Trạm thời tiết (Open-Meteo), macro pad BLE kiểu Stream Deck, dashboard để bàn, khung ảnh từ thẻ SD, pet Tamagotchi, radar máy bay (OpenSky).

## Khi có loa + mic
Trợ lý giọng nói (hướng chính của dự án), bộ đàm ESP-NOW (cần 2 board), máy lên dây đàn (FFT), phân tích phổ / đo độ ồn, máy ghi âm vào thẻ SD, radio internet / MP3, báo thức biết nói.

## Nguồn
- https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display
- https://randomnerdtutorials.com/esp32-tft-lvgl-weather-station
- https://learn.adafruit.com/esp-now-walkie-talkies
