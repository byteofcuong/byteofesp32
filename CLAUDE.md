@AGENTS.md

## Riêng cho Claude Code
- Auto-memory của Claude chỉ giữ sở thích của người dùng. Kiến thức dự án ghi vào `docs/` và `.claude/ontology/` để mọi agent (và Codex) đều đọc được.
- Skill của repo: `/handoff` (cuối session hoặc trước khi context đầy). Skill toàn cục `ontology` dùng để cập nhật `.claude/ontology/`.
- Domain ontology: `.claude/ontology/ontology.md` (khái niệm, quan hệ, trạng thái, quy tắc, có neo vào file). Hook SessionStart chạy check và chỉ in khi neo bị hỏng.
