# Requirement: Temperature Control System Simulator

## 1. Tổng quan

Xây dựng chương trình C++ mô phỏng hệ thống giám sát nhiệt độ phòng máy. Cảm biến nhiệt độ được giả lập bằng phần mềm. Controller đọc nhiệt độ và quyết định trạng thái của quạt và cảnh báo.

**Kiến trúc:**
```
TemperatureSensor → TemperatureController → Fan / Alarm → Display / Logger
```

---

## 2. Yêu cầu chức năng bắt buộc (Mandatory)

| ID | Yêu cầu | Mô tả chi tiết | Trạng thái |
|----|---------|----------------|:----------:|
| TC-01 | AUTO / MANUAL mode | AUTO: chương trình tự sinh nhiệt độ ngẫu nhiên. MANUAL: người dùng nhập nhiệt độ từ bàn phím | ✅ |
| TC-02 | Khoảng nhiệt độ hợp lệ | Quy định khoảng hợp lệ: 0–60°C | ✅ |
| TC-03 | Nhiệt độ < 30°C | Fan OFF, Alarm OFF → Trạng thái NORMAL | ✅ |
| TC-04 | Nhiệt độ 30–39°C | Fan ON, Alarm OFF → Trạng thái WARNING | ✅ |
| TC-05 | Nhiệt độ ≥ 40°C | Fan ON, Alarm ON, hiển thị cảnh báo → Trạng thái CRITICAL | ✅ |
| TC-06 | Hiển thị trạng thái | Hiển thị Temperature, Fan, Alarm và trạng thái hệ thống trên console | ✅ |
| TC-07 | Chu kỳ 1 giây | Hệ thống chạy theo Super-Loop, 1 giây/chu kỳ | ✅ |
| TC-08 | Phát hiện dữ liệu lỗi | Dữ liệu sensor ngoài [0, 60]°C phải được phát hiện và không xử lý như bình thường | ✅ |
| TC-09 | Ghi log | Ghi giá trị nhiệt độ, thay đổi trạng thái và lỗi vào `system.log` | ✅ |

---

## 3. Yêu cầu nâng cao (Optional)

| ID | Yêu cầu | Mô tả chi tiết | 
|----|---------|----------------|
| TC-A01 | Cấu hình threshold | Cho phép thay đổi ngưỡng Fan (30°C), Alarm (40°C) trước khi chạy |
| TC-A02 | Trạng thái chi tiết | Thêm NORMAL / WARNING / CRITICAL / SENSOR_ERROR / FAILSAFE |
| TC-A03 | Thống kê min/max/avg | Tính nhiệt độ nhỏ nhất, lớn nhất, trung bình (running statistics) |
| TC-A04 | Random sensor failure | Sinh giá trị lỗi ngẫu nhiên (-999 hoặc 120) với xác suất 5% |
| TC-A05 | Failsafe 2 cấp | Transient (1-3 chu kỳ): giữ state cũ. Persistent (>3 chu kỳ): Fan ON + Alarm ON |
| TC-A06 | Sensor recovery | Phát hiện và log khi sensor phục hồi từ trạng thái lỗi |
| TC-A07 | Edge-triggered actuator | Chỉ log khi Fan/Alarm thực sự thay đổi trạng thái |
| TC-A08 | Split-screen UI | Dashboard auto-refresh + Input zone cố định (không bị ghi đè) |
| TC-A09 | Thread-safe input | 2 thread: Main loop + Input thread, mutex-protected shared data |

---

## 4. Yêu cầu kỹ thuật

| Hạng mục | Yêu cầu | Chi tiết triển khai |
|----------|---------|---------------------|
| Ngôn ngữ | C++17 | `std::optional`, structured bindings, `if constexpr` |
| Console | Bắt buộc | ANSI escape codes cho split-screen |
| Hardware | Không sử dụng | Mọi thứ giả lập bằng phần mềm |
| Sensor/Input | Giả lập | `std::uniform_real_distribution` + `std::cin` |
| Controller | Tách riêng | Class `TemperatureController` chứa toàn bộ logic |
| State | `enum class` | `SystemStatus`, `DeviceState`, `Mode` |
| Timing | `std::chrono` | `std::this_thread::sleep_for(1s)` |
| Data Structure | Running stats | `TemperatureStats` struct (min/max/sum/count) |
| File I/O | `std::ofstream` | Append mode, flush sau mỗi lần ghi |
| Error handling | 2+ tình huống | Sensor error + input validation + failsafe |
| Threading | `std::thread` | Input thread + `std::mutex` + `std::atomic` |
| Source | Tách .h / .cpp | 8 header files + 8 source files |

---

## 5. Ràng buộc

- Không sử dụng GUI, database, network, Web API
- Không sử dụng Arduino, ESP32, cảm biến hay thiết bị vật lý
- Không sử dụng external library (chỉ C++ Standard Library)
- Chương trình không được crash trong các test case bắt buộc
- Threshold/duration dùng constant/config, không magic number