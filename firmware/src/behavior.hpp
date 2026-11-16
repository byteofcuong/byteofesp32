// Lớp hành vi: quyết định biểu cảm nào hiện lúc nào (docs/design/behavior.md).
// Ba lớp ưu tiên từ trên xuống: hội thoại (lệnh server, có thời gian giữ), phản xạ chạm (chỉ khi rảnh),
// đời sống khi rảnh (chán -> buồn ngủ -> ngủ, ban đêm sớm hơn). Chạy trong loop() (lõi 1), cùng linh vật.
#pragma once

#include <Arduino.h>

#include "mascot.hpp"

namespace behavior {

void begin(Mascot* mascot);

// Chạm nhanh (đã lọc chạm lúc ngủ): phản ứng nhỏ của linh vật + đếm nhịp chạm (vuốt ve / chọc).
void onTap(int32_t x, int32_t y, uint32_t now);
// Giữ tay 0.6–5s: xoa đầu.
void onHold(uint32_t now);
// Chạm lúc đang ngủ: giật mình rồi ngái ngủ.
void onWake(uint32_t now);

// Lệnh từ server. hold_ms = 0: giữ biểu cảm tới lệnh sau; > 0: hết giờ phai về bình thường.
void onServerState(AssistantState s, uint32_t now);
void onServerExpression(Expression e, uint32_t hold_ms, uint32_t now);

// Gọi mỗi vòng loop(), trước Mascot::update.
void update(uint32_t now);

}  // namespace behavior
