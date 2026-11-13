# 开发记录

## 2026-11-13 降低 Korvo 并发语音内部内存峰值

WebSocket transport buffer 2048→1024，任务栈 6144→4096，减少与 I2S、Codec 和 TLS 并存时的内部 RAM 压力。上层已有消息帧组装；不能将 transport buffer 大小当成协议消息长度限制。Korvo 已构建并上板，多轮合成 PCM 打断测试完成 hello/STT/MCP/TTS，最近板测未出现 AES/TLS 内存不足；长消息和长期压力仍需进一步实测。详细板级记录见总工程 gateway/docs/development-log.md。迁移时按平台栈高水位及协议测试重新评估，不能盲目继续缩栈。
