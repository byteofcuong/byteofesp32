# Claude API làm bộ não (nghiên cứu 2026-11-15)

## Gói thuê bao và API
- Gói Max, Ultra, Team có credit API hằng tháng (Max: $100). Dùng được cho Claude API, Agent SDK, Managed Agents, Playground. Không cộng dồn; hết credit thì API dừng, không trừ vào gói.
- Nhận: claude.ai → Settings → Billing → API credits → Link organization (cần đủ 7 ngày trên gói), rồi tạo API key trong Console organization đó.
- Không dùng đăng nhập claude.ai cho server tự làm: Anthropic không cho bên thứ ba dùng đăng nhập claude.ai; Claude Code (`claude -p`) khởi động chậm.

## Chi phí ước tính một lượt (~2.000 token vào, ~200 token ra), giá 2026-11
| Model | Giá vào / ra mỗi 1M token | Mỗi lượt |
|---|---|---|
| Claude Haiku 5.5 | $0.10 / $0.50 | ~$0.0003 |
| Claude Sonnet 5.5 | $2 / $10 | ~$0.006 |
| Claude Opus 5.5 | $4 / $20 | ~$0.012 |

Claude chỉ nhận/trả chữ: cần faster-whisper (speech-to-text) và Piper (text-to-speech) chạy trên server.

## Nguồn
- https://support.claude.com/en/articles/17154008-monthly-api-credits-for-max-ultra-and-team-plans
- https://platform.claude.com/docs/en/about-claude/api-credits-for-subscribers
- https://code.claude.com/docs/en/agent-sdk/overview
