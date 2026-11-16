# Bộ chứng chỉ gốc cho TLS (ADR 0008)

`x509_crt_bundle` là danh sách CA gốc của Mozilla ở dạng nén của ESP32 (chỉ giữ tên và khóa công khai), nhúng vào firmware qua `board_build.embed_files` trong `platformio.ini`. Board dùng nó để kiểm chứng chứng chỉ của server `wss://…` (Cloudflare tự chọn CA giữa Let's Encrypt, Google Trust Services và SSL.com, nên không ghim một CA).

- Nguồn: https://curl.se/ca/cacert.pem, "Certificate data from Mozilla as of: Thu Nov 5 04:12:01 2026 GMT", 118 chứng chỉ, sha256 bắt đầu bằng `427c064216761a1e`.
- Tạo ngày 2026-11-16. File ra: 54 752 byte, sha256 bắt đầu bằng `8ca330673e7ecd07`.

Làm mới (khoảng 1–2 năm một lần, hoặc khi board báo lỗi chứng chỉ). Chạy trong một thư mục trống:

```bash
curl -sS -o cacert.pem https://curl.se/ca/cacert.pem
python ~/.platformio/packages/framework-arduinoespressif32/tools/gen_crt_bundle.py -i cacert.pem
cp x509_crt_bundle "<repo>/firmware/certs/x509_crt_bundle"
```
Công cụ cần gói Python `cryptography`.
