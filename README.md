# MKE-S17 AGS10 I2C TVOC IAQ Sensor Arduino Library

*(English version below)*

---

## 🇻🇳 TIẾNG VIỆT

Đây là thư viện Arduino tối ưu, độ tin cậy cao dành cho cảm biến **MKE-S17 AGS10** do **MakerEdu.vn** thiết kế và duy trì. Thư viện dùng để đọc chỉ số Tổng các hợp chất hữu cơ dễ bay hơi (TVOC) trong không khí thông qua chuẩn giao tiếp I2C.

Phiên bản thư viện này đã được **tùy chỉnh và sửa nhiều lỗi nghiêm trọng** từ thư viện gốc dựa trên Datasheet chính thức của nhà sản xuất:
- Khắc phục lỗi tràn số bằng kiểu `uint32_t`, cho phép đọc dải đo lên tới 99,999 ppb thay vì bị âm ở 32,767.
- Khắc phục lỗi thuật toán CRC: Thư viện cũ chỉ tính CRC trên 1 byte lệnh đầu tiên khiến các lệnh Hiệu chuẩn (Calibration) và Đổi địa chỉ I2C không hoạt động. Thư viện mới đã sửa tính CRC trên trọn 4 byte.
- Tích hợp hàm `isReady()` giúp kiểm tra trạng thái cảm biến mà không cần dùng hàm delay gây nghẽn chương trình.
- Cho phép custom bus `TwoWire` (hỗ trợ tốt các MCU như ESP32/ESP8266 khi cần custom chân I2C).
- **Tích hợp cơ chế chống Spam I2C** để tuân thủ luật thời gian giãn cách 1.5s theo Datasheet.

### 1. Kết Nối Phần Cứng
*   **VCC** -> 3.3V hoặc 5V (tùy thuộc vào phần cứng module, thông thường là 3.3V)
*   **GND** -> GND
*   **SDA** -> SDA (VD: A4 trên Arduino Uno, D21 trên ESP32)
*   **SCL** -> SCL (VD: A5 trên Arduino Uno, D22 trên ESP32)

### 2. Các Lưu Ý Kỹ Thuật Quan Trọng ⚠️

Dưới đây là các lưu ý cực kỳ quan trọng được trích xuất từ Datasheet của cảm biến AGS10:

1. **Giới Hạn Tốc Độ I2C (Clock Speed):** 
   Cảm biến giao tiếp khá chậm. Theo datasheet, tốc độ I2C tối đa không được vượt quá **15 kHz**. Bạn bắt buộc phải hạ xung I2C trong hàm `setup()` bằng lệnh `Wire.setClock(15000);`.
   
2. **Thời Gian Làm Nóng (Pre-heating Time):** 
   Khi cấp nguồn (VCC), thanh gia nhiệt bên trong cần **120 giây** để khởi động. Trong lúc này, hàm `isReady()` sẽ trả về `false`.
   *Lưu ý:* Nếu bạn chỉ reset vi điều khiển (VD: bấm nút RST trên mạch ESP32) mà không ngắt dây nguồn VCC, cảm biến vẫn tiếp tục giữ nhiệt và sẽ không cần chờ lại 120s.

3. **Cơ Chế Chống Spam (Anti-spam Interval):** 
   Datasheet quy định: khoảng cách giữa 2 lệnh yêu cầu dữ liệu (Data Acquisition) không được nhỏ hơn **1.5 giây**. Nếu gọi lệnh quá sát nhau, cảm biến sẽ bị nghẽn và trả về `0 ppb`. Thư viện của MakerEdu.vn đã **tích hợp sẵn bộ đệm thời gian**. Nếu bạn gọi hàm đọc liên tục (<1.5s), nó sẽ tự động trả về giá trị đã lưu trước đó để bảo vệ mạch I2C của cảm biến.

4. **Quá Trình Chạy Rà (Burn-in / Aging):** 
   Đối với cảm biến mới bóc hộp hoặc để trong kho quá 1 tuần, màng phản ứng hóa học (MOX) sẽ cần thời gian ổn định. Bạn nên cấp nguồn liên tục cho cảm biến (Burn-in) từ **12 đến 72 tiếng** trong lần đầu chạy máy để cảm biến đạt độ chính xác tối đa.

5. **Giải Thích Hiện Tượng Đọc Trả Về `0 ppb`:** 
   Trong môi trường thông thoáng và không khí sạch, chỉ số TVOC = 0 ppb là hoàn toàn bình thường. Để kiểm tra cảm biến có "nhạy" không, hãy đặt một hũ cồn y tế đang mở nắp hoặc xả nhẹ khí gas từ hộp quẹt gần cảm biến. Chỉ số sẽ lập tức tăng vọt!

6. **Sinh Nhiệt & Tuổi Thọ Cảm Biến:** 
   Cảm biến MOX bắt buộc phải tự sinh nhiệt (nung nóng màng hóa học) để đo khí. Việc cảm biến ấm lên là thiết kế cốt lõi, không phải lỗi. Theo datasheet, cảm biến được thiết kế để **chạy liên tục 24/7 với tuổi thọ > 5 năm**. Bạn tuyệt đối KHÔNG NÊN bật/tắt nguồn cảm biến liên tục, vì việc thay đổi nhiệt độ đột ngột (thermal cycling) sẽ làm giảm tuổi thọ và mỗi lần bật lại bạn đều phải chờ quá trình Pre-heating rất lâu.

### 3. Đổi Địa Chỉ I2C (Nâng Cao)
Mặc định cảm biến có địa chỉ `0x1A`. Nếu bạn muốn dùng nhiều cảm biến AGS10 trên cùng một bo mạch, bạn có thể đổi địa chỉ của nó (địa chỉ mới sẽ lưu vĩnh viễn vào bộ nhớ của cảm biến).
```cpp
// Khởi tạo cảm biến với địa chỉ hiện tại (vd: 0x1A)
myAGS10.begin(&Wire, 0x1A);

// Đổi sang địa chỉ mới (vd: 0x1B)
myAGS10.setAddress(0x1B);
```
> **CẢNH BÁO QUAN TRỌNG:** Lệnh đổi địa chỉ được gửi broadcast tới địa chỉ hiện tại. Do đó, bạn **chỉ được cắm DUY NHẤT 1 cảm biến** vào mạch khi chạy lệnh này. Nếu cắm nhiều cảm biến cùng lúc, tất cả chúng sẽ cùng đổi sang địa chỉ mới giống hệt nhau!

### 4. Code Mẫu Cơ Bản

```cpp
#include <Wire.h>
#include <AGS10.h>

AGS10 myAGS10;

void setup() {
  Serial.begin(115200);
  
  // Khởi tạo I2C Bus. Nếu dùng ESP32, bạn có thể truyền chân GPIO: Wire.begin(SDA_PIN, SCL_PIN);
  Wire.begin();
  
  // Cực kỳ quan trọng: Hạ tốc độ I2C xuống tối đa 15kHz cho AGS10
  Wire.setClock(15000); 

  myAGS10.begin(&Wire);
}

void loop() {
  if (myAGS10.isReady()) {
    uint32_t tvoc = myAGS10.readTVOC();
    Serial.print("TVOC: ");
    Serial.print(tvoc);
    Serial.println(" ppb");
  } else {
    Serial.println("Sensor warming up or busy, waiting...");
  }
  delay(1500);
}
```

---
---

## 🇬🇧 ENGLISH

This is a robust and highly reliable Arduino library for the **MKE-S17 AGS10** sensor, developed and maintained by **MakerEdu.vn**. It measures Total Volatile Organic Compounds (TVOC) in the air via I2C communication.

This version has been **customized and heavily bug-fixed** against the official Datasheet:
- Supports 32-bit `uint32_t` TVOC values (avoids 16-bit overflow, unlocking the full 99,999 ppb range).
- **Fixed Write CRC Algorithm**: The original library incorrectly calculated CRC for only the first byte of command packets. This has been fixed to cover all 4 bytes, restoring the ability to Calibrate and modify I2C addresses.
- Built-in data validation via CRC-8 checks on all reads.
- Exposes sensor status via `isReady()` to avoid hardcoded delays.
- Allows passing custom `TwoWire` objects (perfect for ESP32/ESP8266 users).
- **Built-in I2C Anti-Spam protection** to comply with the strict 1.5-second polling interval required by the datasheet.

### 1. Hardware Connections
*   **VCC** -> 3.3V or 5V (Check your specific module, usually 3.3V)
*   **GND** -> GND
*   **SDA** -> SDA (e.g., A4 on Uno, D21 on ESP32)
*   **SCL** -> SCL (e.g., A5 on Uno, D22 on ESP32)

### 2. Critical Technical Notes ⚠️

These notes are derived directly from the AGS10 Datasheet and must be followed for stable operation:

1. **I2C Clock Speed Limit:** 
   The sensor relies on a slow I2C bus. The datasheet mandates a maximum speed of **15 kHz**. You must throttle your I2C clock in your `setup()` function using `Wire.setClock(15000);`.
   
2. **Pre-heating Time:** 
   Upon powering up (VCC), the internal MOX heater requires **120 seconds** to stabilize. During this time, `isReady()` will return `false`. 
   *Note:* If you only soft-reset your MCU (e.g. ESP32 Reset button) without cutting power to the sensor, it remains heated and skips the 120s wait.

3. **Anti-Spam Polling Interval:** 
   The datasheet dictates a minimum interval of **1.5 seconds** between data acquisition requests. If polled faster, the sensor will stall and return `0 ppb`. This library features a **built-in timing cache**: calling `readTVOC()` rapidly will safely return the last known value without spamming the physical I2C bus.

4. **Burn-in / Aging:** 
   For brand-new sensors, or sensors stored for over a week, the chemical material needs an aging period. It is highly recommended to leave the sensor powered on (Burn-in) for **12 to 72 hours** to reach its peak accuracy.

5. **Seeing `0 ppb` TVOC?** 
   In clean and well-ventilated environments, a reading of `0 ppb` is perfectly normal. To test its sensitivity, place an open bottle of medical alcohol or release a tiny puff of unlit lighter gas near the sensor—the TVOC value will spike instantly!

6. **Heat Generation & Lifespan:** 
   MOX sensors MUST generate heat internally to measure gases. The sensor feeling warm is by design. According to the datasheet, it is built for **continuous 24/7 operation with a lifespan of > 5 years**. You should NOT constantly power-cycle the sensor. Rapid thermal cycling reduces lifespan, and you will have to wait for the 120s Pre-heating phase every time you turn it back on.

### 3. Modifying I2C Address (Advanced)
The default I2C address is `0x1A`. If you need to connect multiple AGS10 sensors to the same I2C bus, you can change their addresses (the new address is saved permanently).
```cpp
// Initialize with current address (e.g. 0x1A)
myAGS10.begin(&Wire, 0x1A);

// Change to new address (e.g. 0x1B)
myAGS10.setAddress(0x1B);
```
> **CRITICAL WARNING:** You MUST have **ONLY ONE** sensor connected to the bus when running the address change command! If multiple sensors with the same address are connected, they will all receive the broadcast and change to the exact same new address.

### 4. Basic Example

```cpp
#include <Wire.h>
#include <AGS10.h>

AGS10 myAGS10;

void setup() {
  Serial.begin(115200);
  
  // Init I2C Bus. For ESP32 custom pins: Wire.begin(SDA_PIN, SCL_PIN);
  Wire.begin();
  
  // CRITICAL: Throttle I2C clock to 15kHz max
  Wire.setClock(15000); 

  myAGS10.begin(&Wire);
}

void loop() {
  if (myAGS10.isReady()) {
    uint32_t tvoc = myAGS10.readTVOC();
    Serial.print("TVOC: ");
    Serial.print(tvoc);
    Serial.println(" ppb");
  } else {
    Serial.println("Sensor warming up or busy, waiting...");
  }
  delay(1500);
}
```

## Repository
Developed and maintained by [MakerEdu.vn](https://github.com/makereduvn/MKE-S17-AGS10-I2C-TVOC-IAQ-SENSOR).
