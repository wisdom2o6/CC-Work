# Trò chơi Tetris 3D (OpenGL & C++)

Một ứng dụng trò chơi **Tetris 3D** chất lượng cao được lập trình bằng ngôn ngữ **C++** và thư viện đồ họa **OpenGL**, tuân thủ đầy đủ và chính xác tất cả các yêu cầu về luật chơi, sàn 5x5, camera xoay quanh trục tâm và kiến trúc phân tách file rõ ràng.

---

## 🎮 Các Tính Năng & Luật Chơi Chính

1. **Sàn chơi 5x5 (5x5 Floor Grid):**
   - Mặt sàn có kích thước $5 \times 5$ ô (tọa độ $X \in [0, 4]$, $Z \in [0, 4]$) với hoa văn bàn cờ và viền laser Neon Cyan phát sáng.
   - Tháp kính 3D với 4 cột trụ và các vòng vạch chia tầng từ 1 đến 10.
   - Đánh dấu vạch đỏ neon đặc biệt tại **Tầng 10** (Giới hạn trần).

2. **Góc nhìn Camera xoay quanh một trục ở giữa (Orbit Camera 360°):**
   - Trục xoay trung tâm thẳng đứng đi qua tâm hình học của sàn: $(X = 2.5, Y = 5.0, Z = 2.5)$.
   - Dùng chuột kéo (chuột trái hoặc chuột phải) để xoay góc nhìn tự do 360° quanh trục trung tâm.
   - Dùng con lăn chuột (Mouse Wheel) để phóng to / thu nhỏ (Zoom) camera mượt mà.
   - Hỗ trợ đổi nhanh các góc nhìn định sẵn qua phím `V` (Góc Isometric 3D, Góc Top-Down nhìn thẳng từ trên xuống, Góc Front nhìn thẳng cạnh).

3. **Cơ chế Khay 3 khối & Đếm ngược 3 phút (3-Block Batch & 3-Minute Timer):**
   - Mỗi đợt (Round), trò chơi sinh ngẫu nhiên **3 khối 3D** vào khay chọn.
   - Bộ đếm ngược **3 phút (180 giây)** hiển thị rõ ràng trên thanh trạng thái phía trên.
   - Người chơi phải đặt thành công cả 3 khối vào sàn trước khi hết 3 phút.
   - Người chơi có thể tự do chọn đặt khối nào trước bằng cách nhấn phím `1`, `2`, `3` hoặc click chuột trực tiếp vào khay 3 khối ở dưới màn hình.
   - Khi hoàn thành cả 3 khối trong 3 phút: được cộng điểm thưởng (+500 điểm), làm mới bộ đếm 3 phút và tạo tiếp 3 khối mới cho đợt tiếp theo.

4. **Điều kiện Xử Thua (Loss Conditions):**
   - **Thua do hết giờ:** Hết 3 phút mà chưa xếp hết 3 khối trong đợt hiện tại vào sàn.
   - **Thua do vượt quá chiều cao:** Khi xếp, nếu bất kỳ ô nào của khối vượt quá độ cao 10 ($Y \ge 10$) sẽ bị xử thua ngay lập tức.

5. **Xóa tầng & Điểm kỷ lục (Full Layer Clearance & High Score):**
   - Khi lấp đầy toàn bộ 25 ô của 1 tầng ($5 \times 5 = 25$ ô), hiệu ứng chớp sáng vàng/trắng sẽ kích hoạt và tầng đó sẽ bị xóa.
   - Các tầng phía trên sẽ sụt xuống 1 nấc.
   - Hệ thống tính điểm:
     - Đặt thành công 1 khối: +50 điểm
     - Xóa 1 tầng: +1.000 điểm
     - Xóa 2 tầng cùng lúc: +2.500 điểm
     - Xóa 3 tầng cùng lúc: +5.000 điểm
     - Xóa 4 tầng cùng lúc: +10.000 điểm (Combo Mega Bonus)
     - Hoàn thành đợt 3 khối: +500 điểm
   - Điểm kỷ lục (High Score) được tự động lưu vào tệp `highscore.txt` để lưu lại thành tích giữa các lần chơi.

6. **Hệ Thống 20 Khối 3D Đa Dạng & Khối Cao 2-3 Tầng (Tăng Độ Khó):**
   - **Các khối kinh điển 1 tầng:** Thanh dài I (4 ô), Khối vuông O (2x2), Chữ L (4 ô), Chữ J (4 ô), Chữ T (4 ô), Chữ Z (4 ô), Góc nhỏ 3 ô, Thanh ngắn 3 ô.
   - **Các khối cao 2 tầng (Chiếm 2 tầng không gian):**
     - Bậc Thang 2 Tầng (4 ô)
     - Cổng Vòm 2 Tầng (5 ô, vòm rỗng 1 ô ở đáy)
     - Chữ U Đứng 2 Tầng (5 ô)
     - Chữ T Đứng 2 Tầng (4 ô)
     - Tường Góc 2 Tầng (5 ô)
     - Rắn Xoắn 2 Tầng (5 ô, uốn lượn cả 3 trục)
     - Kim Tự Tháp 2 Tầng (5 ô, đế 2x2 chóp 1 ô)
     - Chữ Z Đứng 2 Tầng (4 ô)
     - Góc 3D Tripod (4 ô)
     - Nhánh Xoắn 3D (4 ô)
   - **Các khối cao 3 tầng siêu thách thức:**
     - Cột Cao 3 Tầng (3 ô)
     - Chữ L Đứng 3 Tầng (4 ô)
   - Hiển thị bóng đổ phát sáng (Ghost Block) và vết chiếu sàn (Floor Footprint) để người chơi dễ dàng ngắm vị trí rơi chính xác trong không gian 3D.

7. **Âm thanh tổng hợp tương tác (Procedural Sound):**
   - Âm thanh di chuyển, xoay khối, tiếng rơi đầm ấm, hợp âm vui tươi khi xóa tầng và hiệu ứng khi Game Over.

---

## ⌨️ Bảng Hướng Dẫn Điều Khiển

| Thao Tác | Phím / Chuột | Chức Năng |
| :--- | :--- | :--- |
| **Xoay Camera 360°** | **Kéo Chuột Phải / Trái** | Xoay góc nhìn 360° quanh trục tâm (mượt mà, không giật khi qua mốc 0/360°) |
| **Xoay nhanh 90°** | **Phím [ / ]** hoặc **Click nút HUD** | Xoay tức thì góc nhìn 90° sang trái hoặc phải |
| **Thu phóng** | **Cuộn Chuột (Wheel)** | Zoom lại gần hoặc ra xa tháp 3D |
| **Di chuyển khối** | **W, A, S, D** hoặc **Mũi tên** | Di chuyển khối trên sàn 5x5 (Tự động thích ứng hướng nhìn, hỗ trợ giữ phím DAS/ARR mượt mà) |
| **Xoay trục Y** | **Q / E** | Xoay khối quanh trục thẳng đứng Y (Xoay ngang) |
| **Xoay trục X** | **R / F** | Xoay khối quanh trục X (Lật lên / xuống) |
| **Xoay trục Z** | **Z / C** | Xoay khối quanh trục Z (Nghiêng trái / phải) |
| **Thả rơi ngay** | **Phím Space** | Rơi tức thì xuống đáy sàn (Hard Drop) |
| **Rơi nhanh** | **Shift** hoặc **Phím X** | Cho khối rơi nhanh xuống (Soft Drop liên tục khi giữ phím) |
| **Chọn khối** | **Phím 1, 2, 3** hoặc **Click chuột** | Chọn 1 trong 3 khối trong khay để chơi |
| **Đổi góc nhìn** | **Phím V** hoặc **Click nút HUD** | Chuyển đổi qua lại giữa góc Isometric 3D, Top-Down, Front |
| **Tạm dừng** | **Phím P** | Tạm dừng hoặc tiếp tục ván chơi |
| **Chơi lại** | **Phím R** hoặc **Space** | Khởi động lại ván chơi mới khi bị Game Over |

---

## 📁 Cấu Trúc Dự Án (Phân Tách Module Rõ Ràng)

Mã nguồn được phân chia thành các file riêng biệt để phân định rạch ròi giữa **Logic khối**, **Đồ họa OpenGL**, **Hệ thống Font chữ** và **Luật chơi**:

```
d:\Tetris 3D\
├── src\
│   ├── Config.h         # Cấu hình các hằng số: sàn 5x5, độ cao tối đa 10, thời gian 180s, điểm số
│   ├── Types.h          # Cấu trúc dữ liệu Vector 2D/3D (Vec2, Vec3, Vec3i) và Màu sắc (Color4f)
│   ├── BlockLogic.h     # [LOGIC KHỐI] Khai báo hình dạng polycube, xoay 3D theo 3 trục, khay 3 khối
│   ├── BlockLogic.cpp   # [LOGIC KHỐI] Hiện thực hóa tọa độ khối, thuật toán xoay và chuẩn hóa
│   ├── GameLogic.h      # [LOGIC GAME] Khai báo bảng lưới 5x5x10, kiểm tra va chạm, xóa tầng, bộ đếm giờ
│   ├── GameLogic.cpp    # [LOGIC GAME] Hiện thực hóa luật chơi, điều kiện thua, wall-kick mở rộng
│   ├── Camera.h         # [CAMERA] Khai báo Camera xoay quanh trục tâm 360 độ và thu phóng
│   ├── Camera.cpp       # [CAMERA] Thuật toán Lerp góc ngắn nhất tránh giật 360°, xoay nhanh 90°
│   ├── FontRenderer.h   # [FONT CHỮ] Hệ thống font TrueType Segoe UI tiếng Việt có dấu sắc nét
│   ├── FontRenderer.cpp # [FONT CHỮ] Tạo Dynamic Texture Atlas và render text OpenGL 2D
│   ├── Renderer.h       # [ĐỒ HỌA OPENGL] Khai báo vẽ không gian 3D, bóng chiếu sàn và giao diện HUD 2D
│   ├── Renderer.cpp     # [ĐỒ HỌA OPENGL] Khối 3D vát cạnh, tháp kính, bóng chiếu sàn, HUD hiện đại
│   ├── Audio.h          # Khai báo hệ thống âm thanh hiệu ứng
│   ├── Audio.cpp        # Tổng hợp sóng âm thanh (Procedural WAV synthesis)
│   ├── Input.h          # Quản lý điều khiển chuột và bàn phím (Hỗ trợ DAS/ARR)
│   ├── Input.cpp        # Hiện thực hóa di chuyển thông minh theo góc nhìn camera, click an toàn UI
│   └── main.cpp         # Điểm khởi chạy: Cửa sổ Win32, khởi tạo OpenGL context và game loop
├── build.bat            # Script biên dịch tự động 1-click
├── Tetris3D.exe         # File thực thi đã biên dịch sẵn (chạy ngay)
└── README.md            # Tài liệu hướng dẫn chi tiết
```

---

## 🚀 Cách Biên Dịch & Chạy Game

### Cách 1: Chạy trực tiếp file đã biên dịch
Bạn có thể khởi động ngay trò chơi bằng cách click đúp vào:
```
Tetris3D.exe
```

### Cách 2: Tự biên dịch lại với `build.bat`
Click đúp vào file `build.bat` trong thư mục dự án hoặc chạy lệnh sau trong PowerShell / Command Prompt:
```cmd
.\build.bat
```
Lệnh biên dịch sử dụng trình biên dịch GCC C++ đi kèm:
```cmd
w64devkit\bin\g++.exe -O2 -std=c++17 src\*.cpp -o Tetris3D.exe -lopengl32 -lglu32 -lgdi32 -luser32 -lwinmm
```
